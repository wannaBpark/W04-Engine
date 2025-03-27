#include "PrimitiveBatch.h"
#include "EngineLoop.h"
#include "UnrealEd/EditorViewportClient.h"
extern FEngineLoop GEngineLoop;

UPrimitiveBatch::UPrimitiveBatch()
{
    GenerateGrid(5, 5000);
}

UPrimitiveBatch::~UPrimitiveBatch()
{
    if (pVertexBuffer) {
        pVertexBuffer->Release();
        pVertexBuffer = nullptr;
    }
    ReleaseOBBResources();
    ReleaseBoundingBoxResources();
    ReleaseConeResources();
}

void UPrimitiveBatch::Release()
{
    ReleaseOBBResources();
    ReleaseBoundingBoxResources();
    ReleaseConeResources();
}

void UPrimitiveBatch::GenerateGrid(float spacing, int gridCount)
{
    GridParam.gridSpacing = spacing;
    GridParam.numGridLines = gridCount;
    GridParam.gridOrigin = { 0,0,0 };
}

void UPrimitiveBatch::RenderBatch(const FMatrix& View, const FMatrix& Projection)
{
    FEngineLoop::renderer.PrepareLineShader();

    InitializeVertexBuffer();

    FMatrix Model = FMatrix::Identity;
    FMatrix MVP = Model * View * Projection;
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, FVector4(0,0,0,0), false);
    FEngineLoop::renderer.UpdateGridConstantBuffer(GridParam);

    UpdateBoundingBoxResources();
    UpdateConeResources();
    UpdateOBBResources();
    int boundingBoxSize = static_cast<int>(BoundingBoxes.Num());
    int coneSize = static_cast<int>(Cones.Num());
    int obbSize = static_cast<int>(OrientedBoundingBoxes.Num());
    FEngineLoop::renderer.UpdateLinePrimitveCountBuffer(boundingBoxSize, coneSize);
    FEngineLoop::renderer.RenderBatch(GridParam, pVertexBuffer, boundingBoxSize, coneSize, ConeSegmentCount, obbSize);
    BoundingBoxes.Empty();
    Cones.Empty();
    OrientedBoundingBoxes.Empty();
    FEngineLoop::renderer.PrepareShader();
}
void UPrimitiveBatch::InitializeVertexBuffer()
{
    if (!pVertexBuffer)
        pVertexBuffer = FEngineLoop::renderer.CreateStaticVerticesBuffer();
}

void UPrimitiveBatch::UpdateBoundingBoxResources()
{
    if (BoundingBoxes.Num() > allocatedBoundingBoxCapacity) {
        allocatedBoundingBoxCapacity = BoundingBoxes.Num();

        ReleaseBoundingBoxResources();

        pBoundingBoxBuffer = FEngineLoop::renderer.CreateBoundingBoxBuffer(static_cast<UINT>(allocatedBoundingBoxCapacity));
        pBoundingBoxSRV = FEngineLoop::renderer.CreateBoundingBoxSRV(pBoundingBoxBuffer, static_cast<UINT>(allocatedBoundingBoxCapacity));
    }

    if (pBoundingBoxBuffer && pBoundingBoxSRV){
        int boundingBoxCount = static_cast<int>(BoundingBoxes.Num());
        FEngineLoop::renderer.UpdateBoundingBoxBuffer(pBoundingBoxBuffer, BoundingBoxes, boundingBoxCount);
    }
}

void UPrimitiveBatch::ReleaseBoundingBoxResources()
{
    if (pBoundingBoxBuffer) pBoundingBoxBuffer->Release();
    if (pBoundingBoxSRV) pBoundingBoxSRV->Release();
}

void UPrimitiveBatch::UpdateConeResources()
{
    if (Cones.Num() > allocatedConeCapacity) {
        allocatedConeCapacity = Cones.Num();

        ReleaseConeResources();

        pConesBuffer = FEngineLoop::renderer.CreateConeBuffer(static_cast<UINT>(allocatedConeCapacity));
        pConesSRV = FEngineLoop::renderer.CreateConeSRV(pConesBuffer, static_cast<UINT>(allocatedConeCapacity));
    }

    if (pConesBuffer && pConesSRV) {
        int coneCount = static_cast<int>(Cones.Num());
        FEngineLoop::renderer.UpdateConesBuffer(pConesBuffer, Cones, coneCount);
    }
}

void UPrimitiveBatch::ReleaseConeResources()
{
    if (pConesBuffer) pConesBuffer->Release();
    if (pConesSRV) pConesSRV->Release();
}

void UPrimitiveBatch::UpdateOBBResources()
{
    if (OrientedBoundingBoxes.Num() > allocatedOBBCapacity) {
        allocatedOBBCapacity = OrientedBoundingBoxes.Num();

        ReleaseOBBResources();

        pOBBBuffer = FEngineLoop::renderer.CreateOBBBuffer(static_cast<UINT>(allocatedOBBCapacity));
        pOBBSRV = FEngineLoop::renderer.CreateOBBSRV(pOBBBuffer, static_cast<UINT>(allocatedOBBCapacity));
    }

    if (pOBBBuffer && pOBBSRV) {
        int obbCount = static_cast<int>(OrientedBoundingBoxes.Num());
        FEngineLoop::renderer.UpdateOBBBuffer(pOBBBuffer, OrientedBoundingBoxes, obbCount);
    }
}
void UPrimitiveBatch::ReleaseOBBResources()
{
    if (pOBBBuffer) pOBBBuffer->Release();
    if (pOBBSRV) pOBBSRV->Release();
}
void UPrimitiveBatch::RenderAABB(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix)
{
    FVector localVertices[8] = {
         { localAABB.min.x, localAABB.min.y, localAABB.min.z },
         { localAABB.max.x, localAABB.min.y, localAABB.min.z },
         { localAABB.min.x, localAABB.max.y, localAABB.min.z },
         { localAABB.max.x, localAABB.max.y, localAABB.min.z },
         { localAABB.min.x, localAABB.min.y, localAABB.max.z },
         { localAABB.max.x, localAABB.min.y, localAABB.max.z },
         { localAABB.min.x, localAABB.max.y, localAABB.max.z },
         { localAABB.max.x, localAABB.max.y, localAABB.max.z }
    };

    FVector worldVertices[8];
    worldVertices[0] = center + FMatrix::TransformVector(localVertices[0], modelMatrix);

    FVector min = worldVertices[0], max = worldVertices[0];

    // 첫 번째 값을 제외한 나머지 버텍스를 변환하고 min/max 계산
    for (int i = 1; i < 8; ++i)
    {
        worldVertices[i] = center + FMatrix::TransformVector(localVertices[i], modelMatrix);

        min.x = (worldVertices[i].x < min.x) ? worldVertices[i].x : min.x;
        min.y = (worldVertices[i].y < min.y) ? worldVertices[i].y : min.y;
        min.z = (worldVertices[i].z < min.z) ? worldVertices[i].z : min.z;

        max.x = (worldVertices[i].x > max.x) ? worldVertices[i].x : max.x;
        max.y = (worldVertices[i].y > max.y) ? worldVertices[i].y : max.y;
        max.z = (worldVertices[i].z > max.z) ? worldVertices[i].z : max.z;
    }
    FBoundingBox BoundingBox;
    BoundingBox.min = min;
    BoundingBox.max = max;
    BoundingBoxes.Add(BoundingBox);
}
void UPrimitiveBatch::RenderOBB(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix)
{
    // 1) 로컬 AABB의 8개 꼭짓점
    FVector localVertices[8] =
    {
        { localAABB.min.x, localAABB.min.y, localAABB.min.z },
        { localAABB.max.x, localAABB.min.y, localAABB.min.z },
        { localAABB.min.x, localAABB.max.y, localAABB.min.z },
        { localAABB.max.x, localAABB.max.y, localAABB.min.z },
        { localAABB.min.x, localAABB.min.y, localAABB.max.z },
        { localAABB.max.x, localAABB.min.y, localAABB.max.z },
        { localAABB.min.x, localAABB.max.y, localAABB.max.z },
        { localAABB.max.x, localAABB.max.y, localAABB.max.z }
    };

    FOBB faceBB;
    for (int32 i = 0; i < 8; ++i) {
        // 모델 매트릭스로 점을 변환 후, center를 더해준다.
        faceBB.corners[i] =  center + FMatrix::TransformVector(localVertices[i], modelMatrix);
    }

    OrientedBoundingBoxes.Add(faceBB);

}

void UPrimitiveBatch::AddCone(const FVector& center, float radius, float height, int segments, const FVector4& color, const FMatrix& modelMatrix)
{
    ConeSegmentCount = segments;
    FVector localApex = FVector(0, 0, 0);
    FCone cone;
    cone.ConeApex = center + FMatrix::TransformVector(localApex, modelMatrix);
    FVector localBaseCenter = FVector(height, 0, 0);
    cone.ConeBaseCenter = center + FMatrix::TransformVector(localBaseCenter, modelMatrix);
    cone.ConeRadius = radius;
    cone.ConeHeight = height;
    cone.Color = color;
    cone.ConeSegmentCount = ConeSegmentCount;
    Cones.Add(cone);
}

