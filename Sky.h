#pragma once
#include <wrl/client.h>
#include "Mesh.h"
#include "Camera.h"
#include "memory.h"

// since we are switching to bindless, need more resources on c++ side
struct SkyDrawData
{
	unsigned int vertexShaderBufferIndex;
	unsigned int vertexShaderConstantBuffer;
	unsigned int pixelShaderSkybox;
};

class Sky
{
public:
	Sky(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		std::shared_ptr<Mesh> mesh);
	
	~Sky();

	void Draw(std::shared_ptr<Camera> camera);

private:
	// setup for rendering
	void Initialize();

	// root sig and pso
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

	// mesh object
	std::shared_ptr<Mesh> skyboxMesh;

	unsigned int descriptorIndex;
};