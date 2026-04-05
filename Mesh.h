#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "Vertex.h"
#include <string>

// Structs
struct MeshRayTracingData
{
	D3D12_GPU_DESCRIPTOR_HANDLE IndexBufferSRV{};
	D3D12_GPU_DESCRIPTOR_HANDLE VertexBufferSRV{};
	Microsoft::WRL::ComPtr<ID3D12Resource> BLAS;
};

class Mesh
{
public:
	Mesh(Vertex* vertices, unsigned int* indices, size_t vertexCount, size_t indexCount);
	Mesh(const std::wstring& objFile);

	void CreateBuffer(Vertex* vertices, unsigned int* indices, size_t vertexCount, size_t indexCount);
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexBuffer();
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
	int GetIndexCount();
	int GetVertexCount();
	const MeshRayTracingData& GetRayTracingData();
	~Mesh();

private:
	// Buffers and views
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	int indexCount;
	int vertexCount;
	void CalculateTangents(Vertex* vertices, size_t vertexCount, unsigned int* indices, size_t indexCount);

	// Raytracing variables
	MeshRayTracingData rayTracingData;
};

