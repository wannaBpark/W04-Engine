#include "SoftwareZBuffer.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"   
#include "Editor/UnrealEd/PrimitiveBatch.h"
#include "LevelEditor/SLevelEditor.h"
#include "Runtime/Core/Math/JungleMath.h"
#include "GeometryCore/KDTree.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cfloat>

void SoftwareZBuffer::RasterizeOccluder(const FBoundingBox& bbox, const FMatrix& MVP)
{
    // bbox의 8개 모서리를 변환하여 화면 공간 좌표(ndc, [0,1] 범위)로 계산
    std::vector<FVector4> screenPoints;
    screenPoints.reserve(8);

    for (int i = 0; i < 8; ++i)
    {
        FVector corner;
        corner.X = (i & 1) ? bbox.max.X : bbox.min.X;
        corner.Y = (i & 2) ? bbox.max.Y : bbox.min.Y;
        corner.Z = (i & 4) ? bbox.max.Z : bbox.min.Z;

        // 3D 벡터를 4D 벡터로 변환 (w=1)
        FVector4 corner4(corner.X, corner.Y, corner.Z, 1.0f);
        // mvp 행렬로 변환 (우리 커스텀 FMatrix의 TransformFVector4 사용)
        FVector4 clip = MVP.TransformFVector4(corner4);

        // Perspective divide (w가 0이면 건너뜀 - div by 0 방지)
        if (std::fabs(clip.W) > 1e-6f)
        {
            clip.X /= clip.W;
            clip.Y /= clip.W;
            clip.Z /= clip.W;
        }
        else {
            continue;
        }

        // NDC [-1,1]를 화면 좌표 [0,width] / [0,height]로 매핑 (y축 flip)
        clip.X = (clip.X * 0.5f + 0.5f) * width;
        clip.Y = (-clip.Y * 0.5f + 0.5f) * height;
        // clip.Z는 0~1 depth 범위로 남음

        screenPoints.push_back(clip);
    }

    if (screenPoints.empty())
        return;

    // screenPoints의 최소/최대 x,y와 최소 z 값을 계산
    float minX = FLT_MAX, minY = FLT_MAX, minZ = 1.0f;
    float maxX = -FLT_MAX, maxY = -FLT_MAX;
    for (const auto& sp : screenPoints)
    {
        minX = std::min(minX, sp.X); minY = std::min(minY, sp.Y); 
        minZ = std::min(minZ, sp.Z);
        maxX = std::max(maxX, sp.X);
        maxY = std::max(maxY, sp.Y);
    }

    // 화면 영역으로 clamping (정수 좌표로 변환)
    int x0 = std::max(0, (int)std::floor(minX));
    int y0 = std::max(0, (int)std::floor(minY));
    int x1 = std::min(width - 1, (int)std::ceil(maxX));
    int y1 = std::min(height - 1, (int)std::ceil(maxY));

    // 해당 영역의 각 픽셀에 현재 zBuffer 값과 minZ 중 작은 값으로 업데이트
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            int idx = y * width + x;
            zBuffer[idx] = std::min(zBuffer[idx], minZ);
        }
    }
}

// Software Occlusion Culling 함수 (KD-Tree 기반)
// - kdTree: 씬의 KD-Tree 시스템
// - camera: 카메라 정보 (viewMatrix, projMatrix, position)
// - frustum: 카메라의 view frustum
// - swZBuffer: 소프트웨어 Z-Buffer (낮은 해상도)
// - OutVisibleComponents: 최종 렌더링할 또는 선택할 오브젝트 목록
void SoftwareZBuffer::PerformSWOcclusionCulling(KDTreeSystem*& kdTree, const FFrustum& frustum, SoftwareZBuffer& swZBuffer, TArray<UPrimitiveComponent*>& OutVisibleComponents)
{
    // 1. KD-Tree를 이용해 프러스텀과 교차하는 후보 오브젝트들을 수집
    TArray<UPrimitiveComponent*> candidateComponents;
    if (kdTree->Root)
        kdTree->Root->QueryFrustum(frustum, candidateComponents);
    UE_LOG(LogLevel::Display, "Candiatae Comps : %d", candidateComponents.Num());
    // 2. 후보들을 occluder와 occludee로 분리 (여기서는 간단히, bIsOccluder 플래그 기준)
    TArray<UPrimitiveComponent*> occluders;
    TArray<UPrimitiveComponent*> occludees;
    
    // 임계값: projected area threshold (화면상 면적이 이 값 이상이면 occluder)
    const float projectedAreaThreshold = 0.0f; // (이 값은 조정 가능)

    FMatrix viewMatrix = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();
    FMatrix projMatrix = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    FMatrix ViewProj = viewMatrix * projMatrix;

    for (UPrimitiveComponent* comp : candidateComponents)
    {
        FMatrix model = JungleMath::CreateModelMatrix(
            comp->GetWorldLocation(),
            comp->GetWorldRotation(),
            comp->GetWorldScale()
        );
        FMatrix mvp = model * ViewProj;
        FBoundingBox worldBBox = UPrimitiveBatch::GetWorldBoundingBox(comp->AABB, comp->GetWorldLocation(), model);
        FVector center = (worldBBox.min + worldBBox.max) * 0.5f;

        // 한 모서리(예: bbox.max) 사용하여 approximate radius 계산
        FVector corner = worldBBox.max;
        FVector4 center4(center.X, center.Y, center.Z, 1.0f);
        FVector4 corner4(corner.X, corner.Y, corner.Z, 1.0f);

        FVector4 clipCenter = mvp.TransformFVector4(center4);
        FVector4 clipCorner = mvp.TransformFVector4(corner4);
        if (std::fabs(clipCenter.W) > 1e-6f)
        {
            clipCenter.X /= clipCenter.W;
            clipCenter.Y /= clipCenter.W;
        }
        if (std::fabs(clipCorner.W) > 1e-6f)
        {
            clipCorner.X /= clipCorner.W;
            clipCorner.Y /= clipCorner.W;
        }
        float dx = clipCorner.X - clipCenter.X;
        float dy = clipCorner.Y - clipCenter.Y;
        float projectedRadSquared = dx * dx + dy * dy;
        float projectedArea = 3.1415926f * projectedRadSquared;

        // 분류: 만약 후보가 기본적으로 occluder 플래그가 설정되어 있거나, projectedArea가 임계값 이상이면 occluder로, 그렇지 않으면 occludee로 분류
        if (/*comp->bIsOccluder ||*/ projectedArea > projectedAreaThreshold)
            occluders.Add(comp);
        else
            occludees.Add(comp);
    }

    //UE_LOG(LogLevel::Display, "Occluders Culling : %d", occluders.Num());

    //UE_LOG(LogLevel::Display, "Occludees Culling : %d", occludees.Num());
    // 3. 소프트웨어 Z-Buffer 초기화
    swZBuffer.Clear();

    // 4. occluder들을 SW Z-Buffer에 rasterize
    //    각 occluder에 대해, 모델 행렬과 카메라의 view/proj 행렬을 곱하여 mvp 행렬을 계산하고,
    //    해당 오브젝트의 월드 AABB를 SW Z-Buffer에 rasterize 합니다.
    for (UPrimitiveComponent*& occ : occluders)
    {
        FMatrix model = JungleMath::CreateModelMatrix(
            occ->GetWorldLocation(),
            occ->GetWorldRotation(),
            occ->GetWorldScale()
        );
        FMatrix mvp = model * ViewProj;
        FBoundingBox occBBox = UPrimitiveBatch::GetWorldBoundingBox(
            occ->AABB, occ->GetWorldLocation(), model
        );
        swZBuffer.RasterizeOccluder(occBBox, mvp);
    }

    // 5. occludee에 대해 occlusion test
    //    occludee의 월드 AABB의 중심을 계산하여 mvp를 통해 클립 공간으로 변환한 후, perspective divide를 수행합니다.
    //    이후 ndc 좌표([-1,1])를 화면 좌표([0,width], [0,height])로 매핑하고, 해당 픽셀의 z-buffer 값과 비교합니다.
    for (UPrimitiveComponent*& occd : occludees)
    {
        FMatrix model = JungleMath::CreateModelMatrix(
            occd->GetWorldLocation(),
            occd->GetWorldRotation(),
            occd->GetWorldScale()
        );
        FMatrix mvp = model * ViewProj;
        FBoundingBox occdBBox = UPrimitiveBatch::GetWorldBoundingBox(
            occd->AABB, occd->GetWorldLocation(), model
        );
        
        FVector center = (occdBBox.min + occdBBox.max) * 0.5f; // 단순화를 위해 AABB의 중심 사용
        FVector4 center4(center.X, center.Y, center.Z, 1.0f);  // 4D 벡터로 확장 (w = 1) 
        FVector4 clip = mvp.TransformFVector4(center4);         // mvp 행렬을 통해 변환
        // Perspective divide
        float ndcX = clip.X, ndcY = clip.Y, ndcZ = clip.Z;
        if (std::fabs(clip.W) > 1e-6f) {
            ndcX /= clip.W;
            ndcY /= clip.W;
            ndcZ /= clip.W;
        }
        // ndc 좌표 [-1,1]를 화면 좌표 [0,width] / [0,height]로 매핑 (y축 flip)
        int screenX = (int)((ndcX * 0.5f + 0.5f) * swZBuffer.width);
        int screenY = (int)((-ndcY * 0.5f + 0.5f) * swZBuffer.height);
        // clamping
        screenX = std::max(0, std::min(screenX, swZBuffer.width - 1));
        screenY = std::max(0, std::min(screenY, swZBuffer.height - 1));
        float bufferDepth = swZBuffer.SampleDepth(screenX, screenY);
        // ndcZ가 bufferDepth보다 작으면(즉, 더 앞에 있으면) 해당 occludee는 보임
        if (ndcZ < bufferDepth)
        {
            OutVisibleComponents.Add(occd);
        }
    }
}