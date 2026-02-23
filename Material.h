#pragma once
#include <memory>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d12.h>

class Material
{
public:
	Material(DirectX::XMFLOAT3 colorTint, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset, Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, unsigned int albedo, unsigned int normalMap, unsigned int metalness, unsigned int roughness);

	// Getters
	DirectX::XMFLOAT3 GetColorTint();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();
	unsigned int GetAlbedo();
	unsigned int GetNormalMap();
	unsigned int GetMetalness();
	unsigned int GetRoughness();

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 colorTint);
	void SetUVScale(DirectX::XMFLOAT2 uvScale);
	void SetUVOffSet(DirectX::XMFLOAT2 uvOffset);
	void SetAlbedo(unsigned int albedo);
	void SetNormalMap(unsigned int normalMap);
	void SetMetalness(unsigned int metalness);
	void SetRoughness(unsigned int roughness);

private:
	// Material thangs
	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

	// Texture thangs
	unsigned int albedo;
	unsigned int normalMap;
	unsigned int metalness;
	unsigned int roughness;
};

