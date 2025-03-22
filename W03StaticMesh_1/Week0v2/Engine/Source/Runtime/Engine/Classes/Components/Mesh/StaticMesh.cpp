#include "StaticMesh.h"

uint32 UStaticMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Len(); materialIndex++) {
        if (materials[materialIndex]->MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void UStaticMesh::GetUsedMaterials(TArray<UMaterial*> Out) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Len(); materialIndex++) {
        Out.Add(materials[materialIndex]->Material);
    }
}
