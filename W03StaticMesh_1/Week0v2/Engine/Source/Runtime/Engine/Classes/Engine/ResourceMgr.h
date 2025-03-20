#pragma once
#include <unordered_map>
#include <memory>
#include "StaticMesh.h"
#include "Texture.h"
#include "Define.h"

class FRenderer;
class FGraphicsDevice;
class FResourceMgr
{

public:
    void Initialize(FRenderer* renderer, FGraphicsDevice* device);
    void Release(FRenderer* renderer);
    void LoadObjAsset(FRenderer* renderer, const FString& meshName, const FWString& filepath);
    void LoadObjNormalAsset(FRenderer* renderer, const FString& meshName, const FWString& filepath);
    void LoadObjNormalTextureAsset(FRenderer* renderer, const FString& meshName, const FWString& filepath);
    void RegisterMesh(FRenderer* renderer, const std::string& name, FVertexSimple* vertices, uint32 vCount, uint32* indices, uint32 iCount);
    void RegisterMesh(FRenderer* renderer, const std::string& name, TArray<FVertexSimple>& vertices, uint32 vCount, TArray<uint32>& indices, uint32 iCount);
    void RegisterMesh(FRenderer* renderer, const std::string& name, TArray<FVertexTexture>& vertices, uint32 vCount, TArray<uint32>& indices, uint32 iCount);
    HRESULT LoadTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);
    HRESULT LoadTextureFromDDS(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);

    std::shared_ptr<FStaticMesh> GetMesh(const FString& name) const;
    std::shared_ptr<FTexture> GetTexture(const FWString& name) const;
private:
    TMap<FString, std::shared_ptr<FStaticMesh>> meshMap;
    TMap<FWString, std::shared_ptr<FTexture>> textureMap;
};

