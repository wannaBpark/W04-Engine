#include "StaticMesh.h"
#include "Engine/FLoaderOBJ.h"

UStaticMesh::UStaticMesh()
{
    staticMeshRenderData = FManagerOBJ::LoadObjStaticMeshAsset("Assets/helloBlender.obj");

    if (staticMeshRenderData == nullptr) return;
    
    uint32 verticeNum = staticMeshRenderData->Vertices.Len();
    if(verticeNum <= 0) return;
    staticMeshRenderData->VertexBuffer = GetEngine().renderer.CreateVertexBuffer(staticMeshRenderData->Vertices, verticeNum * sizeof(FVertexSimple));

    
    uint32 indexNum = staticMeshRenderData->Indices.Len();
    if (indexNum <= 0) return;
    staticMeshRenderData->IndexBuffer = GetEngine().renderer.CreateIndexBuffer(staticMeshRenderData->Indices, indexNum * sizeof(FVertexSimple));
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

void UStaticMesh::Initialize()
{

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
