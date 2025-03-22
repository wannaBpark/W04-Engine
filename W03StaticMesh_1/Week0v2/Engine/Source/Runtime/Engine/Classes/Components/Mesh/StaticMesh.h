#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Components/Material/Material.h"
#include "Classes/Engine/StaticMesh.h"

// FStaticMaterial 구조체
struct FStaticMaterial
{
    UMaterial* Material;
    FName MaterialSlotName;
    //FMeshUVChannelInfo UVChannelData;
};

class UStaticMesh : public UObject
{
    DECLARE_CLASS(UStaticMesh, UObject)

public:
    TArray<FStaticMaterial*> GetMaterials() const { return materials; }
    uint32 GetMaterialIndex(FName MaterialSlotName) const;
    void GetUsedMaterials(TArray<UMaterial*> Out) const;
    std::shared_ptr<FStaticMesh> GetRenderData() const { return staticMeshRenderData; }
private:
    std::shared_ptr<FStaticMesh> staticMeshRenderData = FEngineLoop::resourceMgr.GetMesh("UCubeComp"); // 언리얼의 FStaticMeshRenderData
    TArray<FStaticMaterial*> materials;

};