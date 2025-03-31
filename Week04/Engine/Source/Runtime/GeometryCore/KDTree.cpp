#include "KDTree.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"   
#include "Editor/UnrealEd/PrimitiveBatch.h"
#include "Runtime/Core/Math/JungleMath.h"
#include <queue>


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
bool KDIsRayIntersectingSphere(const FVector& Origin, const FVector& Dir, const FVector& Center, float Radius, float& OutDistance)
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

// 생성자: 주어진 Bounds와 깊이로 노드를 초기화하며, 분할 축은 Bounds의 최장 축으로 결정
KDTreeNode::KDTreeNode(const FBoundingBox& InBounds, int InDepth)
    : Bounds(InBounds)
    , Depth(InDepth)
    , Left(nullptr)
    , Right(nullptr)
{
    // Bounds의 확장을 계산하여 가장 긴 축 결정
    FVector Extent = Bounds.max - Bounds.min;
    if (Extent.X >= Extent.Y && Extent.X >= Extent.Z) {
        Axis = 0;
    } else if (Extent.Y >= Extent.X && Extent.Y >= Extent.Z) {
        Axis = 1;
    } else {
        Axis = 2;
    }
    // 분할 좌표는 해당 축의 중앙값
    if (Axis == 0)
        SplitValue = (Bounds.min.X + Bounds.max.X) * 0.5f;
    else if (Axis == 1)
        SplitValue = (Bounds.min.Y + Bounds.max.Y) * 0.5f;
    else
        SplitValue = (Bounds.min.Z + Bounds.max.Z) * 0.5f;
}

KDTreeNode::~KDTreeNode()
{
    if (Left) {
        delete Left;
        Left = nullptr;
    }
    if (Right) {
        delete Right;
        Right = nullptr;
    }
}

// Insert: 객체의 위치를 기준으로 분할
void KDTreeNode::Insert(UPrimitiveComponent* Comp)
{
    // 객체의 월드 위치를 구함 (물체의 중심)
    FVector pos = Comp->GetWorldLocation();

    // 현재 노드 영역(Bounds)에 포함되지 않으면 삽입하지 않음
    if (!Bounds.Contains(pos)) { return; }

    // Leaf 노드이고, 저장 개수가 MAX_OBJECTS 미만이거나 최대 깊이에 도달한 경우 현재 노드에 추가
    if ((Left == nullptr && Right == nullptr) && (Components.Num() < MAX_OBJECTS || Depth >= MAX_DEPTH))
    {
        Components.Add(Comp);
        return;
    }

    // 아직 자식 노드가 생성되지 않았다면 분할
    if (Left == nullptr || Right == nullptr)
    {
        Subdivide();
    }

    // 분할 기준(SplitValue)와 객체 위치를 비교하여 왼쪽/오른쪽 자식에 삽입
    if (pos[Axis] < SplitValue)
        Left->Insert(Comp);
    else
        Right->Insert(Comp);
}
// Subdivide: 현재 노드의 영역을 선택된 축 기준으로 두 영역으로 나눔
void KDTreeNode::Subdivide()
{
    FVector leftMin = Bounds.min;
    FVector leftMax = Bounds.max;
    FVector rightMin = Bounds.min;
    FVector rightMax = Bounds.max;

    // 선택된 축에 대해 영역을 분할 (중앙값 SplitValue 기준)
    if (Axis == 0)
    {
        leftMax.X = SplitValue;
        rightMin.X = SplitValue;
    }
    else if (Axis == 1)
    {
        leftMax.Y = SplitValue;
        rightMin.Y = SplitValue;
    }
    else // Axis == 2
    {
        leftMax.Z = SplitValue;
        rightMin.Z = SplitValue;
    }

    Left = new KDTreeNode(FBoundingBox(leftMin, leftMax), Depth + 1);
    Right = new KDTreeNode(FBoundingBox(rightMin, rightMax), Depth + 1);

    // 기존에 저장된 객체들을 재분배 (객체의 위치를 기준으로)
    TArray<UPrimitiveComponent*> OldComponents = Components;
    Components.Empty();

    for (UPrimitiveComponent*& Comp : OldComponents)
    {
        FVector pos = Comp->GetWorldLocation();
        if (pos[Axis] < SplitValue)
            Left->Insert(Comp);
        else
            Right->Insert(Comp);
    }
}

void KDTreeNode::QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutComponents)
{
    if (!Frustum.Intersects(Bounds))
        return;

    if (Left)
        Left->QueryFrustum(Frustum, OutComponents);
    if (Right)
        Right->QueryFrustum(Frustum, OutComponents);

    for (UPrimitiveComponent* Comp : Components)
    {
        OutComponents.Add(Comp);
    }
}

void KDTreeNode::QueryFrustumUnique(const FFrustum& Frustum, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
{
    if (!Frustum.Intersects(Bounds))
        return;

    for (UPrimitiveComponent*& Comp : Components)
    {
        uint32 UUID = Comp->GetUUID();
        if (!UniqueUUIDs.Contains(UUID))
        {
            UniqueUUIDs.Add(UUID);
            OutComponents.Add(Comp);
        }
    }
    if (Left)
        Left->QueryFrustumUnique(Frustum, OutComponents, UniqueUUIDs);
    if (Right)
        Right->QueryFrustumUnique(Frustum, OutComponents, UniqueUUIDs);
}

void KDTreeNode::QueryRay(const FVector& Origin, const FVector& Dir, TArray<UPrimitiveComponent*>& OutComponents)
{
    float Distance;
    if (!Bounds.Intersect(Origin, Dir, Distance))
        return;

    if (Left)
        Left->QueryRay(Origin, Dir, OutComponents);
    if (Right)
        Right->QueryRay(Origin, Dir, OutComponents);

    for (UPrimitiveComponent*& Comp : Components)
    {
        OutComponents.Add(Comp);
    }
}

void KDTreeNode::QueryRayUnique(const FVector& Origin, const FVector& Dir, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
{
    float Distance;
    if (!Bounds.Intersect(Origin, Dir, Distance))
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
    if (Left)
        Left->QueryRayUnique(Origin, Dir, OutComponents, UniqueUUIDs);
    if (Right)
        Right->QueryRayUnique(Origin, Dir, OutComponents, UniqueUUIDs);
}


void KDTreeNode::RemoveComponent(UPrimitiveComponent* Comp)
{
    while (Components.Contains(Comp))
    {
        int32 Index = Components.Find(Comp);
        Components.RemoveAt(Index);
    }
    if (Left)
        Left->RemoveComponent(Comp);
    if (Right)
        Right->RemoveComponent(Comp);
}

void KDTreeNode::UpdateComponent(UPrimitiveComponent* Comp)
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

void KDTreeSystem::Build(const TArray<UPrimitiveComponent*>& Components)
{
    // Scene의 모든 객체의 위치를 기반으로 최소, 최대 점 계산
    FVector SceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector SceneMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (UPrimitiveComponent* Comp : Components)
    {
        FVector pos = Comp->GetWorldLocation();
        SceneMin = SceneMin.ComponentMin(pos);
        SceneMax = SceneMax.ComponentMax(pos);
    }
    if (Root)
    {
        delete Root;
        Root = nullptr;
    }
    Root = new KDTreeNode(FBoundingBox(SceneMin, SceneMax), 0);
    for (UPrimitiveComponent* Comp : Components)
    {
        Root->Insert(Comp);
    }
}

void KDTreeSystem::AddComponent(UPrimitiveComponent* Comp)
{
    // 객체의 위치를 기준으로 현재 트리의 Bounds 내에 포함되면 삽입,
    // 아니면 전체 재구축
    FVector pos = Comp->GetWorldLocation();
    if (Root && Root->Bounds.Contains(pos))
        Root->Insert(Comp);
    else
        Build({ Comp });
}

void KDTreeSystem::UpdateComponentPosition(UPrimitiveComponent* Comp)
{
    if (Root)
    {
        Root->RemoveComponent(Comp);
        AddComponent(Comp);
    }
}

// KDTreeNode::QueryRayClosestBestFirst: Best-First Traversal을 통해 Ray와 가장 가까운 컴포넌트를 반환
UPrimitiveComponent* KDTreeNode::QueryRayClosestBestFirst(const FVector& Origin, const FVector& Dir)
{
    FVector normDir = Dir.Normalize();
    std::priority_queue<KDPQEntry, std::vector<KDPQEntry>, KDPQEntryComparator> pq;
    float bestT = std::numeric_limits<float>::max();
    UPrimitiveComponent* bestComp = nullptr;

    float tEntry;
    if (Bounds.Intersect(Origin, normDir, tEntry))
    {
        pq.push({ this, tEntry });
    }

    while (!pq.empty())
    {
        KDPQEntry current = pq.top();
        pq.pop();

        // 이미 발견된 교차점보다 멀면 더 이상 탐색할 필요 없음
        if (current.tEntry >= bestT)
            continue;

        KDTreeNode* node = current.Node;
        // Leaf 노드인 경우: 이 노드에 포함된 각 컴포넌트에 대해 Ray-Sphere 교차 판정을 수행
        if (!node->Left && !node->Right)
        {
            for (UPrimitiveComponent* comp : node->Components)
            {
                float tInter;
                FBoundingBox box = GetWorldBox(comp);
                // AABB를 구로 근사: 중심과 반지름 계산
                FVector center = (box.min + box.max) * 0.5f;
                float radius = (box.max - box.min).Magnitude() * 0.5f;

                if (KDIsRayIntersectingSphere(Origin, normDir, center, radius, tInter))
                {
                    if (tInter < bestT)
                    {
                        bestT = tInter;
                        bestComp = comp;
                    }
                }
            }
        }
        else  // 내부 노드인 경우: 자식 노드들을 검사하여 큐에 삽입
        {
            if (node->Left)
            {
                float tLeft;
                if (node->Left->Bounds.Intersect(Origin, normDir, tLeft) && tLeft < bestT)
                    pq.push({ node->Left, tLeft });
            }
            if (node->Right)
            {
                float tRight;
                if (node->Right->Bounds.Intersect(Origin, normDir, tRight) && tRight < bestT)
                    pq.push({ node->Right, tRight });
            }
        }
    }

    if (bestComp)
    {
        UE_LOG(LogLevel::Display, TEXT("Closest intersection distance: %.2f"), bestT);
    return bestComp;
    }
    return nullptr;
}