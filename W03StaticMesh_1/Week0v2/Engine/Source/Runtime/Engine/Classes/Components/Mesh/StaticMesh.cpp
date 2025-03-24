#include "StaticMesh.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/ObjectFactory.h"

UStaticMesh::UStaticMesh()
{
    staticMeshRenderData = FManagerOBJ::LoadObjStaticMeshAsset("Assets/helloBlender.obj");

    if (staticMeshRenderData == nullptr) return;
    
    uint32 verticeNum = staticMeshRenderData->Vertices.Num();
    if(verticeNum <= 0) return;
    staticMeshRenderData->VertexBuffer = GetEngine().renderer.CreateVertexBuffer(staticMeshRenderData->Vertices, verticeNum * sizeof(FVertexSimple));

    uint32 indexNum = staticMeshRenderData->Indices.Num();
    if (indexNum <= 0) return;
    staticMeshRenderData->IndexBuffer = GetEngine().renderer.CreateIndexBuffer(staticMeshRenderData->Indices, indexNum * sizeof(FVertexSimple));

    for (int materialIndex = 0; materialIndex < staticMeshRenderData->Materials.Num(); materialIndex++) {
        FStaticMaterial* newMaterialSlot = new FStaticMaterial();
        UMaterial* newMaterial = FObjectFactory::ConstructObject<UMaterial>();
        newMaterialSlot->Material = newMaterial;
        newMaterialSlot->Material->SetmaterialInfo(staticMeshRenderData->Materials[materialIndex]);
        newMaterialSlot->MaterialSlotName = staticMeshRenderData->Materials[materialIndex].MTLName;
        materials.Add(newMaterialSlot);
    }
}

UStaticMesh::~UStaticMesh()
{
    if (staticMeshRenderData == nullptr) return;

    if (staticMeshRenderData->VertexBuffer) {
        staticMeshRenderData->VertexBuffer->Release();
        staticMeshRenderData->VertexBuffer = nullptr;
    }

    if (staticMeshRenderData->IndexBuffer) {
        staticMeshRenderData->IndexBuffer->Release();
        staticMeshRenderData->IndexBuffer = nullptr;
    }
}

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
