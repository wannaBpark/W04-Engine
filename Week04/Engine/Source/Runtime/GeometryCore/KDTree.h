#pragma once

#include "Define.h"
#include "Container/Array.h"
#include "Container/Set.h"

class UPrimitiveComponent;

// kd‑tree 노드 클래스
class KDTreeNode
{
public:
    FBoundingBox Bounds;    // 현재 노드가 담당하는 영역 (AABB)
    int Depth;              // 현재 노드의 깊이
    int Axis;               // 분할에 사용된 축 (0: x, 1: y, 2: z)
    
    // 해당 축에 대한 분할 좌표 (보통 Bounds의 중앙값)
    float SplitValue;

    // 이 노드에 저장된 컴포넌트 목록
    TArray<UPrimitiveComponent*> Components;

    // 자식 노드 (kd‑tree는 2개의 자식)
    KDTreeNode* Left;
    KDTreeNode* Right;

    // 노드 분할 시 최대 저장 객체 수 및 최대 깊이 (상수)
    static const int MAX_OBJECTS = 10;
    static const int MAX_DEPTH = 12;

public:
    KDTreeNode(const FBoundingBox& InBounds, int InDepth);
    ~KDTreeNode();

    // 컴포넌트 삽입
    void Insert(UPrimitiveComponent* Comp);
    // 노드 분할
    void Subdivide();

    // Frustum 쿼리 (겹치는 모든 컴포넌트 추가)
    void QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutComponents);

    // Frustum 쿼리 (UUID를 통해 중복 제거)
    void QueryFrustumUnique(const FFrustum& Frustum, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs);

    // Ray 쿼리
    void QueryRay(const FVector& Origin, const FVector& Dir, TArray<UPrimitiveComponent*>& OutComponents);
    // Ray 쿼리 (중복 제거)
    void QueryRayUnique(const FVector& Origin, const FVector& Dir, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs);

    // 컴포넌트 제거 (노드 내에서 모두 제거)
    void RemoveComponent(UPrimitiveComponent* Comp);
    // 컴포넌트 위치/상태 업데이트 (Bounds와 비교하여 노드 내 업데이트)
    void UpdateComponent(UPrimitiveComponent* Comp);
};

class KDTreeSystem
{
public:
    KDTreeNode* Root;

    KDTreeSystem() : Root(nullptr) {}
    ~KDTreeSystem() { if (Root) { delete Root; Root = nullptr; } }

    // Scene의 모든 컴포넌트로부터 전체 AABB를 계산하여 kd‑tree를 구성
    void Build(const TArray<UPrimitiveComponent*>& Components);

    // 단일 컴포넌트 추가 (Bounds 변경 시 전체 재구축 고려)
    void AddComponent(UPrimitiveComponent* Comp);

    // 위치 변경 등으로 인한 업데이트
    void UpdateComponentPosition(UPrimitiveComponent* Comp);
};

