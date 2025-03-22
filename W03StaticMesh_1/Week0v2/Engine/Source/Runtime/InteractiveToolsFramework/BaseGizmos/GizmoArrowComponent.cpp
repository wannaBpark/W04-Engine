#include "GizmoArrowComponent.h"

#include "World.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "Components/Player.h"
UGizmoArrowComponent::UGizmoArrowComponent()
{
}

UGizmoArrowComponent::~UGizmoArrowComponent()
{
}

void UGizmoArrowComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UGizmoArrowComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}

void UGizmoArrowComponent::Render()
{
#pragma region GizmoDepth
    ID3D11DepthStencilState* DepthStateDisable = FEngineLoop::graphicDevice.DepthStateDisable;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
#pragma endregion GizmoDepth

    if (!GetWorld()->GetPickingObj() || GetWorld()->GetPlayer()->GetControlMode() != CM_TRANSLATION)
        return;
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());

    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().View * GetEngine().Projection;
	
	
    if (this == GetWorld()->GetPickingGizmo()) {
        FEngineLoop::renderer.UpdateConstant(MVP, 1.0f);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, 0.0f);

    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID); // fill solid로 렌더링.
    Super::Render();
    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.GetCurrentRasterizer()); // 이전 레스터라이저 재설정.

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = FEngineLoop::graphicDevice.DepthStencilState;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}
