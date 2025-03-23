#include "CubeComp.h"
#include "Math/JungleMath.h"
#include "World.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/PrimitiveBatch.h"
UCubeComp::UCubeComp()
{
    SetType(StaticClass()->GetName());
    AABB.max = { 1,1,1 };
    AABB.min = { -1,-1,-1 };
}

UCubeComp::~UCubeComp()
{
}

void UCubeComp::Initialize()
{
    Super::Initialize();
}

void UCubeComp::Update(double deltaTime)
{
}

void UCubeComp::Release()
{
}

void UCubeComp::Render()
{
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());
    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().View * GetEngine().Projection;
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    FVector4 UUIDColor = EncodeUUID() / 255.0f;
    if (this == GetWorld()->GetPickingObj()) {
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        UPrimitiveBatch::GetInstance().RenderAABB(AABB, GetWorldLocation(), Model);
    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        Super::Render();
}
