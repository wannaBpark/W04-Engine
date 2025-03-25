#include "GizmoRectangleComponent.h"

#include "World.h"
#include "Actors/Player.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"


UGizmoRectangleComponent::UGizmoRectangleComponent()
{
}

UGizmoRectangleComponent::~UGizmoRectangleComponent()
{
}

void UGizmoRectangleComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UGizmoRectangleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UGizmoRectangleComponent::Render()
{
#pragma region GizmoDepth
    ID3D11DepthStencilState* DepthStateDisable = FEngineLoop::graphicDevice.DepthStateDisable;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
#pragma endregion GizmoDepth

    if (!GetWorld()->GetPickedActor() || GetWorld()->GetPlayer()->GetControlMode() != CM_SCALE)
        return;
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetQuat(), GetWorldScale());

    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix() * GetEngine().GetLevelEditor()->
        GetActiveViewportClient()->GetProjectionMatrix();
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    FVector4 UUIDColor = EncodeUUID() / 255.0f;
    if (this == GetWorld()->GetPickingGizmo())
    {
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
    }
    else
    {
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, false);
    }

    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID); // fill solid로 렌더링.
    Super::Render();
    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = FEngineLoop::graphicDevice.DepthStencilState;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}
