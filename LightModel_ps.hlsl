
#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

Texture2D    DiffuseMap : register(t0); 
SamplerState TexSampler : register(s0);

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

float4 main(SimplePixelShaderInput input) : SV_Target
{
    // Sample diffuse material colour for this pixel from diffuse map.
    float3 diffuseMapColour = DiffuseMap.Sample(TexSampler, input.uv).rgb;

    // Blend texture colour with fixed per-object colour
    float3 finalColour = gObjectColour * diffuseMapColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for alpha
}