#include "MeshComponent.h"


UMaterial* UMeshComponent::GetMaterial(uint32 ElementIndex) const
{
    if (OverrideMaterials.IsValidIndex(ElementIndex))
        return OverrideMaterials[ElementIndex];

    return nullptr;
}

uint32 UMeshComponent::GetMaterialIndex(FName MaterialSlotName) const
{
    // This function should be overridden
    return INDEX_NONE;
}

UMaterial* UMeshComponent::GetMaterialByName(FName MaterialSlotName) const
{
    int32 MaterialIndex = GetMaterialIndex(MaterialSlotName);
    if (MaterialIndex < 0)
        return nullptr;
    return GetMaterial(MaterialIndex);
}

TArray<FName> UMeshComponent::GetMaterialSlotNames() const
{
    return TArray<FName>();
}

void UMeshComponent::SetMaterial(uint32 ElementIndex, UMaterial* Material)
{
    if (OverrideMaterials.IsValidIndex(ElementIndex) == false) return;

    OverrideMaterials[ElementIndex] = Material;
}

void UMeshComponent::SetMaterialByName(FName MaterialSlotName, UMaterial* Material)
{
    int32 MaterialIndex = GetMaterialIndex(MaterialSlotName);
    if (MaterialIndex < 0)
        return;

    SetMaterial(MaterialIndex, Material);
}

void UMeshComponent::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    for (int32 ElementIndex = 0; ElementIndex < GetNumMaterials(); ElementIndex++)
    {
        if (UMaterial* Material = GetMaterial(ElementIndex))
        {
            Out.Add(Material);
        }
    }
}
