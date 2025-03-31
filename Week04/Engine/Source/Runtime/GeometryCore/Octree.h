#pragma once
#include "Define.h"
#include "Container/Array.h"
#include "Container/Set.h"
#define _LOOSEOCTREE false
// TODO : 변경된 컴포넌트 재빌드 필요 (Octree->Build(DirtyComponents);
class UPrimitiveComponent;

class OctreeNode
{
public:
    FBoundingBox Bounds;
    TArray<UPrimitiveComponent*> Components;
    OctreeNode* Children[8] = { nullptr, };
    int Depth = 0;
    inline static constexpr int MAX_DEPTH = 5;    // 최대 깊이
    inline static constexpr int MAX_OBJECTS = 10; // 한 부모가 최대로 가질 자식 수
    inline static constexpr float LOOSE_FACTOR = 1.5f;  // 느슨한 경계 확장 계수

    OctreeNode(const FBoundingBox& InBounds, int InDepth = 0)
        : Bounds(InBounds)
        , Depth(InDepth)
    {
    }

    ~OctreeNode()
    {
        for (const auto& Child : Children)
        {
            delete Child;
        }
    }

    void Insert(UPrimitiveComponent* Comp);
    void Subdivide();
    void QueryRay(
        const FVector& Origin, const FVector& Dir,
        TArray<UPrimitiveComponent*>& OutComponents
    );
    void QueryRayUnique(const FVector& Origin, const FVector& Dir, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UUIDs);
    void UpdateComponent(UPrimitiveComponent* Comp);
    void RemoveComponent(UPrimitiveComponent* Comp);

    void QueryFrustumOcclusionCulling(const FFrustum& Frustum, const FVector& CameraPos, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs);

    void QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutComponents);
    void QueryFrustumUnique(const FFrustum& Frustum, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs);
    void CollectComponents(TArray<UPrimitiveComponent*>& OutComponents)
    {
        for (const auto& MyComp : Components)
        {
            OutComponents.Add(MyComp);
        }

        if (Children[0])
        {
            for (const auto& Child : Children)
            {
                Child->CollectComponents(OutComponents);
            }
        }
    }

    FBoundingBox GetLooseBounds() const
    {
        FVector Center = (Bounds.min + Bounds.max) * 0.5f;
        FVector HalfExtent = (Bounds.max - Bounds.min) * 0.5f;
        HalfExtent = HalfExtent * LOOSE_FACTOR;
        return FBoundingBox(Center - HalfExtent, Center + HalfExtent);
    }
};

class OctreeSystem
{
public:
    OctreeNode* Root = nullptr;

    void Build(const TArray<UPrimitiveComponent*>& Components);
    void AddComponent(UPrimitiveComponent* Comp);

    void UpdateComponentPosition(UPrimitiveComponent* Comp);
};