#pragma once
#include "GraphicsHelpers.h"
#include <string.h>
class Texture
{
private:
	ID3D11Resource* mDiffuseSpecularMap = nullptr;
	ID3D11ShaderResourceView* mDiffuseSpecularMapSRV = nullptr;
	std::string mTextureName = "";
	ID3D11Resource* mNormalMap = nullptr;
	ID3D11ShaderResourceView* mNormalMapSRV = nullptr;
	std::string mNormalName = "";
public:
	Texture(std::string TextureName) : mTextureName(TextureName)	
	{		
	}
	Texture(std::string TextureName, std::string NormalName) : mTextureName(TextureName), mNormalName(NormalName)
	{
	}
	std::string GetTextureName() { return mTextureName; }
	void SetTextureName(std::string TextureName) { mTextureName = TextureName; }
	std::string GetNormalName() { return mNormalName; }
	void SetNormalName(std::string NormalName) { mNormalName = NormalName; }
	ID3D11Resource* GetDiffuseSpecularMap() { return mDiffuseSpecularMap; }
	void SetDiffuseSpecularMap(ID3D11Resource* DiffuseSpecularMap) { mDiffuseSpecularMap = DiffuseSpecularMap; }
	ID3D11ShaderResourceView* GetDiffuseSpecularMapSRV() { return mDiffuseSpecularMapSRV; }
	void SetDiffuseSpecularMapSRV(ID3D11ShaderResourceView* DiffuseSpecularMapSRV) { mDiffuseSpecularMapSRV = DiffuseSpecularMapSRV; }
	ID3D11Resource* GetNormalMap() { return mNormalMap; }
	void SetNormalMap(ID3D11Resource* NormalMap) { mNormalMap = NormalMap; }
	ID3D11ShaderResourceView* GetNormalMapSRV() { return mNormalMapSRV; }
	void SetNormalMapSRV(ID3D11ShaderResourceView* NormalMapSRV) { mNormalMapSRV = NormalMapSRV; }
};

