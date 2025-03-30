// BVHNode.h
#pragma once
#include "Define.h"
#include "Container/Array.h"
#include "Container/Set.h"

#define BVH_SAT false

class UPrimitiveComponent;
class BVHNode
{
public:
    FBoundingBox Bounds;                         // 이 노드가 포함하는 모든 물체의 바운딩박스의 합집합
    TArray<UPrimitiveComponent*> Components;     // Leaf인 경우 저장되는 물체들
    BVHNode* Left;
    BVHNode* Right;
    int Depth;

    // 분할 조건
    static const int MAX_OBJECTS = 5;   // leaf에 저장할 최대 객체 수
    static const int MAX_DEPTH = 15;    // 최대 트리 깊이

    BVHNode() = default;
    BVHNode(TArray<UPrimitiveComponent*>& Objects, int InDepth);
    ~BVHNode();

    // 재귀적으로 BVH를 구성
#if BVH_SAT
    void Build(TArray<UPrimitiveComponent*>& Objects, int InDepth);
#else
    void Build(const TArray<UPrimitiveComponent*>& Objects, int InDepth);
#endif
    // Query 함수들 (Bounds와 교차하면 하위로 내려감)
    void QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutComponents);
    void QueryFrustumUnique(const FFrustum& Frustum, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs);
    void QueryRay(const FVector& Origin, const FVector& Dir, TArray<UPrimitiveComponent*>& OutComponents);
    void QueryRayUnique(const FVector& Origin, const FVector& Dir, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs);

    // 업데이트/삭제 (간단한 재구축 방식)
    void RemoveComponent(UPrimitiveComponent* Comp);
    void UpdateComponent(UPrimitiveComponent* Comp);
    void QueryRayClosest(const FVector& Origin, const FVector& Dir, UPrimitiveComponent*& ClosestComp, float& MinDistance);
};


class BVHSystem
{
public:
    BVHNode* Root;
    BVHSystem() : Root(nullptr) {}
    // 주어진 컴포넌트 목록으로 BVH를 빌드
    void Build(TArray<UPrimitiveComponent*>& Components)
    {
        if (Root) {
            delete Root;
            Root = nullptr;
        }
        Root = new BVHNode();
        Root->Build(Components, 0); // 깊이 0에서 시작
    }

    // 레이를 사용하여 BVH에서 후보 컴포넌트 검색
    void QueryRay(const FVector& Origin, const FVector& Direction, TArray<UPrimitiveComponent*>& OutComponents)
    {
        if (Root)
            Root->QueryRay(Origin, Direction, OutComponents);
    }

    ~BVHSystem() {
        if (Root) {
            delete Root;
            Root = nullptr;
        }
    }
};