// BVHNode.cpp
#include "BVHNode.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"   
#include "Editor/UnrealEd/PrimitiveBatch.h"
#include "Runtime/Core/Math/JungleMath.h"

#include <cassert>
#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include <limits>
#include <mutex>


#include <immintrin.h>
#include <queue>

static std::mutex g_resultMutex;

bool IntersectSphereRayOptimized(const FVector& rayOrigin, const FVector& rayDir, const FVector& sphereCenter, float radius, float& outDistance)
{
    FVector m = rayOrigin - sphereCenter;
    float b = FVector::DotProduct(m, rayDir);
    float c = FVector::DotProduct(m, m) - radius * radius;

    // 비트마스킹을 이용한 조건 검사
    int mask = ((b > 0) << 1) | (c > 0);
    if (mask == 3) return false;

    float discr = b * b - c;
    if (discr < 0) return false;

    outDistance = -b - FMath::Sqrt(discr);
    if (outDistance < 0) outDistance = 0;

    return true;
}

inline bool FastIsRayIntersectingSphere(const FVector& Origin, const FVector& Dir, const FVector& Center, float Radius, float& OutDistance)
{
    FVector L = Center - Origin;
    float tca = L.Dot(Dir);
    float d2 = L.Dot(L) - tca * tca;
    float radius2 = Radius * Radius;

    // 비트마스킹으로 비교
    if ((*(uint32_t*)&d2) > (*(uint32_t*)&radius2))
        return false;

    float thc = sqrt(radius2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    // t0이 더 크면 스왑
    if (t0 > t1) std::swap(t0, t1);

    // 비트마스킹 사용한 음수 판정
    if ((*(uint32_t*)&t0) >> 31) t0 = t1;
    if ((*(uint32_t*)&t0) >> 31) return false;

    OutDistance = t0;
    return true;
}

// Helper: 객체의 월드 바운딩박스를 계산
static FBoundingBox GetWorldBox(UPrimitiveComponent* Comp)
{
    FMatrix Model = JungleMath::CreateModelMatrix(
        Comp->GetWorldLocation(),
        Comp->GetWorldRotation(),
        Comp->GetWorldScale()
    );
    return UPrimitiveBatch::GetWorldBoundingBox(Comp->AABB, Comp->GetWorldLocation(), Model);
}

// Helper: 객체의 바운딩박스 중심(centroid)
static FVector GetCentroid(UPrimitiveComponent* Comp)
{
    FBoundingBox box = GetWorldBox(Comp);
    return (box.min + box.max) * 0.5f;
}
// Ray-Sphere 충돌 검사 함수
bool IsRayIntersectingSphere(const FVector& Origin, const FVector& Dir, const FVector& Center, float Radius, float& OutDistance)
{
    FVector L = Center - Origin;
    float tca = L.Dot(Dir);
    float d2 = L.Dot(L) - tca * tca;
    float radius2 = Radius * Radius;

    // 교차하지 않음
    if (d2 > radius2)
        return false;

    float thc = sqrt(radius2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    // t0이 더 크면 스왑
    if (t0 > t1)
        std::swap(t0, t1);

    // t0이 음수면 t1로 교체
    if (t0 < 0)
        t0 = t1;

    // 여전히 음수면 교차하지 않음
    if (t0 < 0)
        return false;

    // 교차 거리 저장
    OutDistance = t0;
    return true;
}


bool IsRayIntersectingSphere(const FVector& Origin, const FVector& Dir, const FVector& Center, float Radius, float& OutIntersection, float& OutTca)
{
    FVector L = Center - Origin;
    float tca = L.Dot(Dir);
    OutTca = tca;
    float d2 = L.Dot(L) - tca * tca;
    float radius2 = Radius * Radius;

    if (d2 > radius2)
        return false;

    float thc = sqrt(radius2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;
    if (t0 > t1)
        std::swap(t0, t1);
    if (t0 < 0)
        t0 = t1;
    if (t0 < 0)
        return false;

    OutIntersection = t0;
    return true;
}

BVHNode::BVHNode(TArray<UPrimitiveComponent*>& Objects, int InDepth)
    : Left(nullptr)
    , Right(nullptr)
    , Depth(InDepth)
{
    Build(Objects, InDepth);
}

BVHNode::~BVHNode()
{
    if (Left) { delete Left; Left = nullptr; }
    if (Right) { delete Right; Right = nullptr; }
}

#if BVH_SAT
void BVHNode::Build(TArray<UPrimitiveComponent*>& Objects, int InDepth)
{
    if (Objects.Num() == 0) return;

    // 노드의 Bounds: 모든 객체의 World 바운딩박스의 합집합
    FBoundingBox unionBox = GetWorldBox(Objects[0]);
    for (int i = 1; i < Objects.Num(); ++i)
    {
        unionBox = unionBox.Union(GetWorldBox(Objects[i]));
    }
    Bounds = unionBox;

    // Leaf 조건: 객체 수가 작거나 최대 깊이에 도달한 경우
    if (Objects.Num() <= MAX_OBJECTS || InDepth >= MAX_DEPTH)
    {
        Components = Objects;
        return;
    }

    // SAT 기반 분할 축 계산
    int bestAxis = 0;
    float bestOverlap = FLT_MAX;
    float bestSplit = 0.0f;

    for (int axis = 0; axis < 3; ++axis)
    {
        // 객체들을 중심 기준으로 정렬
        Objects.Sort([axis](UPrimitiveComponent* A, UPrimitiveComponent* B) {
            return GetCentroid(A)[axis] < GetCentroid(B)[axis];
            });

        // 누적 바운딩 박스 계산
        TArray<FBoundingBox> leftBoxes, rightBoxes;
        leftBoxes.SetNum(Objects.Num());
        rightBoxes.SetNum(Objects.Num());

        FBoundingBox leftBox = GetWorldBox(Objects[0]);
        leftBoxes[0] = leftBox;
        for (int i = 1; i < Objects.Num(); ++i)
        {
            leftBox = leftBox.Union(GetWorldBox(Objects[i]));
            leftBoxes[i] = leftBox;
        }

        FBoundingBox rightBox = GetWorldBox(Objects[Objects.Num() - 1]);
        rightBoxes[Objects.Num() - 1] = rightBox;
        for (int i = Objects.Num() - 2; i >= 0; --i)
        {
            rightBox = rightBox.Union(GetWorldBox(Objects[i]));
            rightBoxes[i] = rightBox;
        }

        // SAT 기반 분할 비용 계산
        for (int i = 0; i < Objects.Num() - 1; ++i)
        {
            int leftCount = i + 1;
            int rightCount = Objects.Num() - leftCount;

            float overlap = leftBoxes[i].SurfaceArea() * leftCount + rightBoxes[i + 1].SurfaceArea() * rightCount;
            if (overlap < bestOverlap)
            {
                bestOverlap = overlap;
                bestAxis = axis;
                bestSplit = GetCentroid(Objects[i])[axis];
            }
        }
    }

    // SAT 기반 최적의 분할 축과 값을 기반으로 객체 분할
    TArray<UPrimitiveComponent*> leftObjects, rightObjects;
    for (UPrimitiveComponent* Comp : Objects)
    {
        FVector centroid = GetCentroid(Comp);
        if (centroid[bestAxis] < bestSplit)
            leftObjects.Add(Comp);
        else
            rightObjects.Add(Comp);
    }

    // 한쪽으로 치우친 경우 강제 균등 분할
    if (leftObjects.Num() == 0 || rightObjects.Num() == 0)
    {
        int half = Objects.Num() / 2;
        leftObjects.Empty();
        rightObjects.Empty();
        for (int i = 0; i < Objects.Num(); ++i)
        {
            if (i < half)
                leftObjects.Add(Objects[i]);
            else
                rightObjects.Add(Objects[i]);
        }
    }

    // 자식 노드 생성
    Left = new BVHNode(leftObjects, InDepth + 1);
    Right = new BVHNode(rightObjects, InDepth + 1);

    // 부모 노드의 Bounds를 자식 노드의 Bounds로 갱신
    Bounds = Left->Bounds.Union(Right->Bounds);
}
#else
void BVHNode::Build(const TArray<UPrimitiveComponent*>& Objects, int InDepth)
{
    if (Objects.Num() == 0) return;

    // 노드의 Bounds: 모든 객체의 World 바운딩박스의 합집합
    FBoundingBox unionBox = GetWorldBox(Objects[0]);
    for (int i = 1; i < Objects.Num(); ++i)
    {
        unionBox = unionBox.Union(GetWorldBox(Objects[i]));
    }
    Bounds = unionBox;

    // Leaf 조건: 객체 수가 작거나 최대 깊이에 도달한 경우
    if (Objects.Num() <= MAX_OBJECTS || InDepth >= MAX_DEPTH)
    {
        Components = Objects;
        return;
    }

    // 분할 기준: 모든 객체의 중심을 포함하는 바운딩박스의 최장 축
    FBoundingBox centroidBox = FBoundingBox(GetCentroid(Objects[0]), GetCentroid(Objects[0]));
    for (int i = 1; i < Objects.Num(); ++i)
    {
        FVector centroid = GetCentroid(Objects[i]);
        centroidBox = centroidBox.Union(FBoundingBox(centroid, centroid));
    }

    FVector extents = centroidBox.max - centroidBox.min;
    int axis = 0;
    if (extents.Y >= extents.X && extents.Y >= extents.Z)
        axis = 1;
    else if (extents.Z >= extents.X && extents.Z >= extents.Y)
        axis = 2;

    float splitValue = (centroidBox.min[axis] + centroidBox.max[axis]) * 0.5f;

    // 객체들을 분할: 중심이 splitValue보다 작은 것과 큰 것으로 분리
    TArray<UPrimitiveComponent*> leftObjects, rightObjects;
    for (UPrimitiveComponent* Comp : Objects)
    {
        FVector centroid = GetCentroid(Comp);
        if (centroid[axis] < splitValue)
            leftObjects.Add(Comp);
        else
            rightObjects.Add(Comp);
    }

    // 한쪽에 너무 치우친 경우 강제로 균등 분할
    if (leftObjects.Num() == 0 || rightObjects.Num() == 0)
    {
        int half = Objects.Num() / 2;
        leftObjects.Empty();
        rightObjects.Empty();
        for (int i = 0; i < Objects.Num(); ++i)
        {
            if (i < half)
                leftObjects.Add(Objects[i]);
            else
                rightObjects.Add(Objects[i]);
        }
    }

    // 자식 노드 생성
    Left = new BVHNode(leftObjects, InDepth + 1);
    Right = new BVHNode(rightObjects, InDepth + 1);

    // 부모 노드의 Bounds를 자식 노드의 Bounds로 갱신
    Bounds = Left->Bounds.Union(Right->Bounds);
}
#endif
void BVHNode::QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutComponents)
{
    // 노드의 Bounds와 프러스텀의 교차 여부로 가지치기
    if (!Frustum.Intersects(Bounds))
        return;

    if (Left) Left->QueryFrustum(Frustum, OutComponents);
    if (Right) Right->QueryFrustum(Frustum, OutComponents);

    // Leaf인 경우, 저장된 모든 객체를 결과에 추가
    for (UPrimitiveComponent* Comp : Components)
    {
        OutComponents.Add(Comp);
    }
}

void BVHNode::QueryFrustumUnique(const FFrustum& Frustum, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
{
    if (!Frustum.Intersects(Bounds))
        return;

    for (UPrimitiveComponent* Comp : Components)
    {
        uint32 UUID = Comp->GetUUID();
        if (!UniqueUUIDs.Contains(UUID))
        {
            UniqueUUIDs.Add(UUID);
            OutComponents.Add(Comp);
        }
    }
    if (Left) Left->QueryFrustumUnique(Frustum, OutComponents, UniqueUUIDs);
    if (Right) Right->QueryFrustumUnique(Frustum, OutComponents, UniqueUUIDs);
}

void BVHNode::QueryRay(const FVector& Origin, const FVector& Dir, TArray<UPrimitiveComponent*>& OutComponents)
{
    float distance;
    // BVH 노드의 바운딩 구와 Ray의 교차 여부 확인
    if (!Bounds.Intersect(Origin, Dir, distance))
        return;

    // Leaf 노드의 경우: 모든 객체와 거리 비교
    if (!Left && !Right)
    {
        for (UPrimitiveComponent* Comp : Components)
        {
            float objDistance;
            FBoundingBox box = GetWorldBox(Comp);

            // 구의 중심과 반지름 계산
            FVector center = (box.min + box.max) * 0.5f;
            float radius = (box.max - box.min).Length() * 0.5f;

            // Ray-Sphere 교차 검사 함수 사용
            if (IsRayIntersectingSphere(Origin, Dir, center, radius, objDistance))
            {
                OutComponents.Add(Comp);
            }
        }
        return;
    }

    // 자식 노드 탐색
    if (Left) Left->QueryRay(Origin, Dir, OutComponents);
    if (Right) Right->QueryRay(Origin, Dir, OutComponents);
}

void BVHNode::QueryRayUnique(const FVector& Origin, const FVector& Dir, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
{
    float distance;
    if (!Bounds.Intersect(Origin, Dir, distance))
        return;

    for (UPrimitiveComponent* Comp : Components)
    {
        uint32 UUID = Comp->GetUUID();
        if (!UniqueUUIDs.Contains(UUID))
        {
            UniqueUUIDs.Add(UUID);
            OutComponents.Add(Comp);
        }
    }
    if (Left) Left->QueryRayUnique(Origin, Dir, OutComponents, UniqueUUIDs);
    if (Right) Right->QueryRayUnique(Origin, Dir, OutComponents, UniqueUUIDs);
}

void BVHNode::RemoveComponent(UPrimitiveComponent* Comp)
{
    while (Components.Contains(Comp))
    {
        int32 Index = Components.Find(Comp);
        Components.RemoveAt(Index);
    }
    if (Left) Left->RemoveComponent(Comp);
    if (Right) Right->RemoveComponent(Comp);
}

void BVHNode::UpdateComponent(UPrimitiveComponent* Comp)
{
    FVector pos = Comp->GetWorldLocation();
    if (!Bounds.Contains(pos))
    {
        RemoveComponent(Comp);
        return;
    }
    if (Left && Left->Bounds.Contains(pos))
    {
        Left->UpdateComponent(Comp);
        return;
    }
    if (Right && Right->Bounds.Contains(pos))
    {
        Right->UpdateComponent(Comp);
        return;
    }
    int32 Index = Components.Find(Comp);
    if (Index != INDEX_NONE)
    {
        Components[Index] = Comp;
    }
}

void BVHNode::QueryRayClosestInternal(const FVector& Origin, const FVector& Dir, UPrimitiveComponent*& OutClosest, float& OutMinDistance)
{
    float dummy;
    // 현재 노드의 Bounds와 Ray 교차 검사 (AABB 기반)
    if (!Bounds.Intersect(Origin, Dir, dummy))
        return;

    // Leaf 노드인 경우: 각 컴포넌트에 대해 구 기반 교차 검사 수행
    if (!Left && !Right)
    {
        for (UPrimitiveComponent*& Comp : Components)
        {
            float tIntersection;
            FBoundingBox box = GetWorldBox(Comp);
            // 구의 중심과 반지름 계산 (AABB를 사용)
            FVector center = (box.min + box.max) * 0.5f;
            float radius = (box.max - box.min).Length() * 0.5f;

            // Ray와 구의 교차 검사: Ray Origin에서의 교차 거리를 계산
            if (IsRayIntersectingSphere(Origin, Dir, center, radius, tIntersection))
            {
                if (tIntersection < OutMinDistance)
                {
                    OutMinDistance = tIntersection;
                    OutClosest = Comp;
                }
            }
        }
        return;
    }

    // 내부 노드인 경우: 자식 노드를 재귀적으로 탐색하여 최소 교차 거리를 갱신
    if (Left)
        Left->QueryRayClosestInternal(Origin, Dir, OutClosest, OutMinDistance);
    if (Right)
        Right->QueryRayClosestInternal(Origin, Dir, OutClosest, OutMinDistance);
}

// public 함수: 내부 재귀 함수를 호출하여 가장 가까운 컴포넌트를 반환합니다.
UPrimitiveComponent* BVHNode::QueryRayClosest(const FVector& Origin, const FVector& Dir)
{
    UPrimitiveComponent* closest = nullptr;
    float minDistance = FLT_MAX;
    QueryRayClosestInternal(Origin, Dir, closest, minDistance);
    if (closest)
    {
        //UE_LOG(LogLevel::Display, TEXT("Closest intersection distance: %.2f"), minDistance);
        return closest;
    }
    return nullptr;
}


UPrimitiveComponent* BVHNode::QueryRayClosestBestFirst(const FVector& Origin, const FVector& Dir)
{
    FVector normDir = Dir.GetSafeNormal();

    std::priority_queue<PQEntry, std::vector<PQEntry>, PQEntryComparator> pq;
    float bestT = FLT_MAX;
    UPrimitiveComponent* bestComp = nullptr;

    float tEntry;
    if (Bounds.Intersect(Origin, normDir, tEntry)) {
        pq.push({ this, tEntry });
    }

    while (!pq.empty())
    {
        PQEntry current = pq.top();
        pq.pop();

        // 만약 현재 노드의 진입 거리가 이미 최단 교차보다 멀다면 더 이상 탐색할 필요가 없음.
        if (current.tEntry >= bestT)
            continue;

        BVHNode* node = current.Node;

        // Leaf 노드인 경우, 이 노드에 있는 모든 컴포넌트에 대해 Ray-Sphere 교차 검사 수행
        if (!node->Left && !node->Right)
        {
            for (UPrimitiveComponent*& comp : node->Components)
            {
                float tInter;
                FBoundingBox box = GetWorldBox(comp);
                // AABB를 구로 근사: 구의 중심과 반지름 계산
                FVector center = (box.min + box.max) * 0.5f;
                float radius = (box.max - box.min).Length() * 0.5f;

                // 비트마스킹을 이용한 빠른 충돌 검사
                if (FastIsRayIntersectingSphere(Origin, normDir, center, radius, tInter))
                {
                    if (tInter < bestT)
                    {
                        bestT = tInter;
                        bestComp = comp;
                    }
                }
            }
        }
        else // 내부 노드인 경우, 자식 노드를 검사
        {
            if (node->Left)
            {
                float tLeft;
                if (node->Left->Bounds.Intersect(Origin, normDir, tLeft) && tLeft < bestT)
                {
                    pq.push({ node->Left, tLeft });
                }
            }
            if (node->Right)
            {
                float tRight;
                if (node->Right->Bounds.Intersect(Origin, normDir, tRight) && tRight < bestT)
                {
                    pq.push({ node->Right, tRight });
                }
            }
        }
    }

    if (bestComp)
    {
        //UE_LOG(LogLevel::Display, TEXT("Closest intersection distance: %.2f"), bestT);
        return bestComp;
    }
    return nullptr;
}


