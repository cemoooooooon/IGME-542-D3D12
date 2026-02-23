#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f

// light struct
struct Light
{
    int type;
    float3 direction;
    float range;
    float3 position;
    float intensity;
    float3 color;
    float spotInnerAngle;
    float spotOuterAngle;
    float2 padding;
};

// helper functions for normal mapping (from reading)
float3 NormalMapCalculations(Texture2D normalFromTexture, SamplerState sampleState, float2 uv, float3 normalFromVS, float3 tangentFromVS)
{
    // normals from sample
    float3 unpackedNormal = normalFromTexture.Sample(sampleState, uv).rgb * 2.0f - 1.0f;
    
    // TBN matrix
    float3 N = normalize(normalFromVS);
    float3 T = normalize(tangentFromVS - dot(tangentFromVS, N) * N);
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    return mul(unpackedNormal, TBN);
}

// helper functions and lighting functions
float3 CalculateDiffusionTerm(float3 inputNormal, float3 lightDirection)
{
    return saturate(dot(inputNormal, lightDirection));
}

float CalculateSpecularTerm(float3 inputNormal, float3 lightDirection, float3 cameraDirection, float roughness)
{
    // use roughness
    float specularExp = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    
    // quick check if roughness is 1
    [branch]
    if (roughness == 1)
    {
        specularExp = 0.95f * MAX_SPECULAR_EXPONENT;
    }
    
    // view and reflection
    float3 R = reflect(-lightDirection, inputNormal);
    
    return pow(max(dot(cameraDirection, R), 0.0f), specularExp);
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

// PBR functions!!

static const float MIN_ROUGHNESS = 0.0000001;
static const float PI = 3.14159265359f;

// Normal Distribution:
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness)
{
    // allll the variables needed
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS);
    
    // ((n dot h)^2 * (a^2 - 1) + 1)
    float denomToSquare = NdotH2 * (a2 - 1) + 1;
    
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term (Schlick approximation):
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
    float VdotH = saturate(dot(v, h));
    
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing (Schlick-GGX):
// G_Schlick(n,v,a) = (n . v) / ((n . v) * (1 - k) * k)
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));
    return 1 / (NdotV * (1 - k) + k);
}

// Cook-Torrance BRDF:
// f(l, v) = D(h) F(v, h) G(l, v, h) / 4(n . 1)(n . v)
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0)
{
    float3 h = normalize(v + l);
    
    float NdotL = saturate(dot(n, l));
    float NdotV = saturate(dot(n, v));
    
    // run each function in numerator
    float D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
    
    // denominator with a tiny offset to avoid dividing by 0
    float denom = 4.0f * max(NdotV * NdotL, 0.001f);
    
    return (D * F * G) / denom;

}

// Conserving Energy
float3 DiffuseEnergyConserve(float3 albedo, float3 F, float metalness)
{
    return albedo * (1 - F) * (1 - metalness);
}

// big helper to do all pbr math for a light!
float3 EvaluatePBR(float3 n, float3 l, float3 v, float3 albedo, float3 f0, float roughness, float metalness)
{
    float NdotL = saturate(dot(n, l));
    float NdotV = saturate(dot(n, v));

    float3 h = normalize(v + l);

    // fresnel for this v/h pair
    float3 F = F_Schlick(v, h, f0);

    // diffuse BRDF with energy conservation
    float3 kdTimesAlbedo = DiffuseEnergyConserve(albedo, F, metalness);
    float3 diffuseBRDF = kdTimesAlbedo / PI;

    // specular BRDF
    float3 specularBRDF = MicrofacetBRDF(n, l, v, roughness, f0);

    // combine and apply NdotL once
    float3 radiance = (diffuseBRDF + specularBRDF) * NdotL;

    return radiance;
}

float3 DirectionalLight(Light dirLight, float3 inputNormal, float3 surfaceColor, float3 f0, float3 cameraPosition, float3 worldPosition, float roughness, float metalness)
{
    // light and camera direction
    float3 lightDirection = normalize(-dirLight.direction);
    float3 camDirection = normalize(cameraPosition - worldPosition);
    
    // get the light
    float3 radiance = EvaluatePBR(inputNormal, lightDirection, camDirection, surfaceColor, f0, roughness, metalness);
    
    // apply it
    return radiance * dirLight.intensity * dirLight.color;
}

float3 PointLight(Light pointLight, float3 inputNormal, float3 surfaceColor, float3 f0, float3 cameraPosition, float3 worldPosition, float roughness, float metalness)
{
    // light and camera direction
    float3 lightDirection = normalize(-pointLight.direction);
    float3 camDirection = normalize(cameraPosition - worldPosition);
    
    // get the light
    float3 radiance = EvaluatePBR(inputNormal, lightDirection, camDirection, surfaceColor, f0, roughness, metalness);
    
    // attenuation
    float atten = Attenuate(pointLight, worldPosition);
    
    return radiance * atten * pointLight.intensity * pointLight.color;
}

float3 SpotLight(Light spotLight, float3 inputNormal, float3 surfaceColor, float3 f0, float3 cameraPosition, float3 worldPosition, float roughness, float metalness)
{
    float3 lightDirection = normalize(spotLight.position - worldPosition);
    
    float angle = saturate(dot(worldPosition, lightDirection));
    
    // law of cosines using angle
    float cosOuter = cos(spotLight.spotOuterAngle);
    float cosInner = cos(spotLight.spotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    float spotTerm = saturate((cosOuter - angle) / falloffRange);
    
    return PointLight(spotLight, inputNormal, surfaceColor, f0, cameraPosition, worldPosition, roughness, metalness) * spotTerm;
}
#endif