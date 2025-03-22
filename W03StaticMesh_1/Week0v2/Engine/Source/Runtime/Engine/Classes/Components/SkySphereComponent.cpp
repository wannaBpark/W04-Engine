#include "SkySphereComponent.h"

#include "World.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
USkySphereComponent::USkySphereComponent()
{
    SetType(StaticClass()->GetName());
}

USkySphereComponent::~USkySphereComponent()
{
}

void USkySphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USkySphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}

void USkySphereComponent::Render()
{
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());

    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().View * GetEngine().Projection;
    FEngineLoop::renderer.UpdateNormalConstantBuffer(Model);
    if (this == GetWorld()->GetPickingObj()) {
        FEngineLoop::renderer.UpdateConstant(MVP, 1.0f);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, 0.0f);
    FEngineLoop::renderer.UpdateUUIDConstantBuffer(EncodeUUID());

    FVector scale = GetWorldScale();
    FVector r = { 1,1,1 };
    bool isUniform = (fabs(scale.x - scale.y) < 1e-6f) && (fabs(scale.y - scale.z) < 1e-6f);
    r = { r.x * scale.x,r.y * scale.y,r.z * scale.z };

    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB)) {
        UPrimitiveBatch::GetInstance().RenderAABB(AABB, GetWorldLocation(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(AABB, GetWorldLocation(), Model);
    }
    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        FEngineLoop::renderer.RenderTexturedModelPrimitive(staticMesh->vertexBuffer,
            staticMesh->numVertices, staticMesh->indexBuffer, staticMesh->numIndices,
            Texture->TextureSRV, Texture->SamplerState
        );
}
