#include "SphereComp.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "Engine/Source/Runtime/Engine/World.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"


USphereComp::USphereComp()
{
    SetType(StaticClass()->GetName());
    AABB.max = {1, 1, 1};
    AABB.min = {-1, -1, -1};
}

USphereComp::~USphereComp()
{
}

void USphereComp::InitializeComponent()
{
    Super::InitializeComponent();
}

void USphereComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void USphereComp::Render()
{
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    FVector4 UUIDColor = EncodeUUID() / 255.0f;
    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix() * GetEngine().GetLevelEditor()->
        GetActiveViewportClient()->GetProjectionMatrix();
    if (GetWorld()->GetPickedActor() == GetOwner())
    {
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
    }
    else
    {
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, false);
    }

    FVector scale = GetWorldScale();
    FVector r = {1, 1, 1};
    bool isUniform = (fabs(scale.x - scale.y) < 1e-6f) && (fabs(scale.y - scale.z) < 1e-6f);
    r = {r.x * scale.x, r.y * scale.y, r.z * scale.z};

    if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
    {
        UPrimitiveBatch::GetInstance().RenderAABB(AABB, GetWorldLocation(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(AABB, GetWorldLocation(), Model);
    }
    if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
    {
        Super::Render();
    }
}
