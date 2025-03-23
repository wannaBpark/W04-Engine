#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Components/Material/Material.h"
#include "Define.h"

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
    UStaticMesh();
    ~UStaticMesh();
    void Initialize() override;
    TArray<FStaticMaterial*> GetMaterials() const { return materials; }
    uint32 GetMaterialIndex(FName MaterialSlotName) const;
    void GetUsedMaterials(TArray<UMaterial*> Out) const;
    OBJ::FStaticMeshRenderData* GetRenderData() const { return staticMeshRenderData; }
private:
    OBJ::FStaticMeshRenderData* staticMeshRenderData;
    TArray<FStaticMaterial*> materials;

};