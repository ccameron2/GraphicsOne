//--------------------------------------------------------------------------------------
// Per-Pixel Lighting Pixel Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli" // Shaders can also use include files - note the extension

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

Texture2D DiffuseSpecularMap : register(t0);
SamplerState TexSampler      : register(s0);

Texture2D DiffuseSpecularMap2 : register(t1);

Texture2D NormalMap : register(t2);

Texture2D NormalMap2 : register(t3);

float4 main(NormalPixelShaderInput input) : SV_Target
{
	//************************
	// Normal Map Extraction
	//************************

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
	float3 modelNormal = normalize(input.modelNormal);
	float3 modelTangent = normalize(input.modelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space. This is just a matrix built from the three axes (very advanced note - by default shader matrices
	// are stored as columns rather than in rows as in the C++. This means that this matrix is created "transposed" from what we would
	// expect. However, for a 3x3 rotation matrix the transpose is equal to the inverse, which is just what we require)
	float3 modelBiTangent = cross(modelNormal, modelTangent);
	float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);
	
	//****| INFO |**********************************************************************************//
// The following few lines are the parallax mapping. Converts the camera direction into model
// space and adjusts the UVs based on that and the bump depth of the texel we are looking at
// Although short, this code involves some intricate matrix work / space transformations
//**********************************************************************************************//

// Get normalised vector to camera for parallax mapping and specular equation (this vector was calculated later in previous shaders)
	float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

	// Transform camera vector from world into model space. Need *inverse* world matrix for this.
	// Only need 3x3 matrix to transform vectors, to invert a 3x3 matrix we transpose it (flip it about its diagonal)
	float3x3 invWorldMatrix = transpose((float3x3)gWorldMatrix);
	float3 cameraModelDir = normalize(mul(invWorldMatrix, cameraDirection)); // Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
	float3x3 tangentMatrix = transpose(invTangentMatrix);
	float2 textureOffsetDir = mul(cameraModelDir, tangentMatrix).xy;

	//textureOffsetDir.xy = 0;

	// Get the height info from the normal map's alpha channel at the given texture coordinate
	// Rescale from 0->1 range to -x->+x range, x determined by ParallaxDepth setting
	float textureHeight = gParallaxDepth * (NormalMap.Sample(TexSampler, input.uv).a - 0.5f);

	// Use the depth of the texture to offset the given texture coordinate - this corrected texture coordinate will be used from here on
	float2 offsetTexCoord = input.uv + textureHeight * textureOffsetDir;


	//*******************************************

	//****| INFO |**********************************************************************************//
	// The above chunk of code is used only to calculate "offsetTexCoord", which is the offset in 
	// which part of the texture we see at this pixel due to it being bumpy. The remaining code is 
	// exactly the same as normal mapping, but uses offsetTexCoord instead of the usual input.uv
	//**********************************************************************************************//

	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
	float3 textureNormal = 2.0f * NormalMap.Sample(TexSampler, offsetTexCoord).rgb - 1.0f; // Scale from 0->1 to -1->1

	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
	float3 worldNormal = normalize(mul((float3x3)gWorldMatrix, mul(textureNormal, invTangentMatrix)));
	
	///////////////////////
	// Calculate lighting
	
	//Light 3
	float3 light3Direction = normalize(gLight3Position - input.worldPosition.xyz);
	float3 light3Dist = length(gLight3Position - input.worldPosition);
	float3 diffuseLight3 = gLight3Colour * max(dot(worldNormal, light3Direction), 0) / light3Dist;
	
	float3 halfway = normalize(light3Direction + cameraDirection);
	float3 specularLight3 = gLight3Colour * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);
	
	float3 diffuseLight = gAmbientColour + diffuseLight3;
	float3 specularLight = specularLight3;
	
	////////////////////
	// Combine lighting and textures
	
	// Sample diffuse material and specular material colour for this pixel from textures using a given sampler that you set up in the C++ code
	float4 textureColour1 = DiffuseSpecularMap.Sample(TexSampler, offsetTexCoord);
	//float4 textureColour2 = DiffuseSpecularMap2.Sample(TexSampler, offsetTexCoord);
	//
	//
	//float sinWiggle = sin(wiggle * 0.1);
	//float maxWiggle = max(sinWiggle, 0);
	//
	////If sinWiggle is negative, make it positive.
	//if (maxWiggle == 0)
	//{
	//	sinWiggle = 0 - sinWiggle;
	//}
	////Cycle between two textures using wiggle variable.
	//float3 diffuseMaterialColour = lerp(textureColour1.rgb, textureColour2.rgb, sinWiggle); // Diffuse material colour in texture RGB (base colour of model)
	//float specularMaterialColour = lerp(textureColour1.a, textureColour2.a, sinWiggle);   // Specular material colour in texture A (shininess of the surface)
	//
	float3 diffuseMaterialColour = DiffuseSpecularMap.Sample(TexSampler, offsetTexCoord).rgb;
	float specularMaterialColour = DiffuseSpecularMap.Sample(TexSampler, offsetTexCoord).a;
	// Combine lighting with texture colours
	float3 finalColour = diffuseLight * diffuseMaterialColour + specularLight * specularMaterialColour;
	
	return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab
}