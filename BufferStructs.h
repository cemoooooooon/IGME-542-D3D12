#pragma once
#include <DirectXMath.h>
#include "Lights.h"

struct VertexShaderExternalData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 worldInverseTranspose;
};

struct PixelShaderExternalData
{
	unsigned int albedoIndex;
	unsigned int normalIndex;
	unsigned int metalIndex;
	unsigned int roughnessIndex;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT3 camPos;
	unsigned int lightCount;
	Light lights[4];
};

// Overall scene data for ray tracing
struct RaytracingSceneData
{
	DirectX::XMFLOAT4X4 InverseViewProjection;
	DirectX::XMFLOAT3 CameraPosition;
	float pad;
};

// Root constants for bindless resources
struct RayTracingDrawData
{
	unsigned int SceneDataConstantBufferIndex;
	unsigned int EntityDataDescriptorIndex;
	unsigned int SceneTLASDescriptorIndex;
	unsigned int OutputUAVDescriptorIndex;
};

// Overall scene data for ray tracing (constant buffer)
struct RayTracingSceneData
{
	DirectX::XMFLOAT4X4 InverseViewProjection;
	DirectX::XMFLOAT3 CameraPosition;
	float pad;
};

// Per entity information (geom, material, etc.)
// - Multiple sets of these will be stored in a structured buffer
// - Materials *could* be separated out into their own buffer
// to cut down on data repetition
struct RayTracingEntityData
{
	DirectX::XMFLOAT4 Color;
	unsigned int VertexBufferDescriptorIndex;
	unsigned int IndexBufferDescriptorIndex;
	float pad[2];
};