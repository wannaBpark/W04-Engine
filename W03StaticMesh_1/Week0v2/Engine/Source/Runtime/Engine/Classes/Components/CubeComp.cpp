#include "CubeComp.h"
#include "Math/JungleMath.h"
#include "World.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/PrimitiveBatch.h"
UCubeComp::UCubeComp() : UPrimitiveComponent("Cube")
{
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
    FEngineLoop::renderer.UpdateNormalConstantBuffer(Model);
    if (this == GetWorld()->GetPickingObj()) {
        FEngineLoop::renderer.UpdateConstant(MVP, 1.0f);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, 0.0f);

    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        UPrimitiveBatch::GetInstance().RenderAABB(AABB, GetWorldLocation(), Model);
    if (ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        Super::Render();
}
