#pragma once
#include "Define.h"
#include "Container/Array.h"

// TODO : 변경된 컴포넌트 재빌드 필요 (Octree->Build(DirtyComponents);
class UPrimitiveComponent;

class OctreeNode
{
public:
    FBoundingBox Bounds;
    TArray<UPrimitiveComponent*> Components;
    OctreeNode* Children[8] = { nullptr };
    int Depth{ 0 };
    inline static const int MAX_DEPTH = 5;          // 최대 깊이
    inline static const int MAX_OBJECTS = 12;       // 한 부모가 최대로 가질 자식 수

    OctreeNode(const FBoundingBox& InBounds, int InDepth = 0)
        : Bounds(InBounds), Depth(InDepth)
    {

    }

    ~OctreeNode() { for (auto& Child : Children) delete Child; }

    void Insert(UPrimitiveComponent * Comp);
    void Subdivide();
    void QueryRay(const FVector& Origin, const FVector& Dir,
        TArray<UPrimitiveComponent*>& OutComponents);
    void UpdateComponent(UPrimitiveComponent* Comp);
    void RemoveComponent(UPrimitiveComponent* Comp);
};

class OctreeSystem
{
public:
    OctreeNode* Root{ nullptr };

    void Build(const TArray<UPrimitiveComponent*>& Components);
    void AddComponent(UPrimitiveComponent* Comp);

    void UpdateComponentPosition(UPrimitiveComponent* Comp);

};
