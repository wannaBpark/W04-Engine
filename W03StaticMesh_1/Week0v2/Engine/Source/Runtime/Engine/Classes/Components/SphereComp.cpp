#include "SphereComp.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "Engine/Source/Runtime/Engine/World.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
USphereComp::USphereComp() : UPrimitiveComponent("Sphere")
{
    AABB.max = { 1,1,1 };
    AABB.min = { -1,-1,-1 };
}

USphereComp::~USphereComp()
{
}

void USphereComp::Initialize()
{
    Super::Initialize();
}

void USphereComp::Update(double deltaTime)
{
}

void USphereComp::Release()
{
}

void USphereComp::Render()
{
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());

    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().View * GetEngine().Projection;
    //FMatrix MVP = Model * GetEngine().GetViewportClient()->GetViewMatrix() * GetEngine().GetViewportClient()->GetPerspectiveMatrix();
    FEngineLoop::renderer.UpdateNormalConstantBuffer(Model);
    if (this == GetWorld()->GetPickingObj()) {
        FEngineLoop::renderer.UpdateConstant(MVP, 1.0f);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, 0.0f);

    FVector scale = GetWorldScale();
    FVector r = { 1,1,1 };
    bool isUniform = (fabs(scale.x - scale.y) < 1e-6f) && (fabs(scale.y - scale.z) < 1e-6f);
    r = { r.x * scale.x,r.y * scale.y,r.z * scale.z };

    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB)) {
        UPrimitiveBatch::GetInstance().RenderAABB(AABB, GetWorldLocation(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(AABB, GetWorldLocation(), Model);
    }
    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        Super::Render();
}
