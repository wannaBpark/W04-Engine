#include "CubeComp.h"
#include "Math/JungleMath.h"
#include "World.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/ObjectFactory.h"

#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

#include "Engine/FLoaderOBJ.h"

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

    FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
    SetStaticMesh(FManagerOBJ::GetStaticMesh(L"helloBlender.obj"));
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
    FMatrix MVP = Model * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix() * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    FVector4 UUIDColor = EncodeUUID() / 255.0f;
    if (this == GetWorld()->GetPickingObj()) {
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

    if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        UPrimitiveBatch::GetInstance().RenderAABB(AABB, GetWorldLocation(), Model);
    if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        Super::Render();
}
