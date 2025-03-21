#include "GizmoRectangleComponent.h"

#include "World.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "Components/Player.h"
UGizmoRectangleComponent::UGizmoRectangleComponent()
{
}

UGizmoRectangleComponent::~UGizmoRectangleComponent()
{
}

void UGizmoRectangleComponent::Initialize()
{
    Super::Initialize();
}

void UGizmoRectangleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UGizmoRectangleComponent::Release()
{
}

void UGizmoRectangleComponent::Render()
{
#pragma region GizmoDepth
    ID3D11DepthStencilState* DepthStateDisable = FEngineLoop::graphicDevice.DepthStateDisable;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
#pragma endregion GizmoDepth

    if (!GetWorld()->GetPickingObj() || GetWorld()->GetPlayer()->GetControlMode() != CM_SCALE)
        return;
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetQuat(), GetWorldScale());

    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().View * GetEngine().Projection;
    if (this == GetWorld()->GetPickingGizmo()) {
        FEngineLoop::renderer.UpdateConstant(MVP, 1.0f);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, 0.0f);

    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID); // fill solid로 렌더링.
    Super::Render();
    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = FEngineLoop::graphicDevice.DepthStencilState;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}
