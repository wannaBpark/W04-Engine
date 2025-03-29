#include "KDTree.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"   
#include "Editor/UnrealEd/PrimitiveBatch.h"
#include "Runtime/Core/Math/JungleMath.h"


// 생성자: 주어진 Bounds와 깊이로 노드를 초기화하며, 분할 축은 Bounds의 최장 축으로 결정
KDTreeNode::KDTreeNode(const FBoundingBox& InBounds, int InDepth)
    : Bounds(InBounds)
    , Depth(InDepth)
    , Left(nullptr)
    , Right(nullptr)
{
    // Bounds의 확장을 계산하여 가장 긴 축 결정
    FVector Extent = Bounds.max - Bounds.min;
    if (Extent.x >= Extent.y && Extent.x >= Extent.z) {
        Axis = 0;
    } else if (Extent.y >= Extent.x && Extent.y >= Extent.z) {
        Axis = 1;
    } else {
        Axis = 2;
    }
    // 분할 좌표는 해당 축의 중앙값
    if (Axis == 0)
        SplitValue = (Bounds.min.x + Bounds.max.x) * 0.5f;
    else if (Axis == 1)
        SplitValue = (Bounds.min.y + Bounds.max.y) * 0.5f;
    else
        SplitValue = (Bounds.min.z + Bounds.max.z) * 0.5f;
}

KDTreeNode::~KDTreeNode()
{
    if (Left)
    {
        delete Left;
        Left = nullptr;
    }
    if (Right)
    {
        delete Right;
        Right = nullptr;
    }
}

void KDTreeNode::Insert(UPrimitiveComponent* Comp)
{
    // 컴포넌트의 월드 AABB 계산 (JungleMath 및 UPrimitiveBatch 사용)
    FMatrix Model = JungleMath::CreateModelMatrix(
        Comp->GetWorldLocation(),
        Comp->GetWorldRotation(),
        Comp->GetWorldScale()
    );
    FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
        Comp->AABB, Comp->GetWorldLocation(), Model
    );

    // 현재 노드의 Bounds와 교차하지 않으면 삽입하지 않음
    if (!WorldBBox.Intersects(Bounds))
        return;

    // 자식이 없고, 아직 저장 객체 수가 MAX_OBJECTS 미만이거나 최대 깊이에 도달한 경우 현재 노드에 추가
    if ((Left == nullptr && Right == nullptr) && (Components.Num() < MAX_OBJECTS || Depth >= MAX_DEPTH))
    {
        Components.Add(Comp);
        return;
    }

    // 아직 자식이 생성되지 않았다면 분할 수행
    if (Left == nullptr || Right == nullptr)
    {
        Subdivide();
    }

    // 좌측 자식의 Bounds에 완전히 포함되면 좌측에 삽입
    if (Left->Bounds.Intersects(WorldBBox))
    {
        Left->Insert(Comp);
        return;
    }
    // 우측 자식의 Bounds에 완전히 포함되면 우측에 삽입
    else if (Right->Bounds.Intersects(WorldBBox))
    {
        Right->Insert(Comp);
        return;
    }
    // 어느 한쪽에도 완전히 포함되지 않으면 현재 노드에 추가
    else Components.Add(Comp);
}

void KDTreeNode::Subdivide()
{
    // 현재 노드의 Bounds를 해당 Axis에 대해 중앙으로 분할
    FVector leftMin = Bounds.min;
    FVector leftMax = Bounds.max;
    FVector rightMin = Bounds.min;
    FVector rightMax = Bounds.max;

    // 선택된 축에 따라 분할
        if (Axis == 0)
        {
            leftMax.x = SplitValue;
            rightMin.x = SplitValue;
        }
        else if (Axis == 1)
        {
            leftMax.y = SplitValue;
            rightMin.y = SplitValue;
        }
        else // Axis == 2
        {
            leftMax.z = SplitValue;
            rightMin.z = SplitValue;
        }

    Left = new KDTreeNode(FBoundingBox(leftMin, leftMax), Depth + 1);
    Right = new KDTreeNode(FBoundingBox(rightMin, rightMax), Depth + 1);

    // 기존에 저장된 컴포넌트들을 재분배
    TArray<UPrimitiveComponent*> OldComponents;
    for (auto& Comp : Components) { OldComponents.Add(Comp); }
    Components.Empty();

    for (UPrimitiveComponent* Comp : OldComponents)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(
            Comp->GetWorldLocation(),
            Comp->GetWorldRotation(),
            Comp->GetWorldScale()
        );
        FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
            Comp->AABB, Comp->GetWorldLocation(), Model
        );
        if (Left->Bounds.Intersects(WorldBBox))
        {
            Left->Insert(Comp);
        }
        else if (Right->Bounds.Intersects(WorldBBox))
        {
            Right->Insert(Comp);
        }
        else
        {
            Components.Add(Comp);
        }
    }
}

void KDTreeNode::QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutComponents)
{
    // 현재 노드의 Bounds가 Frustum과 교차하지 않으면 가지치기
    if (!Frustum.Intersects(Bounds))
        return;

    
    // 자식 노드가 있다면 재귀적으로 호출
    if (Left)
        Left->QueryFrustum(Frustum, OutComponents);
    if (Right)
        Right->QueryFrustum(Frustum, OutComponents);
    // 현재 노드의 컴포넌트 추가
    
    for (UPrimitiveComponent* Comp : Components)
    {
        OutComponents.Add(Comp);
    }
}

void KDTreeNode::QueryFrustumUnique(const FFrustum& Frustum, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
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
    /*if (!Left && !Right)*/ {
        for (UPrimitiveComponent*& Comp : Components)
        {
            OutComponents.Add(Comp);
        }
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
    // 컴포넌트의 월드 AABB 재계산
    FMatrix Model = JungleMath::CreateModelMatrix(
        Comp->GetWorldLocation(),
        Comp->GetWorldRotation(),
        Comp->GetWorldScale()
    );
    FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
        Comp->AABB, Comp->GetWorldLocation(), Model
    );
    if (!WorldBBox.Intersects(Bounds))
    {
        RemoveComponent(Comp);
        return;
    }

    // 자식 노드에 포함되는지 검사하여 재귀 호출
    if (Left && Left->Bounds.Intersects(WorldBBox))
    {
        Left->UpdateComponent(Comp);
        return;
    }
    if (Right && Right->Bounds.Intersects(WorldBBox))
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

// Scene의 모든 컴포넌트로부터 전체 AABB를 계산하여 kd‑tree를 구성
void KDTreeSystem::Build(const TArray<UPrimitiveComponent*>& Components)
{
    FVector SceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector SceneMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (UPrimitiveComponent* Comp : Components)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(
            Comp->GetWorldLocation(),
            Comp->GetWorldRotation(),
            Comp->GetWorldScale()
        );
        FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
            Comp->AABB, Comp->GetWorldLocation(), Model
        );
        SceneMin = SceneMin.ComponentMin(WorldBBox.min);
        SceneMax = SceneMax.ComponentMax(WorldBBox.max);
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

// 단일 컴포넌트 추가 (Bounds 변경 시 전체 재구축 고려)
void KDTreeSystem::AddComponent(UPrimitiveComponent* Comp)
{
    if (Root && Root->Bounds.Contains(UPrimitiveBatch::GetWorldBoundingBox(Comp->AABB, Comp->GetWorldLocation(), JungleMath::CreateModelMatrix(Comp->GetWorldLocation(), Comp->GetWorldRotation(), Comp->GetWorldScale()))))
        Root->Insert(Comp);
    else
    {
        // 루트 영역이 변경될 경우 전체 재구축
        Build({ Comp });
    }
}

// 위치 변경 등으로 인한 업데이트
void KDTreeSystem::UpdateComponentPosition(UPrimitiveComponent* Comp)
{
    if (Root)
    {
        Root->RemoveComponent(Comp);
        AddComponent(Comp);
    }
}
bool RayIntersectsObject(const FVector& RayOrigin, const FVector& RayDir, UPrimitiveComponent* Candidate, float& outDistance, int& outIntersectCount)
{
    // Candidate의 월드 변환 행렬 계산
    FMatrix Model = JungleMath::CreateModelMatrix(
        Candidate->GetWorldLocation(),
        Candidate->GetWorldRotation(),
        Candidate->GetWorldScale()
    );
    // 월드 AABB 계산 (UPrimitiveBatch를 통해 구함)
    FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
        Candidate->AABB, Candidate->GetWorldLocation(), Model
    );

    // slab method를 사용한 ray-AABB 교차 테스트
    // outDistance에는 ray 원점에서 AABB와의 교차까지의 거리가 반환됩니다.
    if (WorldBBox.Intersect(RayOrigin, RayDir, outDistance))
    {
        // 여기서는 간단히 교차 횟수를 1로 설정
        outIntersectCount = 1;
        return true;
    }
    else
    {
        return false;
    }
}

void KDTreeSystem::FastRayPick(const FVector& RayOrigin, const FVector& RayDir, 
    TArray<UPrimitiveComponent*>& OutSortedCandidates)
{
    TArray<UPrimitiveComponent*> CandidateArray;
    // 1. kd‑tree를 통해 ray와 교차하는 후보 컴포넌트들을 추출
    if (Root)
    {
        Root->QueryRay(RayOrigin, RayDir, CandidateArray);
    }

    // 2. 각 후보에 대해 대략적인 거리(예: AABB 중심 기준)를 계산하여 정렬
    TArray<std::pair<UPrimitiveComponent*, float>> SortedCandidates;
    for (UPrimitiveComponent*& comp : CandidateArray)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(
            comp->GetWorldLocation(),
            comp->GetWorldRotation(),
            comp->GetWorldScale()
        );
        FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
            comp->AABB, comp->GetWorldLocation(), Model
        );
        FVector center = (WorldBBox.min + WorldBBox.max) * 0.5f;
        FVector toCenter = center - RayOrigin;
        float approxDistance = toCenter.x * toCenter.x + toCenter.y * toCenter.y + toCenter.z * toCenter.z;
        float outDistance;
        if (WorldBBox.Intersect(RayOrigin, RayDir, outDistance))
        {
            SortedCandidates.Add(std::make_pair(comp, approxDistance));
        }
    }
    int idx = 0;
    for (auto& i : SortedCandidates) { 
        OutSortedCandidates.Add(i.first); 
        if (++idx >= 20) { break; } // TODO : HARD CODING! 
    }
    return;
    //SortedCandidates.Sort([](const std::pair<UPrimitiveComponent*, float>& A, const std::pair<UPrimitiveComponent*, float>& B)
    //    {
    //        return A.second < B.second;
    //    });
    //return SortedCandidates[0].first;
    //// 3. 가까운 후보부터 정밀한 ray-picking 테스트 수행
    //UPrimitiveComponent* BestCandidate = nullptr;
    //float BestDistance = FLT_MAX;
    //int BestIntersectCount = 0;
    //for (const auto& candidate : SortedCandidates)
    //{
    //    UPrimitiveComponent* comp = candidate.first;
    //    float preciseDistance = 0.0f;
    //    int intersectCount = 0;
    //    // 정밀한 ray-AABB 교차 테스트 수행
    //    if (RayIntersectsObject(RayOrigin, RayDir, comp, preciseDistance, intersectCount))
    //    {
    //        if (preciseDistance < BestDistance ||
    //            (std::fabs(preciseDistance - BestDistance) < FLT_EPSILON && intersectCount > BestIntersectCount))
    //        {
    //            BestDistance = preciseDistance;
    //            BestIntersectCount = intersectCount;
    //            BestCandidate = comp;
    //            // 충분히 가까운 hit를 찾으면 바로 종료할 수도 있음.
    //            // break;
    //        }
    //    }
    //}

    //return BestCandidate;
}