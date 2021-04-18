//--------------------------------------------------------------------------------------
// Vertex shader for cell shading
//--------------------------------------------------------------------------------------

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D    DiffuseMap : register(t0); // Diffuse map only
Texture2D    CellMap    : register(t3); // CellMap is a 1D map that is used to limit the range of colours used in cell shading

Texture2D ShadowMapLight1 : register(t1); // Texture holding the view of the scene from a light
SamplerState PointClamp   : register(s1); // No filtering for shadow maps (you might think you could use trilinear or similar, but it will filter light depths not the shadows cast...)

Texture2D ShadowMapLight2 : register(t2);

SamplerState TexSampler       : register(s0); // Sampler for use on textures
SamplerState PointSampleClamp : register(s2); // No filtering of cell maps (otherwise the cell edges would be blurred)


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
float4 main(LightingPixelShaderInput input) : SV_Target
{
    // Lighting equations
    input.worldNormal = normalize(input.worldNormal); // Normal might have been scaled by model scaling or interpolation so renormalise
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

	const float DepthAdjust = 0.002f;

	// LIGHT 1

	float3 diffuseLight1 = 0; // Initialy assume no contribution from this light
	float3 specularLight1 = 0;

	float3 light1Vector = gLight1Position - input.worldPosition;
	float  light1Distance = length(light1Vector);
	float3 light1Direction = light1Vector / light1Distance; // Quicker than normalising as we have length for attenuation

	// Check if pixel is within light cone
	if (mul(-gLight1Facing, light1Direction) > gLight1CosHalfAngle)
	{
		// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
		// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
		// These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
		float4 light1ViewPosition = mul(gLight1ViewMatrix, float4(input.worldPosition, 1.0f));
		float4 light1Projection = mul(gLight1ProjectionMatrix, light1ViewPosition);

		// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
		// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
		float2 shadowMapUV = 0.5f * light1Projection.xy / light1Projection.w + float2(0.5f, 0.5f);
		shadowMapUV.y = 1.0f - shadowMapUV.y;	// Check if pixel is within light cone

		// Get depth of this pixel if it were visible from the light (another advanced projection step)
		float depthFromLight = light1Projection.z / light1Projection.w - DepthAdjust; //*** Adjustment so polygons don't shadow themselves

		// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		// to the light than this pixel - so the pixel gets no effect from this light
		if (depthFromLight < ShadowMapLight1.Sample(PointClamp, shadowMapUV).r)
		{
			float  diffuseLevel1 = max(dot(input.worldNormal, light1Direction), 0);
			float  cellDiffuseLevel1 = CellMap.Sample(PointSampleClamp, diffuseLevel1).r;
			float3 diffuseLight1 = gLight1Colour * cellDiffuseLevel1 / light1Distance;

			float3 halfway = normalize(light1Direction + cameraDirection);
			float3 specularLight1 = diffuseLight1 * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference
		}
	}
	// LIGHT 2
	//Duplicated from light 1
	float3 diffuseLight2 = 0;
	float3 specularLight2 = 0;

	float3 light2Vector = gLight2Position - input.worldPosition;
	float  light2Distance = length(light2Vector);
	float3 light2Direction = light2Vector / light2Distance;

	if (mul(-gLight2Facing, light2Direction) > gLight2CosHalfAngle)
	{
		float4 light2ViewPosition = mul(gLight2ViewMatrix, float4(input.worldPosition, 1.0f));
		float4 light2Projection = mul(gLight2ProjectionMatrix, light2ViewPosition);

		float2 shadowMapUV = 0.5f * light2Projection.xy / light2Projection.w + float2(0.5f, 0.5f);
		shadowMapUV.y = 1.0f - shadowMapUV.y;

		float depthFromLight = light2Projection.z / light2Projection.w - DepthAdjust;

		if (depthFromLight < ShadowMapLight2.Sample(PointClamp, shadowMapUV).r)
		{
			float  diffuseLevel2 = max(dot(input.worldNormal, light2Direction), 0);
			float  cellDiffuseLevel2 = CellMap.Sample(PointSampleClamp, diffuseLevel2).r;
			float3 diffuseLight2 = gLight2Colour * cellDiffuseLevel2 / light2Distance;

			float3 halfway = normalize(light2Direction + cameraDirection);
			float3 specularLight2 = diffuseLight2 * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference
		}
	}
    // Light 3
    float3 light3Vector = gLight3Position - input.worldPosition;
    float  light3Distance = length(light3Vector);
    float3 light3Direction = light3Vector / light3Distance; // Quicker than normalising as we have length for attenuation

    //****| INFO |*************************************************************************************//
    // To make a cartoon look to the lighting, we clamp the basic light level to just a small range of
    // colours. This is done by using the light level itself as the U texture coordinate to look up
    // a colour in a special 1D texture (a single line). This could be done with if statements, but
    // GPUs are much faster at looking up small textures than if statements
    //*************************************************************************************************//
    float  diffuseLevel3 = max(dot(input.worldNormal, light3Direction), 0);
    float  cellDiffuseLevel3 = CellMap.Sample(PointSampleClamp, diffuseLevel3).r;
    float3 diffuseLight3 = gLight3Colour * cellDiffuseLevel3 / light3Distance;

    float3 halfway = normalize(light3Direction + cameraDirection);
    float3 specularLight3 = diffuseLight3 * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower); // Multiplying by diffuseLight instead of light colour - my own personal preference


    // Light 4
    float3 light4Vector = gLight4Position - input.worldPosition;
    float  light4Distance = length(light4Vector);
    float3 light4Direction = light4Vector / light4Distance;

    float  diffuseLevel4 = max(dot(input.worldNormal, light4Direction), 0);
    float  cellDiffuseLevel4 = CellMap.Sample(PointSampleClamp, diffuseLevel4).r;
    float3 diffuseLight4 = gLight4Colour * cellDiffuseLevel4 / light4Distance;

    halfway = normalize(light4Direction + cameraDirection);
    float3 specularLight4 = diffuseLight4 * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);


    // Sample diffuse material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    // Ignoring any alpha in the texture, just reading RGB
    float4 textureColour = DiffuseMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb;
    float specularMaterialColour = textureColour.a;

    float3 finalColour = (gAmbientColour + diffuseLight1 + diffuseLight2 + diffuseLight3 + diffuseLight4) * diffuseMaterialColour +
                         (specularLight1 + specularLight2 + specularLight3 + specularLight4) * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for alpha - no alpha blending in this lab
}