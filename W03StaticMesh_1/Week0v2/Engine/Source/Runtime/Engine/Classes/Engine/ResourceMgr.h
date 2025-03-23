#pragma once
#include <unordered_map>
#include <memory>
#include "StaticMesh.h"
#include "Texture.h"
#include "Define.h"
#include "Container/Map.h"

class FRenderer;
class FGraphicsDevice;
class FResourceMgr
{

public:
    void Initialize(FRenderer* renderer, FGraphicsDevice* device);
    void Release(FRenderer* renderer);
    HRESULT LoadTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);
    HRESULT LoadTextureFromDDS(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);

    std::shared_ptr<FTexture> GetTexture(const FWString& name) const;
private:
    TMap<FWString, std::shared_ptr<FTexture>> textureMap;
};