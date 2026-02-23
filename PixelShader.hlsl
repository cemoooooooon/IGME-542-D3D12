#include "ShaderIncludes.hlsli"
SamplerState BasicSampler : register(s0);

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 worldPosition : POSITION;
};

// Struct for all of the data that the pixel shader will need to access
cbuffer ExternalData : register(b0)
{
    unsigned int albedoIndex;
    unsigned int normalIndex;
    unsigned int metalIndex;
    unsigned int roughnessIndex;
    float2 uvScale;
    float2 uvOffset;
    float3 camPos;
    unsigned int lightCount;
    Light lights[4];
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // load textures
    Texture2D Albedo = ResourceDescriptorHeap[albedoIndex];
    Texture2D NormalMap = ResourceDescriptorHeap[normalIndex];
    Texture2D MetalnessMap = ResourceDescriptorHeap[metalIndex];
    Texture2D RoughnessMap = ResourceDescriptorHeap[roughnessIndex];
    
    // adjusting uvs
    input.uv = input.uv * uvScale + uvOffset;
    
    // normalize... normals? and tangents!
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    
    // apply normal mapping from texture
    input.normal = normalize(
    NormalMapCalculations(NormalMap, BasicSampler, input.uv, input.normal, input.tangent)
    );
    
    // sample the albedo and gamma correct-correct!
    float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
    
    // sample roughness
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    
    // sample metalness
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    
    // determining specular color
    float3 specularColor = lerp(0.04f, surfaceColor.rgb, metalness);
    
    // ambient lights
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < lightCount; i++)
    {
        Light light = lights[i];
        light.direction = normalize(light.direction);
        
        switch (light.type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                finalColor += DirectionalLight(light, input.normal, surfaceColor, specularColor, camPos, input.worldPosition, roughness, metalness);
                break;
            
            case LIGHT_TYPE_POINT:
                finalColor += PointLight(light, input.normal, surfaceColor, specularColor, camPos, input.worldPosition, roughness, metalness);
                break;
            
            case LIGHT_TYPE_SPOT:
                finalColor += SpotLight(light, input.normal, surfaceColor, specularColor, camPos, input.worldPosition, roughness, metalness);
                break;
        }
    }
    
    //gamma correct that final color!
    return float4(pow(finalColor, 1.0f / 2.2f), 1);
}