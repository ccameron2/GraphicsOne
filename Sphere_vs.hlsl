// Light Model Vertex Shader

#include "Common.hlsli"


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

NormalPixelShaderInput main(TangentVertex modelVertex)
{
    NormalPixelShaderInput output; 

    // Input position is x,y,z only - need a 4th element to multiply by a 4x4 matrix. Use 1 for a point (0 for a vector)
    float4 modelPosition = float4(modelVertex.position, 1);

    // Multiply by the world matrix passed from C++ to transform the model vertex position into world space. 
    float4 worldPosition = mul(gWorldMatrix, modelPosition);

    //Transform model normals into world space using world matrix to calculate per pixel lighting
    float4 modelNormal = float4(modelVertex.normal, 0);
    float4 worldNormal = mul(gWorldMatrix, modelNormal);

    //Set world position to be offset by wiggle variable
    worldPosition.x += sin(modelPosition.y + wiggle) * 0.3f;
    worldPosition.y += sin(modelPosition.x + wiggle) * 0.3f;
    worldPosition += worldNormal * (sin(wiggle) + 1.0f) * 0.3f;

    // In a similar way use the view matrix to transform the vertex from world space into view space (camera's point of view)
    // and then use the projection matrix to transform the vertex to 2D projection space (project onto the 2D screen)
    float4 viewPosition = mul(gViewMatrix, worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);

    output.worldPosition = worldPosition.xyz; // Also pass world position to pixel shader for lighting

    // Unlike the position, send the model's normal and tangent untransformed (in model space). The pixel shader will do the matrix work on normals
    output.modelNormal = modelVertex.normal;
    output.modelTangent = modelVertex.tangent; // Also pass world position to pixel shader for lighting


    // Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
    output.uv = modelVertex.uv;

    return output; // Ouput data sent down the pipeline
}
