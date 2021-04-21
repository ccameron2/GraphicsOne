//--------------------------------------------------------------------------------------
// Depth-Only Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader that only outputs the depth of a pixel - used to render the scene from the point
// of view of the lights in preparation for shadow mapping

#include "Common.hlsli"


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Shader used when rendering the shadow map depths. The pixel shader doesn't really need to calculate a pixel colour
// since we are only writing to the depth buffer.
float4 main(SimplePixelShaderInput input) : SV_Target
{
	// Output the value that would go in the depth buffer to the pixel colour (greyscale)
    // The pow just rescales the values so they are easier to visualise. Although depth values range from 0 to 1 most
    // values are close to 1, which might give the (mistaken) impression that the depth buffer is empty
	return pow(input.projectedPosition.z, 20);
}
