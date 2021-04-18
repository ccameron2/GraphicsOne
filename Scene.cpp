//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

//New classes
#include "Light.h"
#include "Texture.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"

#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include "ColourRGBA.h" 

#include <sstream>
#include <memory>


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------
// Addition of Mesh, Model and Camera classes have greatly simplified this section
// Geometry data has gone to Mesh class. Positions, rotations, matrices have gone to Model and Camera classes
// Light and Texture classes have also been added. Textures contain both the diffuse and normal maps for the texture.
// Lights contain a colour, strength value and model.

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)


// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gFoxMesh;
Mesh* gCrateMesh;
Mesh* gGroundMesh;
Mesh* gSphereMesh;
Mesh* gLightMesh;
Mesh* gTeapotMesh;
Mesh* gCubeMesh;
Mesh* gTreeMesh;
Mesh* gBatMesh;
Mesh* gGlassCubeMesh;
Mesh* gSpriteMesh;
Mesh* gTankMesh;
Mesh* gHatMesh;
Mesh* gPotionMesh;
Mesh* gCatMesh;
Mesh* gTrunkMesh;
Mesh* gLeavesMesh;
Mesh* gTowerMesh;
Mesh* gGriffinMesh;
Mesh* gWizardMesh;
Mesh* gBoxMesh;
Mesh* gWellMesh;

//Array to hold all meshes. This allows for easy deletion.
const int NUM_MESHES = 22;
Mesh* gMeshes[NUM_MESHES] = { gFoxMesh, gCrateMesh, gGroundMesh, gSphereMesh, gLightMesh,
                              gTeapotMesh, gCubeMesh, gTreeMesh, gBatMesh, gGlassCubeMesh,
                              gSpriteMesh, gTankMesh, gHatMesh, gPotionMesh, gCatMesh, gTrunkMesh,
                              gLeavesMesh, gTowerMesh , gGriffinMesh, gWizardMesh, gBoxMesh, gWellMesh };

Model* gFox;
Model* gCrate;
Model* gGround;
Model* gSphere;
Model* gTeapot;
Model* gCube;
Model* gBat;
Model* gGlassCube;
Model* gSprite;
Model* gTank;
Model* gHat;
Model* gPotion;
Model* gCat;
Model* gTrunk;
Model* gLeaves;
Model* gTower;
Model* gGriffin;
Model* gWizard;
Model* gBox;
Model* gWell;
Model* gPortal;

//Array to hold all models, this allows for easy deletion
const int NUM_MODELS = 21;
Model* gModels[NUM_MODELS] = { gFox, gCrate, gGround, gSphere,
                              gTeapot, gCube, gBat, gGlassCube,
                              gSprite, gTank, gHat, gPotion, gCat, gTrunk,
                              gLeaves, gTower , gGriffin, gWizard, gBox, gWell, gPortal };

//Array to hold trees. This allows for easy placement in the scene setup
const int NUM_TREES = 10;
Model* gTrees[NUM_TREES];

//Array to hold bats for easy scene setup placement
const int NUM_BATS = 10;
Model* gBats[NUM_BATS];

Camera* gCamera;
Camera* gPortalCamera;

// Store lights in an array
const int NUM_LIGHTS = 4;
Light* gLights[NUM_LIGHTS];


// Additional light information
CVector3 gAmbientColour = { 0.2f, 0.2f, 0.3f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 4096; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f };

// Variables controlling light1's orbiting of the cube
const float gLightOrbit = 30.0f;
const float gLightOrbitSpeed = 0.7f;

float gSpotlightConeAngle = 90.0f; // Spot light cone angle (degrees)

// Lock FPS to monitor refresh rate, for me this is 165. Press 'p' to toggle to full fps
bool lockFPS = true;

//--------------------------------------------------------------------------------------
//**** Portal Texture  ****//
//--------------------------------------------------------------------------------------
// This texture will have the scene renderered on it. Then the texture is applied to a model

// Dimensions of portal texture - controls quality of rendered scene in portal
int gPortalWidth = 512;
int gPortalHeight = 512;

// The portal texture - each frame it is rendered to, then it is used as a texture for model
ID3D11Texture2D* gPortalTexture = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11RenderTargetView* gPortalRenderTarget = nullptr; // This object is used when we want to render to the texture above
ID3D11ShaderResourceView* gPortalTextureSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

// Also need a depth/stencil buffer for the portal - it's just another kind of texture
// NOTE: ***Can share this depth buffer between multiple portals of the same size***
ID3D11Texture2D* gPortalDepthStencil = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11DepthStencilView* gPortalDepthStencilView = nullptr; // This object is used when we want to use the texture above as the depth buffer

//--------------------------------------------------------------------------------------
//**** Shadow Texture  ****//
//--------------------------------------------------------------------------------------
// This texture will have the scene from the point of view of the light renderered on it. This texture is then used for shadow mapping

// Dimensions of shadow map textures - controls quality of shadows
int gShadowMapSize  = 1024;

// The shadow textures - effectively a depth buffer of the scene **from the light's point of view**
//                       Each frame it is rendered to, then the texture is used to help the per-pixel lighting shader identify pixels in shadow
ID3D11Texture2D*          gShadowMap1Texture      = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11DepthStencilView*   gShadowMap1DepthStencil = nullptr; // This object is used when we want to render to the texture above **as a depth buffer**
ID3D11ShaderResourceView* gShadowMap1SRV          = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

ID3D11Texture2D*          gShadowMap2Texture      = nullptr;
ID3D11DepthStencilView*   gShadowMap2DepthStencil = nullptr;
ID3D11ShaderResourceView* gShadowMap2SRV          = nullptr;


//*********************//



//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--



//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
const int NUM_TEXTURES = 25;

//Create texture objects, constructor can take up to two parameters, DiffuseSpecularMap name and NormalMap name.
Texture* gTrollTexture = new Texture("TrollDiffuseSpecular.dds");
Texture* gCargoTexture = new Texture("CargoA.dds");
Texture* gGrassTexture = new Texture("GrassDiffuseSpecular.dds");
Texture* gFlareTexture = new Texture("Flare.jpg");
Texture* gWoodTexture = new Texture("WoodDiffuseSpecular.dds", "WoodDiffuseSpecular.dds");
Texture* gTechTexture = new Texture("TechDiffuseSpecular.dds", "TechNormalHeight.dds");
Texture* gCobbleTexture = new Texture("CobbleDiffuseSpecular.dds", "CobbleNormalHeight.dds");
Texture* gBrainTexture = new Texture("BrainDiffuseSpecular.dds", "BrainNormalHeight.dds");
Texture* gPatternTexture = new Texture("PatternDiffuseSpecular.dds", "PatternNormalHeight.dds");
Texture* gFoxTexture = new Texture("fox.png");
Texture* gBatTexture = new Texture("Bat.png");
Texture* gWallTexture = new Texture("WallDiffuseSpecular.dds", "WallNormalHeight.dds");
Texture* gGlassTexture = new Texture("Glass.jpg");
Texture* gSpriteTexture = new Texture("wizard.jpg");
Texture* gMetalTexture = new Texture("MetalDiffuseSpecular.dds", "MetalNormal.dds");
Texture* gHatTexture = new Texture("hat.jpeg", "hatnormal.png");
Texture* gPotionTexture = new Texture("potion.png");
Texture* gTankTexture = new Texture("Tank.dds");
Texture* gCatTexture = new Texture("CatTexture.dds");
Texture* gTrunkTexture = new Texture("Trunk.png");
Texture* gLeavesTexture = new Texture("Leaves.png");
Texture* gGriffinTexture = new Texture("griffin.png");
Texture* gTowerTexture = new Texture("wizardTowerDiff.png");
Texture* gWizardTexture = new Texture("wizardDiff.png");
Texture* gTVTexture = new Texture("tv.png");

//Array to hold all textures. This allows for the loading of all textures and easy deletion.
Texture* gTextures[NUM_TEXTURES] = { gTrollTexture, gCargoTexture, gGrassTexture, gFlareTexture,
                                     gWoodTexture, gTechTexture, gCobbleTexture, gBrainTexture,
                                     gPatternTexture, gFoxTexture, gBatTexture, gWallTexture, 
                                     gGlassTexture, gSpriteTexture, gMetalTexture, gHatTexture,
                                     gPotionTexture, gTankTexture, gCatTexture , gTrunkTexture, 
                                     gLeavesTexture, gGriffinTexture, gTowerTexture, gWizardTexture,
                                     gTVTexture                                                     };


//Parallax variables
float gParallaxDepth = 0.1f;
bool gUseParallax = true;

//--------------------------------------------------------------------------------------
// Light Helper Functions
//--------------------------------------------------------------------------------------

// Get "camera-like" view matrix for a spotlight
CMatrix4x4 CalculateLightViewMatrix(int lightIndex)
{
    return InverseAffine(gLights[lightIndex]->GetModel()->WorldMatrix());
}

// Get "camera-like" projection matrix for a spotlight
CMatrix4x4 CalculateLightProjectionMatrix(int lightIndex)
{
    return MakeProjectionMatrix(1.0f, ToRadians(gSpotlightConeAngle)); // Helper function in Utility\GraphicsHelpers.cpp
}


//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
    // Load mesh geometry data, just like TL-Engine this doesn't create anything in the scene. Create a Model for that.
    try 
    {
        gFoxMesh       = new Mesh("Fox.fbx");
        gCrateMesh     = new Mesh("CargoContainer.x");
        gGroundMesh    = new Mesh("Hills.x");
        gSphereMesh    = new Mesh("Sphere.x", true);
        gLightMesh     = new Mesh("Light.x");
        gTeapotMesh    = new Mesh("Teapot.x",true);
        gCubeMesh      = new Mesh("Cube.x", true);
        gTreeMesh      = new Mesh("Tree.fbx");
        gBatMesh       = new Mesh("bat.fbx");
        gGlassCubeMesh = new Mesh("Cube.x");    
        gSpriteMesh    = new Mesh("portal.x");
        gTankMesh      = new Mesh("Tank.fbx");
        gHatMesh       = new Mesh("WizardHat.fbx", true);
        gPotionMesh    = new Mesh("potion.fbx");
        gCatMesh       = new Mesh("Cat.fbx");
        gTrunkMesh     = new Mesh("Trunk.fbx");
        gLeavesMesh    = new Mesh("Leaves.fbx");
        gGriffinMesh   = new Mesh("griffin.fbx");
        gTowerMesh     = new Mesh("Tower.fbx");
        gWizardMesh    = new Mesh("wizard.fbx");
        gBoxMesh       = new Mesh("box.fbx");
        gWellMesh      = new Mesh("well.fbx");

    }
    catch (std::runtime_error e)  // Constructors cannot return error messages so use exceptions to catch mesh errors
    {
        gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
        return false;
    }

    //Create light objects
    CreateLights();
    
    // Load the shaders required for the geometry we will use (see Shader.cpp / .h)
    if (!LoadShaders())
    {
        gLastError = "Error loading shaders";
        return false;
    }


    // Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
    // These allow us to pass data from CPU to shaders such as lighting information or matrices
    // See the comments above where these variable are declared and also the UpdateScene function
    gPerFrameConstantBuffer = CreateConstantBuffer(sizeof(gPerFrameConstants));
    gPerModelConstantBuffer = CreateConstantBuffer(sizeof(gPerModelConstants));
    if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr)
    {
        gLastError = "Error creating constant buffers";
        return false;
    }


    //// Load / prepare textures on the GPU ////

    // Load textures and create DirectX objects for them
    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        //Variables to hold diffuse maps
        ID3D11Resource* DiffuseSpecularMap = nullptr;
        ID3D11ShaderResourceView* DiffuseSpecularMapSRV = nullptr;
        std::string TextureName = gTextures[i]->GetTextureName();

        //Variables to hold normal maps
        ID3D11Resource* NormalMap = nullptr;
        ID3D11ShaderResourceView* NormalMapSRV = nullptr;
        std::string NormalName = gTextures[i]->GetNormalName();

        //Load the diffuse map textures
        if (!LoadTexture(TextureName, &DiffuseSpecularMap, &DiffuseSpecularMapSRV))
        {
            gLastError = "Error creating texture";
            return false;
        }
        //Load the normal map textures if there is a provided name for the file.
        if (NormalName != "")
        {
            if (!LoadTexture(NormalName, &NormalMap, &NormalMapSRV))
            {
                gLastError = "Error creating texture normal";
                return false;
            }
        }
        //Set textures
        gTextures[i]->SetDiffuseSpecularMap(DiffuseSpecularMap);
        gTextures[i]->SetDiffuseSpecularMapSRV(DiffuseSpecularMapSRV);
        gTextures[i]->SetNormalMap(NormalMap);
        gTextures[i]->SetNormalMapSRV(NormalMapSRV);
    }

    //**** Create Portal Texture ****//

 // Using a helper function to load textures from files above. Here we create the portal texture manually
 // as we are creating a special kind of texture (one that we can render to). Many settings to prepare:
    D3D11_TEXTURE2D_DESC portalDesc = {};
    portalDesc.Width = gPortalWidth;  // Size of the portal texture determines its quality
    portalDesc.Height = gPortalHeight;
    portalDesc.MipLevels = 1; // No mip-maps when rendering to textures (or we would have to render every level)
    portalDesc.ArraySize = 1;
    portalDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA texture (8-bits each)
    portalDesc.SampleDesc.Count = 1;
    portalDesc.SampleDesc.Quality = 0;
    portalDesc.Usage = D3D11_USAGE_DEFAULT;
    portalDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE; // IMPORTANT: Indicate we will use texture as render target, and pass it to shaders
    portalDesc.CPUAccessFlags = 0;
    portalDesc.MiscFlags = 0;
    if (FAILED(gD3DDevice->CreateTexture2D(&portalDesc, NULL, &gPortalTexture)))
    {
        gLastError = "Error creating portal texture";
        return false;
    }

    // We created the portal texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
    // we use when rendering to it (see RenderScene function below)
    if (FAILED(gD3DDevice->CreateRenderTargetView(gPortalTexture, NULL, &gPortalRenderTarget)))
    {
        gLastError = "Error creating portal render target view";
        return false;
    }

    // We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc = {};
    srDesc.Format = portalDesc.Format;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MostDetailedMip = 0;
    srDesc.Texture2D.MipLevels = 1;
    if (FAILED(gD3DDevice->CreateShaderResourceView(gPortalTexture, &srDesc, &gPortalTextureSRV)))
    {
        gLastError = "Error creating portal shader resource view";
        return false;
    }
    //**** Create Portal Depth Buffer ****//

// We also need a depth buffer to go with our portal
//**** This depth buffer can be shared with any other portals of the same size
    portalDesc = {};
    portalDesc.Width = gPortalWidth;
    portalDesc.Height = gPortalHeight;
    portalDesc.MipLevels = 1;
    portalDesc.ArraySize = 1;
    portalDesc.Format = DXGI_FORMAT_D32_FLOAT; // Depth buffers contain a single float per pixel
    portalDesc.SampleDesc.Count = 1;
    portalDesc.SampleDesc.Quality = 0;
    portalDesc.Usage = D3D11_USAGE_DEFAULT;
    portalDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    portalDesc.CPUAccessFlags = 0;
    portalDesc.MiscFlags = 0;
    if (FAILED(gD3DDevice->CreateTexture2D(&portalDesc, NULL, &gPortalDepthStencil)))
    {
        gLastError = "Error creating portal depth stencil texture";
        return false;
    }

    // Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC portalDescDSV = {};
    portalDescDSV.Format = portalDesc.Format;
    portalDescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    portalDescDSV.Texture2D.MipSlice = 0;
    portalDescDSV.Flags = 0;
    if (FAILED(gD3DDevice->CreateDepthStencilView(gPortalDepthStencil, &portalDescDSV, &gPortalDepthStencilView)))
    {
        gLastError = "Error creating portal depth stencil view";
        return false;
    }


    //*****************************//
    
	//**** Create Shadow Map texture ****//
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width  = gShadowMapSize; // Size of the shadow map determines quality / resolution of shadows
	textureDesc.Height = gShadowMapSize;
	textureDesc.MipLevels = 1; // 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS; // The shadow map contains a single 32-bit value [tech gotcha: have to say typeless because depth buffer and shaders see things slightly differently]
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE; // Indicate we will use texture as a depth buffer and also pass it to shaders
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &gShadowMap1Texture) ))
	{
		gLastError = "Error creating shadow map texture";
		return false;
	}
    if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &gShadowMap2Texture)))
    {
        gLastError = "Error creating shadow map texture";
        return false;
    }
	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // See "tech gotcha" above. The depth buffer sees each pixel as a "depth" float
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = 0;
	if (FAILED(gD3DDevice->CreateDepthStencilView(gShadowMap1Texture, &dsvDesc, &gShadowMap1DepthStencil) ))
	{
		gLastError = "Error creating shadow map depth stencil view";
		return false;
	}
    if (FAILED(gD3DDevice->CreateDepthStencilView(gShadowMap2Texture, &dsvDesc, &gShadowMap2DepthStencil)))
    {
        gLastError = "Error creating shadow map depth stencil view";
        return false;
    }

   
 	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // See "tech gotcha" above. The shaders see textures as colours, so shadow map pixels are not seen as depths
                                           // but rather as "red" floats (one float taken from RGB). Although the shader code will use the value as a depth
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	if (FAILED(gD3DDevice->CreateShaderResourceView(gShadowMap1Texture, &srvDesc, &gShadowMap1SRV) ))
	{
		gLastError = "Error creating shadow map shader resource view";
		return false;
	}
    if (FAILED(gD3DDevice->CreateShaderResourceView(gShadowMap2Texture, &srvDesc, &gShadowMap2SRV)))
    {
        gLastError = "Error creating shadow map shader resource view";
        return false;
    }

   //*****************************//


  	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}

	return true;
}


// Prepare the scene
// Returns true on success
bool InitScene()
{
    //// Set up scene ////

    gFox       = new Model(gFoxMesh);
    gCrate     = new Model(gCrateMesh);
    gGround    = new Model(gGroundMesh);
    gSphere    = new Model(gSphereMesh);
    gTeapot    = new Model(gTeapotMesh);
    gCube      = new Model(gCubeMesh);
    gGlassCube = new Model(gGlassCubeMesh);
    gSprite    = new Model(gSpriteMesh);
    gTank      = new Model(gTankMesh);
    gHat       = new Model(gHatMesh);
    gPotion    = new Model(gPotionMesh);
    gCat       = new Model(gCatMesh);
    gTrunk     = new Model(gTrunkMesh);
    gLeaves    = new Model(gLeavesMesh);
    gGriffin   = new Model(gGriffinMesh);
    gTower     = new Model(gTowerMesh);
    gWizard    = new Model(gWizardMesh);
    gBox       = new Model(gBoxMesh);
    gWell      = new Model(gWellMesh);
    gPortal    = new Model(gSpriteMesh);

    //Bats
    for (int i = 0; i < NUM_BATS; i++)
    {
        gBats[i] = new Model(gBatMesh);
        gBats[i]->SetPosition({ -130 + 20 * sin(i * 10.0f), 24 ,150 + 20 * cos(i * 10.0f) });
        gBats[i]->SetScale(0.1);
    }
    //Trees
    for (int i = 0; i < NUM_TREES; i++)
    {
        gTrees[i] = new Model(gTreeMesh);
        gTrees[i]->SetPosition({ -170 , 3, 100 + i * 10.0f });
        gTrees[i]->SetScale(0.06);
    }

	// Initial positions
	gFox->SetPosition({ -135, 2, 150 });
    gFox->SetScale(0.2);
    gFox->SetRotation({ 0, ToRadians(220), 0 });
	gCrate->SetPosition({ 58, 4, 100 });
	gCrate->SetScale(6);
	gCrate->SetRotation({ 0.0f, ToRadians(-180.0f), 0.0f });
    gSphere->SetPosition({ 70, 20, 10 });
    gTeapot->SetPosition({ 40, 5, 70 });
    gCube->SetPosition({ 40, 15, 10 });
    gCube->SetScale(2);
    gGlassCube->SetPosition({ 30, 25, -110 });
    gGlassCube->SetScale(3);
    gSprite->SetPosition({ 80, 25, -140 });
    gSprite->SetScale(0.8);
    gTank->SetPosition({ 80, 5, -110 });
    gTank->SetScale(0.05);
    gTank->SetRotation({ 0.0f, ToRadians(-180.0f), 0.0f });
    gHat->SetPosition(gFox->Position() + CVector3{5, 17.5f, 6.2f });
    gHat->SetRotation({ ToRadians(-6.0f), 0.0f , ToRadians(10.0f) });
    gHat->SetScale(11);
    gCat->SetPosition({ -125, 2, 145 });
    gCat->SetRotation({ 0.0f, ToRadians(-100.0f), 0.0f });
    gCat->SetScale(0.013);
    gPotion->SetPosition(gCat->Position() + CVector3{ 5.8f, 4.0f, 1.0f });
    gPotion->SetScale(0.01);
    gTrunk->SetScale(0.15);
    gLeaves->SetScale(gTrunk->Scale());
    gTrunk->SetPosition({-140,2,188});
    gLeaves->SetPosition(gTrunk->Position() + CVector3{ 0, 30.0f, 0 });
    gLeaves->SetRotation({ 0,ToRadians(180) ,0 });
    gGriffin->SetPosition({-160,80,100});
    gGriffin->SetRotation({ 0,ToRadians(240) ,0 });
    gGriffin->SetScale(0.1);
    gTower->SetPosition({ -117.0f,22.0f,30.6f });
    gTower->SetRotation({ ToRadians(-5.0f), ToRadians(-200.0f), 0.0F });
    gTower->SetScale(0.1);
    gWizard->SetScale(0.1);
    gWizard->SetPosition({ -143.1f,7.0f,96.5f });
    gWizard->SetRotation({ 0.0f, ToRadians(-140.0f), 0.0F });
    gBox->SetScale(0.1);
    gBox->SetPosition({ -93,28,-2 });
    gBox->SetRotation({ 0,ToRadians(180),0 });
    gWell->SetScale(0.1);
    gWell->SetPosition({ -58.1f ,4.6f,180.7f });
    gPortal->SetPosition({ 80, 60, -140 });

    // Light set-up
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        Model* lightModel = new Model(gLightMesh);
        gLights[i]->SetModel(lightModel);
    }

    gLights[0]->SetColour(CVector3{ 0.8f, 0.8f, 1.0f });
    gLights[0]->SetStrength(10);
    gLights[0]->GetModel()->SetPosition({ 30, 28, 0 });
    gLights[0]->GetModel()->SetScale(pow(gLights[0]->GetStrength(), 0.7f)); // Convert light strength into a nice value for the scale of the light - equation is ad-hoc.
	gLights[0]->GetModel()->FaceTarget(gFox->Position());
                
    gLights[1]->SetColour(CVector3{ 1.0f, 0.8f, 0.2f });
    gLights[1]->SetStrength(50);
    gLights[1]->GetModel()->SetPosition({ -15, 60, 120 });
    gLights[1]->GetModel()->SetScale(pow(gLights[1]->GetStrength(), 0.7f));
	gLights[1]->GetModel()->FaceTarget({ gTeapot->Position() });
                
    gLights[2]->SetColour(CVector3{ 1.0f, 0.8f, 0.2f });
    gLights[2]->SetStrength(25);
    gLights[2]->GetModel()->SetPosition({ 50, 80, -110 });
    gLights[2]->GetModel()->SetScale(pow(gLights[2]->GetStrength(), 0.7f));

    gLights[3]->SetColour(CVector3{ 1.0f, 0.8f, 0.2f });
    gLights[3]->SetStrength(25);
    gLights[3]->GetModel()->SetPosition({ -120, 80, 130 });
    gLights[3]->GetModel()->SetScale(pow(gLights[3]->GetStrength(), 0.7f));

    //// Set up camera ////

    gCamera = new Camera();
    gCamera->SetPosition({ 25, 30, 160 });
    gCamera->SetRotation({ ToRadians(10), ToRadians(180), 0 });

    //Set up portal camera
    gPortalCamera = new Camera();
    gPortalCamera->SetPosition({ -110, 12, 185 });
    gPortalCamera->SetRotation({ ToRadians(-10), ToRadians(210), 0 });

    return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
    ReleaseStates();

    if (gShadowMap1DepthStencil)  gShadowMap1DepthStencil->Release();
    if (gShadowMap1SRV)           gShadowMap1SRV->Release();
    if (gShadowMap1Texture)       gShadowMap1Texture->Release();
    if (gShadowMap2DepthStencil)  gShadowMap2DepthStencil->Release();
    if (gShadowMap2SRV)           gShadowMap2SRV->Release();
    if (gShadowMap2Texture)       gShadowMap2Texture->Release();
    if (gPortalDepthStencilView)  gPortalDepthStencilView->Release();
    if (gPortalDepthStencil)      gPortalDepthStencil->Release();
    if (gPortalTextureSRV)        gPortalTextureSRV->Release();
    if (gPortalRenderTarget)      gPortalRenderTarget->Release();
    if (gPortalTexture)           gPortalTexture->Release();

    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        if (gTextures[i]->GetDiffuseSpecularMap() && gTextures[i]->GetDiffuseSpecularMapSRV())
        {
            gTextures[i]->GetDiffuseSpecularMap()->Release();
            gTextures[i]->GetDiffuseSpecularMapSRV()->Release();
        }
    }

    if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
    if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();

    ReleaseShaders();

    // See note in InitGeometry about why we're not using unique_ptr and having to manually delete
    delete gCamera;        gCamera = nullptr;
    delete gPortalCamera;  gPortalCamera = nullptr;

    //Delete light models
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        delete gLights[i]->GetModel();  gLights[i]->SetModel(nullptr);
    }

    //Delete Unique Models
    for (int i = 0; i < NUM_MODELS; i++)
    {
        delete gModels[i]; gModels[i] = nullptr;
    }

    //Delete Trees
    for (int i = 0; i < NUM_TREES; i++)
    {
        delete gTrees[i]; gTrees[i] = nullptr;
    }

    //Delete Bats
    for (int i = 0; i < NUM_BATS; i++)
    {
        delete gBats[i]; gTrees[i] = nullptr;
    }

    //Delete Meshes
    for (int i = 0; i < NUM_MODELS; i++)
    {
        delete gMeshes[i]; gMeshes[i] = nullptr;
    }

}



//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render the scene from the given light's point of view. Only renders depth buffer
void RenderDepthBufferFromLight(int lightIndex)
{
    // Get camera-like matrices from the spotlight, seet in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix           = CalculateLightViewMatrix(lightIndex);
    gPerFrameConstants.projectionMatrix     = CalculateLightProjectionMatrix(lightIndex);
    gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


    //// Only render models that cast shadows ////

    // Use special depth-only rendering shaders
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gDepthOnlyPixelShader,       nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);

    // Render models - no state changes required between each object in this situation (no textures used in this step)
    gGround->Render();
    gFox->Render();
    gCrate->Render();
    gSphere->Render();
    gTeapot->Render();
    gCube->Render();
    for (int i = 0; i < NUM_TREES; i++)
    {
        gTrees[i]->Render();
    }
    for (int i = 0; i < NUM_BATS; i++)
    {
        gBats[i]->Render();
    }
    gGlassCube->Render();
    gSprite->Render();
    gTank->Render();
    gHat->Render();
    gPotion->Render();
    gCat->Render();
    gTrunk->Render();
    gLeaves->Render();
    gGriffin->Render();
    gTower->Render();
    gWizard->Render();
    gBox->Render();
    gWell->Render();
    gPortal->Render();
}



// Render everything in the scene from the given camera
// This code is common between rendering the main scene and rendering the scene in the portal
// See RenderScene function below
void RenderSceneFromCamera(Camera* camera)
{
    // Set camera matrices in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
    gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


    //// Render lit models ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gPixelLightingPixelShader,  nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);


    // Select the approriate textures and sampler to use in the pixel shader
    ID3D11ShaderResourceView* grassDiffuseSpecularMapSRV = gGrassTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &grassDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // Render model - it will update the model's world matrix and send it to the GPU in a constant buffer, then it will call
    // the Mesh render function, which will set up vertex & index buffer before finally calling Draw on the GPU
    gGround->Render();

    // Render other lit models, only change textures for each onee
        //Render trees
    ID3D11ShaderResourceView* treeDiffuseSpecularMapSRV = gGrassTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &treeDiffuseSpecularMapSRV);

    for (int i = 0; i < NUM_TREES; i++)
    {
        gTrees[i]->Render();
    }
    //Render bats;
    ID3D11ShaderResourceView* batDiffuseSpecularMapSRV = gBatTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &batDiffuseSpecularMapSRV);

    for (int i = 0; i < NUM_BATS; i++)
    {
        gBats[i]->Render();
    }

    //Render Fox
    ID3D11ShaderResourceView* foxDiffuseSpecularMapSRV = gFoxTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &foxDiffuseSpecularMapSRV);
    gFox->Render();

    //Render Trunk
    ID3D11ShaderResourceView* trunkDiffuseSpecularMapSRV = gTrunkTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &trunkDiffuseSpecularMapSRV);
    gTrunk->Render();
    
    //Render Leaves
    ID3D11ShaderResourceView* leavesDiffuseSpecularMapSRV = gLeavesTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &leavesDiffuseSpecularMapSRV);
    gLeaves->Render();

    //Render Crate
    ID3D11ShaderResourceView* crateDiffuseSpecularMapSRV = gCargoTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &crateDiffuseSpecularMapSRV);
    gCrate->Render();

    //Render Tank
    ID3D11ShaderResourceView* tankDiffuseSpecularMapSRV = gTankTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &tankDiffuseSpecularMapSRV);
    gTank->Render();

    //Render Cat
    ID3D11ShaderResourceView* catDiffuseSpecularMapSRV = gCatTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &catDiffuseSpecularMapSRV);
    gCat->Render();

    //Render Griffin
    ID3D11ShaderResourceView* griffinDiffuseSpecularMapSRV = gGriffinTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &griffinDiffuseSpecularMapSRV);
    gGriffin->Render();

    //Render Tower
    ID3D11ShaderResourceView* towerDiffuseSpecularMapSRV = gTowerTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &towerDiffuseSpecularMapSRV);
    gTower->Render();

    //Render Wizard
    ID3D11ShaderResourceView* wizardDiffuseSpecularMapSRV = gWizardTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &wizardDiffuseSpecularMapSRV);
    gWizard->Render();

    //Render Box
    ID3D11ShaderResourceView* boxDiffuseSpecularMapSRV = gTowerTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &boxDiffuseSpecularMapSRV);
    gBox->Render();

    //Render Well
    ID3D11ShaderResourceView* wellDiffuseSpecularMapSRV = gWizardTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &wellDiffuseSpecularMapSRV);
    gWell->Render();

    //Set Portal Shader
    gD3DContext->PSSetShader(gTVPixelShader, nullptr, 0);

    //Render Portal
    gD3DContext->PSSetShaderResources(0, 1, &gPortalTextureSRV);
    ID3D11ShaderResourceView* tvDiffuseSpecularMapSRV = gTVTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(3, 1, &tvDiffuseSpecularMapSRV);
    gPortal->Render();

    //Set Normal Mapping Shaders
    gD3DContext->VSSetShader(gParallaxMappingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gNormalMappingPixelShader, nullptr, 0);

    //Render Hat
    ID3D11ShaderResourceView* hatDiffuseSpecularMapSRV = gHatTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &hatDiffuseSpecularMapSRV);

    ID3D11ShaderResourceView* hatNormalMapSRV = gHatTexture->GetNormalMapSRV();
    gD3DContext->PSSetShaderResources(3, 1, &hatNormalMapSRV);

    gHat->Render();


    //Set Parallax Mapping Shaders
    gD3DContext->VSSetShader(gParallaxMappingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gParallaxMappingPixelShader, nullptr, 0);
    
    //Render Teapot
    ID3D11ShaderResourceView* teapotDiffuseSpecularMapSRV = gTechTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &teapotDiffuseSpecularMapSRV);

    ID3D11ShaderResourceView* teapotNormalMapSRV = gTechTexture->GetNormalMapSRV();
    gD3DContext->PSSetShaderResources(3, 1, &teapotNormalMapSRV);

    gTeapot->Render();

    //Set Sphere Shaders
    gD3DContext->VSSetShader(gSphereVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gSpherePixelShader, nullptr, 0);

    //Render Sphere
    ID3D11ShaderResourceView* sphereDiffuseSpecularMapSRV = gBrainTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &sphereDiffuseSpecularMapSRV);
    ID3D11ShaderResourceView* sphereNormalHeightMapSRV = gBrainTexture->GetNormalMapSRV();
    gD3DContext->PSSetShaderResources(3, 1, &sphereNormalHeightMapSRV);
    gSphere->Render();
    
    //Set Cube Shaders
    gD3DContext->VSSetShader(gParallaxMappingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gCubePixelShader, nullptr, 0);

    //Render Cube
    ID3D11ShaderResourceView* cubeDiffuseSpecularMapSRV = gWallTexture->GetDiffuseSpecularMapSRV();
    ID3D11ShaderResourceView* cube2DiffuseSpecularMapSRV = gCobbleTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &cubeDiffuseSpecularMapSRV);
    gD3DContext->PSSetShaderResources(3, 1, &cube2DiffuseSpecularMapSRV);

    //Render Cube
    ID3D11ShaderResourceView* cubeNormalMapSRV =  gWallTexture->GetNormalMapSRV();
    ID3D11ShaderResourceView* cube2NormalMapSRV = gCobbleTexture->GetNormalMapSRV();
    gD3DContext->PSSetShaderResources(4, 1, &cubeNormalMapSRV);
    gD3DContext->PSSetShaderResources(5, 1, &cube2NormalMapSRV);

    gCube->Render();

    //Set Alpha Testing shader
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gSpritePixelShader, nullptr, 0);
   

    //Render Wizard
    ID3D11ShaderResourceView* spriteDiffuseSpecularMapSRV = gSpriteTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &spriteDiffuseSpecularMapSRV);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gSprite->Render();

    //Multiplicative Blending
    // Select which shaders to use next
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);

    //Set blend state
    gD3DContext->OMSetBlendState(gMultiplicativeBlending, nullptr, 0xffffff);

    //Render Potion
    ID3D11ShaderResourceView* potionDiffuseSpecularMapSRV = gPotionTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &potionDiffuseSpecularMapSRV);
    gPotion->Render();

    //Render glass cube
    ID3D11ShaderResourceView* glassCubeDiffuseSpecularMapSRV = gGlassTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &glassCubeDiffuseSpecularMapSRV);
    gGlassCube->Render();

    //// Render lights ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gLightModelPixelShader,      nullptr, 0);

    // Select the texture and sampler to use in the pixel shader
    ID3D11ShaderResourceView* lightDiffuseSpecularMapSRV = gFlareTexture->GetDiffuseSpecularMapSRV();
    gD3DContext->PSSetShaderResources(0, 1, &lightDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shaer
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // States - additive blending, read-only depth buffer and no culling
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);

    // Render all the lights in the array
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gPerModelConstants.objectColour = gLights[i]->GetColour(); // Set any per-model constants apart from the world matrix just before calling render (light colour here)
        gLights[i]->GetModel()->Render();
    }
}




// Rendering the scene now renders everything twice. First it renders the scene for the portal into a texture.
// Then it renders the main scene using the portal texture on a model.
void RenderScene()
{
    //// Common settings ////

    // Set up the light information in the constant buffer
    // Don't send to the GPU yet, the function RenderSceneFromCamera will do that
    gPerFrameConstants.light1Colour   =         gLights[0]->GetColour() * gLights[0]->GetStrength();
    gPerFrameConstants.light1Position =         gLights[0]->GetModel()->Position();
    gPerFrameConstants.light1Facing   =         Normalise(gLights[0]->GetModel()->WorldMatrix().GetZAxis());    // Additional lighting information for spotlights
    gPerFrameConstants.light1CosHalfAngle =     cos(ToRadians(gSpotlightConeAngle / 2)); // --"--
    gPerFrameConstants.light1ViewMatrix       = CalculateLightViewMatrix(0);         // Calculate camera-like matrices for...
    gPerFrameConstants.light1ProjectionMatrix = CalculateLightProjectionMatrix(0);   //...lights to support shadow mapping

    gPerFrameConstants.light2Colour =           gLights[1]->GetColour() * gLights[1]->GetStrength();
    gPerFrameConstants.light2Position =         gLights[1]->GetModel()->Position();
    gPerFrameConstants.light2Facing =           Normalise(gLights[1]->GetModel()->WorldMatrix().GetZAxis());    // Additional lighting information for spotlights
    gPerFrameConstants.light2CosHalfAngle =     cos(ToRadians(gSpotlightConeAngle / 2)); // --"--
    gPerFrameConstants.light2ViewMatrix =       CalculateLightViewMatrix(1);         // Calculate cara-like matrices for...
    gPerFrameConstants.light2ProjectionMatrix = CalculateLightProjectionMatrix(1);   //...lights to pport shadow mapping

    gPerFrameConstants.light3Colour =           gLights[2]->GetColour() * gLights[2]->GetStrength();
    gPerFrameConstants.light3Position =         gLights[2]->GetModel()->Position();

    gPerFrameConstants.light4Colour = gLights[3]->GetColour() * gLights[3]->GetStrength();
    gPerFrameConstants.light4Position = gLights[3]->GetModel()->Position();

    gPerFrameConstants.ambientColour  = gAmbientColour;
    gPerFrameConstants.specularPower  = gSpecularPower;
    gPerFrameConstants.cameraPosition = gCamera->Position();
    gPerFrameConstants.parallaxDepth = (gUseParallax ? gParallaxDepth : 0);


    //***************************************//
    //// Render from light's point of view ////
    
    // Only rendering from light 1 to begin with

    // Setup the viewport to the size of the shadow map texture
    D3D11_VIEWPORT vp;
    vp.Width  = static_cast<FLOAT>(gShadowMapSize);
    vp.Height = static_cast<FLOAT>(gShadowMapSize);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
    // Also clear the the shadow map depth buffer to the far distance
    gD3DContext->OMSetRenderTargets(0, nullptr, gShadowMap1DepthStencil);
    gD3DContext->ClearDepthStencilView(gShadowMap1DepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Render the scene from the point of view of light 1 (only depth values written)
    RenderDepthBufferFromLight(0);

    // Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
    // Also clear the the shadow map depth buffer to the far distance
    gD3DContext->OMSetRenderTargets(0, nullptr, gShadowMap2DepthStencil);
    gD3DContext->ClearDepthStencilView(gShadowMap2DepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Render the scene from the point of view of light 2 (only depth values written)
    RenderDepthBufferFromLight(1);

    //**************************//


    //// Portal scene rendering ////

    // Set the portal texture and portal depth buffer as the targets for rendering
    // The portal texture will later be used on models in the main scene
    gD3DContext->OMSetRenderTargets(1, &gPortalRenderTarget, gPortalDepthStencilView);

    // Clear the portal texture to a fixed colour and the portal depth buffer to the far distance
    gD3DContext->ClearRenderTargetView(gPortalRenderTarget, &gBackgroundColor.r);
    gD3DContext->ClearDepthStencilView(gPortalDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Setup the viewport for the portal texture size
    vp.Width = static_cast<FLOAT>(gPortalWidth);
    vp.Height = static_cast<FLOAT>(gPortalHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Render the scene for the portal
    RenderSceneFromCamera(gPortalCamera);

    //// Main scene rendering ////

    // Set the back buffer as the target for rendering and select the main depth buffer.
    // When finished the back buffer is sent to the "front buffer" - which is the monitor.
    gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

    // Clear the back buffer to a fixed colour and the depth buffer to the far distance
    gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
    gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Setup the viewport to the size of the main window
    vp.Width  = static_cast<FLOAT>(gViewportWidth);
    vp.Height = static_cast<FLOAT>(gViewportHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Set shadow maps in shaders
    // First parameter is the "slot", must match the Texture2D declaration in the HLSL code
    // In this app the diffuse map uses slot 0, the shadow maps use slots 1 onwards. If we were using other maps (e.g. normal map) then
    // we might arrange things differently
    gD3DContext->PSSetShaderResources(1, 1, &gShadowMap1SRV);
    gD3DContext->PSSetShaderResources(2, 1, &gShadowMap2SRV);
    gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);

    // Render the scene for the main window
    RenderSceneFromCamera(gCamera);

    // Unbind shadow maps from shaders - prevents warnings from DirectX when we try to render to the shadow maps again next frame
    ID3D11ShaderResourceView* nullView = nullptr;
    gD3DContext->PSSetShaderResources(1, 1, &nullView);

    //// Scene completion ////

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    // Set first parameter to 1 to lock to vsync (typically 60fps)
    gSwapChain->Present(lockFPS ? 1 : 0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
	// Control sphere (will update its world matrix)
	gFox->Control(frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma );

    // Orbit the light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static float rotate = 0.0f;
    static bool go = true;
	gLights[0]->GetModel()->SetPosition( gFox->Position() + CVector3{ cos(rotate) * gLightOrbit, 20, sin(rotate) * gLightOrbit } );
	gLights[0]->GetModel()->FaceTarget(gFox->Position());
    if (go)  rotate -= gLightOrbitSpeed * frameTime;
    if (KeyHit(Key_1))  go = !go;

    //Create create wiggle variable
    gPerFrameConstants.wiggle += 6 * frameTime;

    //Pulse light 1 on and off
    static bool lightOn = true;

    float lightStrength = gLights[0]->GetStrength();
    if (lightOn)
    {       
        gLights[0]->SetStrength(lightStrength += 0.05) ;
        if (gLights[0]->GetStrength() >= 20)
        {                          
            lightOn = false;
        }
    }
    else
    {
        gLights[0]->SetStrength(lightStrength += -0.05);
        if (gLights[0]->GetStrength() <= 0.05)
        {
            lightOn = true;
        }
    }
    gLights[0]->GetModel()->SetScale(pow(lightStrength, 0.7f));

    //Change light 2 colour
    static float r = 0.2;
    static float g = 0.2;
    static float b = 0.2;
    static bool redCycle = false;
    static bool blueCycle = false;
    static bool greenCycle = false;

    if (!redCycle)
    {
        r = r + 0.001;
        if (b > 0.2)
        {
            b = b - 0.001;
        }
        if (r >= 1)
        {
            redCycle = true;
        }
    }
    else if (!greenCycle)
    {
        g = g + 0.001;
        if (r > 0.2)
        {
            r = r - 0.001;
        }
        if (g >= 1)
        {
            greenCycle = true;
        }
    }
    else if (!blueCycle)
    {
        b = b + 0.001;
        if (g > 0.2)
        {
            g = g - 0.001;
        }
        if (b >= 1)
        {
            blueCycle = true;
        }
    }
    else if (redCycle && greenCycle && blueCycle)
    {
        redCycle = false;
        greenCycle = false;
        blueCycle = false;
    }

    gLights[1]->SetColour(CVector3{ r, g, b });

    // Toggle parallax
    if (KeyHit(Key_2))
    {
        gUseParallax = !gUseParallax;
    }


	// Control camera (will update its view matrix)
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D );


    // Toggle FPS limiting
    if (KeyHit(Key_P))  lockFPS = !lockFPS;

    // Show frame time / FPS in the window title //
    const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
    static float totalFrameTime = 0;
    static int frameCount = 0;
    totalFrameTime += frameTime;
    ++frameCount;
    if (totalFrameTime > fpsUpdateTime)
    {
        // Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
        float avgFrameTime = totalFrameTime / frameCount;
        std::ostringstream frameTimeMs;
        frameTimeMs.precision(2);
        frameTimeMs << std::fixed << avgFrameTime * 1000;
        std::string windowTitle = "CO2409 Week 20: Shadow Mapping - Frame Time: " + frameTimeMs.str() +
                                  "ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f)) + ", XPos: " + std::to_string(gCamera->Position().x) +
                                   ", YPos: " + std::to_string(gCamera->Position().y) + ", ZPos: " + std::to_string(gCamera->Position().z) + 
                                   ", XRot: "+ std::to_string(ToDegrees(gCamera->Rotation().x)) + ", YRot: " + std::to_string(ToDegrees(gCamera->Rotation().y)) +
                                   ", ZRot: " + std::to_string(ToDegrees(gCamera->Rotation().z));
        SetWindowTextA(gHWnd, windowTitle.c_str());
        totalFrameTime = 0;
        frameCount = 0;
    }
}

void CreateLights()//Create light objects
{
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        Light* light = new Light();
        gLights[i] = light;
    }
}