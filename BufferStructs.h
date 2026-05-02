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

struct FullScreenData
{
	unsigned int screenIndex;
};

struct RefractionData
{
	DirectX::XMFLOAT2 screenSize;
	unsigned int normalIndex;
	unsigned int refractionScale;
	unsigned int screenIndex;
};

// simplified struct that will send data for skyboxes
struct PerFrameData
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};