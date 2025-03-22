#include "GizmoCircleComponent.h"

#include "World.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "Components/Player.h"
#define DISC_RESOLUTION 128

static const FVector Colors[] = {
    FVector(1.0f, 0.0f, 0.0f),  // X��: ����
    FVector(0.0f, 1.0f, 0.0f),  // Y��: �ʷ�
    FVector(0.0f, 0.0f, 1.0f)   // Z��: �Ķ�
};

// {
//     inner = innerRadius;
//     FVector color3 = Colors[color];
//     TArray<FVertexSimple> vertices;
//     TArray<uint32>  indices;
//     if (vertices.IsEmpty())
//     {
//         float angleStep = 2.0f * 3.1415926535f / DISC_RESOLUTION;
//
//         // 원의 외곽 버텍스 추가
//         for (int i = 0; i <= DISC_RESOLUTION; ++i) {
//             float angle = i * angleStep;
//             float x = cos(angle);
//             float z = sin(angle);
//             vertices.Add({ x, 0.0f, z ,color3.x, color3.y, color3.z, 1 });
//
//             x *= innerRadius;
//             z *= innerRadius;
//             vertices.Add({ x, 0.0f, z ,color3.x, color3.y, color3.z, 1 });
//         }
//
//         // 인덱스 설정 (삼각형 리스트)
//         for (int i = 0; i <= DISC_RESOLUTION + 1; ++i) {
//             int pivot = 2 * i;
//             indices.Add(pivot);      // 중심점
//             indices.Add(pivot + 3);      // 현재 점
//             indices.Add(pivot + 2);  // 다음 점 (마지막 점 예외 처리)
//
//             indices.Add(pivot);      // 중심점
//             indices.Add(pivot + 1);      // 현재 점
//             indices.Add(pivot + 3);  // 다음 점 (마지막 점 예외 처리)
//         }
//         for (int i = 0; i <= DISC_RESOLUTION + 1; ++i) {
//             int pivot = 2 * i;
//             indices.Add(pivot);      // 중심점
//             indices.Add(pivot + 2);      // 현재 점
//             indices.Add(pivot + 3);  // 다음 점 (마지막 점 예외 처리)
//
//             indices.Add(pivot);      // 중심점
//             indices.Add(pivot + 3);      // 현재 점
//             indices.Add(pivot + 1);  // 다음 점 (마지막 점 예외 처리)
//         }
//
//     }
//
//     FEngineLoop::resourceMgr.RegisterMesh(&FEngineLoop::renderer,
//         type, vertices, vertices.Num(), indices, indices.Num());
// }

UGizmoCircleComponent::UGizmoCircleComponent()
{
    //inner = 0.5;
    //TArray<FVertexSimple> vertices;
    //TArray<uint32>  indices;

    //if (vertices.IsEmpty())
    //{
    //    float angleStep = 2.0f * 3.1415926535f / DISC_RESOLUTION;

    //    // 원의 외곽 버텍스 추가
    //    for (int i = 0; i <= DISC_RESOLUTION; ++i) {
    //        float angle = i * angleStep;
    //        float x = cos(angle);
    //        float z = sin(angle);
    //        vertices.Add({ x, 0.0f, z ,1,1,1, 1 });

    //        x *= 0.5;
    //        z *= 0.5;
    //        vertices.Add({ x, 0.0f, z ,1,1,1, 1 });
    //    }

    //    // 인덱스 설정 (삼각형 리스트)
    //    for (int i = 0; i <= DISC_RESOLUTION + 1; ++i) {
    //        int pivot = 2 * i;
    //        indices.Add(pivot);      // 중심점
    //        indices.Add(pivot + 3);      // 현재 점
    //        indices.Add(pivot + 2);  // 다음 점 (마지막 점 예외 처리)

    //        indices.Add(pivot);      // 중심점
    //        indices.Add(pivot + 1);      // 현재 점
    //        indices.Add(pivot + 3);  // 다음 점 (마지막 점 예외 처리)

    //    }
    //}

    //FEngineLoop::resourceMgr.RegisterMesh(&FEngineLoop::renderer,
    //    FString("Disc"), vertices, vertices.Num(), indices, indices.Num());

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
    FMatrix MVP = Model * GetEngine().View * GetEngine().Projection;
    if (this == GetWorld()->GetPickingGizmo()) {
        FEngineLoop::renderer.UpdateConstant(MVP, 1.0f);
    }
    else
        FEngineLoop::renderer.UpdateConstant(MVP, 0.0f);
    FEngineLoop::renderer.UpdateUUIDConstantBuffer(EncodeUUID());

    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID); // fill solid로 렌더링.
    Super::Render();
    FEngineLoop::graphicDevice.DeviceContext->RSSetState(FEngineLoop::graphicDevice.GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* OriginalDepthState = FEngineLoop::graphicDevice.DepthStencilState;
    FEngineLoop::graphicDevice.DeviceContext->OMSetDepthStencilState(OriginalDepthState, 0);
#pragma endregion GizmoDepth
}

