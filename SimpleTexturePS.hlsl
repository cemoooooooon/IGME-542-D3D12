cbuffer DrawData : register(b0)
{
    uint TextureIndex;
}

// input to this shader
struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    Texture2D t = ResourceDescriptorHeap[TextureIndex];
    return t.Sample(BasicSampler, input.uv);
}