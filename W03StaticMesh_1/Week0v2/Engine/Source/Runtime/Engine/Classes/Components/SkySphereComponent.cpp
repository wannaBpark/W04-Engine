#include "SkySphereComponent.h"

#include "World.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

#include "Mesh/StaticMesh.h"
USkySphereComponent::USkySphereComponent()
{
    SetType(StaticClass()->GetName());
}

USkySphereComponent::~USkySphereComponent()
{
}

void USkySphereComponent::Initialize()
{
    Super::Initialize();
}

void USkySphereComponent::Update(double deltaTime)
{
}

void USkySphereComponent::Release()
{
}

void USkySphereComponent::Render()
{
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());

    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix() * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    FVector4 UUIDColor = EncodeUUID() / 255.0f;
    if (this == GetWorld()->GetPickingObj())
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
    else
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

    FVector scale = GetWorldScale();
    FVector r = { 1,1,1 };
    bool isUniform = (fabs(scale.x - scale.y) < 1e-6f) && (fabs(scale.y - scale.z) < 1e-6f);
    r = { r.x * scale.x,r.y * scale.y,r.z * scale.z };

    if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB)) {
        UPrimitiveBatch::GetInstance().RenderAABB(AABB, GetWorldLocation(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(AABB, GetWorldLocation(), Model);
    }
    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives)) {
        //std::shared_ptr<FStaticMeshRenderData> renderData = staticMesh->GetRenderData();
        //std::shared_ptr<FStaticMeshRenderData> renderData = FEngineLoop::resourceMgr.GetMesh(GetType());
        //FEngineLoop::renderer.RenderTexturedModelPrimitive(renderData->vertexBuffer,
        //    renderData->numVertices, renderData->indexBuffer, renderData->numIndices,
        //    Texture->TextureSRV, Texture->SamplerState
        //);
    }
}
