
#include "Common.hlsli" 


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

SimplePixelShaderInput main(BasicVertex modelVertex)
{
    SimplePixelShaderInput output;

    // Input position is x,y,z only - need a 4th element to multiply by a 4x4 matrix. Use 1 for a point (0 for a vector)
    float4 modelPosition = float4(modelVertex.position, 1); 

    // Multiply by the world matrix passed from C++ to transform the model vertex position into world space. 
    // In a similar way use the view matrix to transform the vertex from world space into view space (camera's point of view)
    // and then use the projection matrix to transform the vertex to 2D projection space (project onto the 2D screen)
    float4 worldPosition     = mul(gWorldMatrix,      modelPosition);
    float4 viewPosition      = mul(gViewMatrix,       worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);

    // Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
    output.uv = modelVertex.uv;

    return output; 
}
