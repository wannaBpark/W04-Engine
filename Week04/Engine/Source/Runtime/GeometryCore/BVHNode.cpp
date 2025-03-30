// BVHNode.cpp
#include "BVHNode.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"   
#include "Editor/UnrealEd/PrimitiveBatch.h"
#include "Runtime/Core/Math/JungleMath.h"

#include <algorithm>
#include <cassert>

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

BVHNode::BVHNode(const TArray<UPrimitiveComponent*>& Objects, int InDepth)
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
    if (!Bounds.Intersect(Origin, Dir, distance))
        return;

    if (Left) Left->QueryRay(Origin, Dir, OutComponents);
    if (Right) Right->QueryRay(Origin, Dir, OutComponents);
    for (UPrimitiveComponent* Comp : Components)
    {
        OutComponents.Add(Comp);
    }
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
