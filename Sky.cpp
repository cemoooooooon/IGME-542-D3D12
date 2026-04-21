#include "Sky.h"
#include "Graphics.h"
#include <d3d12.h>
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "PathHelpers.h"
#include "BufferStructs.h"

// compiler helpers for shader blobs
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

Sky::Sky(const wchar_t* right, const wchar_t* left, const wchar_t* up, const wchar_t* down, const wchar_t* front, const wchar_t* back, std::shared_ptr<Mesh> mesh) :
	skyboxMesh(mesh)
{
	// set up for rendering
	Initialize();

	// get our cubemap set up
	descriptorIndex = Graphics::CreateCubemap(right, left, up, down, front, back);
}

Sky::~Sky()
{
}

void Sky::Draw(std::shared_ptr<Camera> camera)
{
	// set up pipeline and root sig for skybox
	Graphics::CommandList->SetPipelineState(pipelineState.Get());
	Graphics::CommandList->SetGraphicsRootSignature(rootSignature.Get());

	// set up cbuffer
	SkyDrawData drawData{};
	drawData.pixelShaderSkybox = descriptorIndex;
	drawData.vertexShaderBufferIndex = Graphics::GetDescriptorIndex(skyboxMesh->GetVertexBufferDescriptorHandle());

	// fill out vertes shader data
	{
		PerFrameData frameData{};
		frameData.view = camera->GetViewMatrix();
		frameData.projection = camera->GetProjectionMatrix();

		D3D12_GPU_DESCRIPTOR_HANDLE cbHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
			(void*)(&frameData), sizeof(PerFrameData));

		drawData.vertexShaderConstantBuffer = Graphics::GetDescriptorIndex(cbHandle);
	}

	// send to command list
	Graphics::CommandList->SetGraphicsRoot32BitConstants(
		0,
		sizeof(SkyDrawData) / sizeof(unsigned int),
		&drawData,
		0);

	// grab buffer views from mesh
	D3D12_INDEX_BUFFER_VIEW ibv = skyboxMesh->GetIndexBufferView();
	Graphics::CommandList->IASetIndexBuffer(&ibv);

	// draw it
	Graphics::CommandList->DrawIndexedInstanced((UINT)skyboxMesh->GetIndexCount(), 1, 0, 0, 0);
}


// set up root signature and the pipeline state object
void Sky::Initialize()
{
	// root signature
	{
		D3D12_ROOT_PARAMETER rootParams[1] = {};

		// Define the root parameter
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParams[0].Constants.Num32BitValues = sizeof(SkyDrawData) / sizeof(unsigned int);
		rootParams[0].Constants.RegisterSpace = 0;
		rootParams[0].Constants.ShaderRegister = 0;

		// Create a single static sampler for skybox
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
		rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
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

	// pipleline state
	{
		// load shaders for blob
		Microsoft::WRL::ComPtr<ID3DBlob> vsByte;
		Microsoft::WRL::ComPtr<ID3DBlob> psByte;
		D3DReadFileToBlob(FixPath(L"SkyVS.cso").c_str(), vsByte.GetAddressOf());
		D3DReadFileToBlob(FixPath(L"SkyPS.cso").c_str(), psByte.GetAddressOf());

		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// -- Input assembler related ---
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Overall primitive topology type (triangle, line, etc.) is set here
		// IASetPrimTop() is still used to set list/strip/adj options

		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();

		// -- Shaders (VS/PS) ---
		psoDesc.VS.pShaderBytecode = vsByte->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vsByte->GetBufferSize();
		psoDesc.PS.pShaderBytecode = psByte->GetBufferPointer();
		psoDesc.PS.BytecodeLength = psByte->GetBufferSize();

		// -- Render targets ---
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;

		// -- States ---
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; // because skybox
		psoDesc.RasterizerState.DepthClipEnable = true;
		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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
}
