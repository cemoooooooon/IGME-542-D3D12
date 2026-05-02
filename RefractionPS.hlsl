#include "ShaderIncludes.hlsli"
SamplerState BasicSampler : register(s0);
SamplerState ClampSampler : register(s1);

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 worldPosition : POSITION;
};

cbuffer ExternalData : register(b0)
{
    float2 screenSize;
    unsigned int normalIndex;
    unsigned int refractionScale;
    unsigned int screenIndex;
};

// gotten from screen space refraction slides
float4 main(VertexToPixel input) : SV_TARGET
{
    // grab resources
    Texture2D NormalTexture = ResourceDescriptorHeap[normalIndex];
    Texture2D ScreenPixels = ResourceDescriptorHeap[screenIndex];
    
    // get screenspace uvs
    float2 screenUV = input.screenPosition.xy / screenSize;
    
    // object's normal will be an offset for uvs
    float2 offsetUV = NormalTexture.Sample(BasicSampler, input.uv).xy * 2 - 1;
    offsetUV.y *= -1;
    
    // distort using offset
    float2 refractedUV = screenUV + offsetUV * refractionScale;
    return ScreenPixels.Sample(ClampSampler, refractedUV);
}