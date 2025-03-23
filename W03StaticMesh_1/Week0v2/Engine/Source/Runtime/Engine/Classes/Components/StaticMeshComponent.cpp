#include "Components/StaticMeshComponent.h"


uint32 UStaticMeshComponent::GetNumMaterials() const
{
    if (staticMesh == nullptr) return 0;
        
    return staticMesh->GetMaterials().Len();
}

UMaterial* UStaticMeshComponent::GetMaterial(uint32 ElementIndex) const
{
    if (staticMesh == nullptr) return nullptr;

    if (OverrideMaterials[ElementIndex] != nullptr)
        return OverrideMaterials[ElementIndex];

    if (staticMesh->GetMaterials().IsValidIndex(ElementIndex))
        return staticMesh->GetMaterials()[ElementIndex]->Material;
}

uint32 UStaticMeshComponent::GetMaterialIndex(FName MaterialSlotName) const
{
    if (staticMesh == nullptr) return -1;

    return staticMesh->GetMaterialIndex(MaterialSlotName);
}

TArray<FName> UStaticMeshComponent::GetMaterialSlotNames() const
{
    TArray<FName> MaterialNames;
    if (staticMesh == nullptr) return MaterialNames;

    for (uint32 MaterialIndex = 0; MaterialIndex < staticMesh->GetMaterials().Len(); MaterialIndex++) {
        MaterialNames.Add(staticMesh->GetMaterials()[MaterialIndex]->MaterialSlotName);
    }

    return MaterialNames;
}

void UStaticMeshComponent::GetUsedMaterials(TArray<UMaterial*> Out) const
{
    if (staticMesh == nullptr) return;
    staticMesh->GetUsedMaterials(Out);
    for (int materialIndex = 0; materialIndex < GetNumMaterials(); materialIndex++) {
        if (OverrideMaterials[materialIndex] != nullptr)
            Out[materialIndex] = OverrideMaterials[materialIndex];
    }
}
