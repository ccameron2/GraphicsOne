#pragma once
#include "GraphicsHelpers.h"
#include <string.h>
class Texture
{
private:
	ID3D11Resource* mDiffuseSpecularMap = nullptr;
	ID3D11ShaderResourceView* mDiffuseSpecularMapSRV = nullptr;
	std::string mTextureName;
public:
	Texture(std::string TextureName) : mTextureName(TextureName)	
	{		
	}
	std::string GetTextureName() { return mTextureName; }
	void SetTextureName(std::string TextureName) { mTextureName = TextureName; }
	ID3D11Resource* GetDiffuseSpecularMap() { return mDiffuseSpecularMap; }
	void SetDiffuseSpecularMap(ID3D11Resource* DiffuseSpecularMap) { mDiffuseSpecularMap = DiffuseSpecularMap; }
	ID3D11ShaderResourceView* GetDiffuseSpecularMapSRV() { return mDiffuseSpecularMapSRV; }
	void SetDiffuseSpecularMapSRV(ID3D11ShaderResourceView* DiffuseSpecularMapSRV) { mDiffuseSpecularMapSRV = DiffuseSpecularMapSRV; }
};

