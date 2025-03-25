#include "GizmoCircleComponent.h"

#include "World.h"
#include "Math/JungleMath.h"
#include "Components/Player.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

#define DISC_RESOLUTION 128

UGizmoCircleComponent::UGizmoCircleComponent()
{
}

UGizmoCircleComponent::~UGizmoCircleComponent()
{
}

bool UGizmoCircleComponent::IntersectsRay(const FVector& rayOrigin, const FVector& rayDir, float& dist)
{
    if (rayDir.y == 0) return false; // normal to normal vector of plane

    dist = -rayOrigin.y / rayDir.y;

    FVector intersectionPoint = rayOrigin + rayDir * dist;
    float intersectionToDiscCenterSquared = intersectionPoint.Magnitude();

    return (inner * inner < intersectionToDiscCenterSquared && intersectionToDiscCenterSquared < 1);
}

void UGizmoCircleComponent::Render()
{
#pragma region GizmoDepth
    ID3D11DepthStencilState* DepthStateDisable = FEngineLoop::graphicDevice.DepthStateDisable;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
#pragma endregion GizmoDepth

    if (!GetWorld()->GetPickingObj() || GetWorld()->GetPlayer()->GetControlMode() != CM_ROTATION)
        return;
    FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());

    // 최종 MVP 행렬
    FMatrix MVP = Model * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix() * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    FVector4 UUIDColor = EncodeUUID() / 255.0f;
    if (this == GetWorld()->GetPickingGizmo()) {
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID); // fill solid로 렌더링.
    Super::Render();
    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* OriginalDepthState = FEngineLoop::graphicDevice.DepthStencilState;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(OriginalDepthState, 0);
#pragma endregion GizmoDepth
}

