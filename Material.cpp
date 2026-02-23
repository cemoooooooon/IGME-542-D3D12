#include "Material.h"

Material::Material(DirectX::XMFLOAT3 colorTint, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset, Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, unsigned int albedo, unsigned int normalMap, unsigned int metalness, unsigned int roughness) :
	colorTint(colorTint), uvScale(uvScale), uvOffset(uvOffset), pipelineState(pipelineState), albedo(albedo), normalMap(normalMap), metalness(metalness), roughness(roughness)
{

}

DirectX::XMFLOAT3 Material::GetColorTint()
{
	return colorTint;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState()
{
	return pipelineState;
}

unsigned int Material::GetAlbedo()
{
	return albedo;
}

unsigned int Material::GetNormalMap()
{
	return normalMap;
}

unsigned int Material::GetMetalness()
{
	return metalness;
}

unsigned int Material::GetRoughness()
{
	return roughness;
}

void Material::SetColorTint(DirectX::XMFLOAT3 colorTint)
{
	this->colorTint = colorTint;
}

void Material::SetUVScale(DirectX::XMFLOAT2 uvScale)
{
	this->uvScale = uvScale;
}

void Material::SetUVOffSet(DirectX::XMFLOAT2 uvOffset)
{
	this->uvOffset = uvOffset;
}

void Material::SetAlbedo(unsigned int albedo)
{
	this->albedo = albedo;
}

void Material::SetNormalMap(unsigned int normalMap)
{
	this->normalMap = normalMap;
}

void Material::SetMetalness(unsigned int metalness)
{
	this->metalness = metalness;
}

void Material::SetRoughness(unsigned int roughness)
{
	this->roughness = roughness;
}
