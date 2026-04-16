#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "BufferStructs.h"
#include "RayTracing.h"

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// Random function using ranges
#define Random(min, max) (float)rand() / RAND_MAX * (max - min) + min

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Initialize raytracing
	RayTracing::Initialize(
		Window::Width(),
		Window::Height(),
		FixPath(L"RayTracing.cso"));

	//CreateRootSigAndPipelineState();
	CreateGeometry();

	// Create camera
	camera = std::make_shared<Camera>(
		Window::AspectRatio(),
		XMFLOAT3(0.0f, 2.0f, -5.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		90.0f,
		0.1f,
		1000.0f,
		30.0f,
		20.0f);
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Wait for the GPU before we shut down
	Graphics::WaitForGPU();
}

// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature
// and pipeline state object for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;

	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(
			FixPath(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(
			FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
	}

	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION"; // Name must match semantic in shader
		inputElements[0].SemanticIndex = 0; // This is the first POSITION semantic
		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT; // R32 G32 = float2
		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].SemanticIndex = 0; // This is the first TEXCOORD semantic
		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[2].SemanticName = "NORMAL";
		inputElements[2].SemanticIndex = 0; // This is the first NORMAL semantic
		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0; // This is the first TANGENT semantic
	}

	// Root Signature
	{
		// Define a table of CBV's (constant buffer views)
		D3D12_DESCRIPTOR_RANGE cbvTable = {};
		cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTable.NumDescriptors = 1;
		cbvTable.BaseShaderRegister = 0;
		cbvTable.RegisterSpace = 0;
		cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		
		// Define the root parameter
		D3D12_ROOT_PARAMETER rootParam = {};
		rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParam.DescriptorTable.NumDescriptorRanges = 1;
		rootParam.DescriptorTable.pDescriptorRanges = &cbvTable;

		// New root params for the pixel shader
		D3D12_ROOT_PARAMETER psRootParam = {};
		psRootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		psRootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		psRootParam.DescriptorTable.NumDescriptorRanges = 1;
		psRootParam.DescriptorTable.pDescriptorRanges = &cbvTable;

		D3D12_ROOT_PARAMETER rootParams[] = { rootParam, psRootParam };

		// Create a single static sampler for allll the pixel shaders
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0; // Means register(s0) in the shader
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe the overall the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;
		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors->GetBufferPointer());
		}

		// Actually create the root sig
		Graphics::Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}

	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// -- Input assembler related ---
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Overall primitive topology type (triangle, line, etc.) is set here
		// IASetPrimTop() is still used to set list/strip/adj options
		
		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();

		// -- Shaders (VS/PS) ---
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();

		// -- Render targets ---
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;

		// -- States ---
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;
		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask =
			D3D12_COLOR_WRITE_ENABLE_ALL;

		// -- Misc ---
		psoDesc.SampleMask = 0xffffffff;

		// Create the pipe state object
		Graphics::Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));
	}

	// Set up the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Load in meshes
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str());
	std::shared_ptr<Mesh> cylinder = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str());
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str());
	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str());
	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str());
	std::shared_ptr<Mesh> quad2Side = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> can = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/can.obj").c_str());

	// Load in textures

	// rock
	unsigned int rockAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/rock/albedo.jpg").c_str());
	unsigned int rockNormal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/rock/normal.jpg").c_str());
	unsigned int rockMetal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/rock/metalness.png").c_str());
	unsigned int rockRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/rock/roughness.jpg").c_str());

	// marble
	unsigned int marbleAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/marble/albedo.jpg").c_str());
	unsigned int marbleNormal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/marble/normal.jpg").c_str());
	unsigned int marbleMetal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/marble/metalness.png").c_str());
	unsigned int marbleRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/marble/roughness.jpg").c_str());

	// wood
	unsigned int woodAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/wood/albedo.jpg").c_str());
	unsigned int woodNormal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/wood/normal.jpg").c_str());
	unsigned int woodMetal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/wood/metalness.png").c_str());
	unsigned int woodRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/wood/roughness.jpg").c_str());

	// Make them textures
	std::shared_ptr<Material> rockMaterial = std::make_shared<Material>(
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f),
		DirectX::XMFLOAT2(0.0f, 0.0f),
		pipelineState, rockAlbedo, rockNormal, rockMetal, rockRoughness
	);

	std::shared_ptr<Material> marbleMaterial = std::make_shared<Material>(
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f),
		DirectX::XMFLOAT2(0.0f, 0.0f),
		pipelineState, marbleAlbedo, marbleNormal, marbleMetal, marbleRoughness
	);

	std::shared_ptr<Material> woodMaterial = std::make_shared<Material>(
		DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f),
		DirectX::XMFLOAT2(1.0f, 1.0f),
		DirectX::XMFLOAT2(0.0f, 0.0f),
		pipelineState,woodAlbedo,woodNormal, woodMetal, woodRoughness
	);

	// Actual entities
	std::shared_ptr<GameEntity> torusEntity = std::make_shared<GameEntity>(torus, rockMaterial);
	std::shared_ptr<GameEntity> floorEntity = std::make_shared<GameEntity>(cube, woodMaterial);

	torusEntity->GetTransform()->SetPosition(0.0f, 5.0f, 0.0f);

	floorEntity->GetTransform()->SetPosition(0.0f, -10.0f, 0.0f);
	floorEntity->GetTransform()->SetScale(10.0f, 10.0f, 10.0f);

	entities.push_back(torusEntity);
	entities.push_back(floorEntity);

	// Create 20 random spheres
	for (unsigned int i = 0; i < 20; i++)
	{
		std::shared_ptr<Material> coloredMaterial = std::make_shared<Material>(
			XMFLOAT3(Random(0.0f, 1.0f), Random(0.0f, 1.0f), Random(0.0f, 1.0f)),
			XMFLOAT2(1.0f, 1.0f),
			XMFLOAT2(0.0f, 0.0f),
			pipelineState,
			marbleAlbedo,
			marbleNormal,
			0,
			0);

		float randomScale = Random(0.3f, 1.6f);

		std::shared_ptr<GameEntity> coloredSphere = std::make_shared<GameEntity>(sphere, coloredMaterial);
		coloredSphere->GetTransform()->SetScale(randomScale, randomScale, randomScale);
		coloredSphere->GetTransform()->SetPosition(
			Random(-5.0f, 5.0f),
			1.0f,
			Random(-5.0f, 5.0f));

		entities.push_back(coloredSphere);
	}

	// After creating entites, make sure they have their buffer data
	RayTracing::CreateEntityDataBuffer(entities);

	// Once we have all of the BLASs ready, we can make a TLAS
	RayTracing::CreateTopLevelAccelerationStructureForScene(entities);

	// Finalize any initialization and wait for the GPU
	// before proceeding to the game loop
	Graphics::CloseAndExecuteCommandList();
	Graphics::WaitForGPU();
	Graphics::ResetAllocatorAndCommandList();

	// Create lights
	directionalLight = {};
	directionalLight.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	directionalLight.range = 8.0f;
	directionalLight.position = XMFLOAT3(0.0f, 6.0f, 0.0f);
	directionalLight.intensity = 2.6f;
	directionalLight.color = XMFLOAT3(0.23f, 0.87f, 0.33f);

	directionalLight2 = {};
	directionalLight2.type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.direction = XMFLOAT3(1.0f, -0.75f, 0.0f);
	directionalLight2.range = 4.0f;
	directionalLight2.position = XMFLOAT3(-4.8f, 2.38f, 0.0f);
	directionalLight2.intensity = 1.3f;
	directionalLight2.color = XMFLOAT3(0.8f, 0.0f, 0.0f);

	spotLight = {};
	spotLight.type = LIGHT_TYPE_SPOT;
	spotLight.direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	spotLight.range = 6.0f;
	spotLight.position = XMFLOAT3(0.0f, 0.0f, -3.0f);
	spotLight.intensity = 2.2f;
	spotLight.spotInnerAngle = 28.0f;
	spotLight.spotOuterAngle = 39.5f;
	spotLight.color = XMFLOAT3(0.2f, 0.7f, 0.0f);

	pointLight = {};
	pointLight.type = LIGHT_TYPE_POINT;
	pointLight.range = 3.0f;
	pointLight.position = XMFLOAT3(2.0f, 4.8f, -1.3f);
	pointLight.intensity = 1.55f;
	pointLight.color = XMFLOAT3(0.0f, 0.0f, 1.0f);


	lights.push_back(directionalLight);
	lights.push_back(directionalLight2);
	lights.push_back(spotLight);
	lights.push_back(pointLight);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Update camera projection matrix
	camera->UpdateProjectionMatrix(Window::AspectRatio());

	// Resize the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}

	// Resize raytracing output texture
	RayTracing::ResizeOutputUAV(Window::Width(), Window::Height());
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Update Camera
	camera->Update(deltaTime);

	entities[0]->GetTransform()->Rotate(deltaTime, deltaTime, 0.0f);

	// Loop through colored spheres and move them
	for (int i = 2; i < entities.size(); i++)
	{
		XMFLOAT3 spherePosition = entities[i]->GetTransform()->GetPosition();

		switch (i % 2)
		{
		case 0:
			spherePosition.x = (float)sin((totalTime + i) * 0.3) * 3;
			break;

		case 1:
			spherePosition.z = (float)sin((totalTime + i) * 0.3) * 3;
			break;
		}

		entities[i]->GetTransform()->SetPosition(spherePosition);
	}

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer =
		Graphics::BackBuffers[Graphics::SwapChainIndex()];

	// Raytracing
	{
		RayTracing::CreateTopLevelAccelerationStructureForScene(entities);
		RayTracing::Raytrace(camera, currentBackBuffer);
	}

	// Present
	{
		// Must occur BEFORE present
		Graphics::CloseAndExecuteCommandList();
		// Present the current back buffer and move to the next one
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
		Graphics::AdvanceSwapChainIndex();
		// Wait for the GPU to be done and then reset the command list & allocator
		Graphics::WaitForGPU();
		Graphics::ResetAllocatorAndCommandList();
	}
}



