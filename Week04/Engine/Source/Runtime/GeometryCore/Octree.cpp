#include "Octree.h"  
#include "Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"   
#include "Editor/UnrealEd/PrimitiveBatch.h"
#include "Runtime/Core/Math/JungleMath.h"

// 거리 계산 함수 (카메라 위치와 노드 중심점 거리)
inline float CalculateDistance(const FVector& a, const FVector& b) {
    return sqrt((a.X - b.X) * (a.X - b.X) +
        (a.Y - b.Y) * (a.Y - b.Y) +
        (a.Z - b.Z) * (a.Z - b.Z));
}

#pragma region Query Frustum Intersection
void OctreeNode::QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutComponents)
{
    // 현재 노드의 AABB가 프러스텀과 교차하지 않으면 가지치기
    if (!Frustum.Intersects(Bounds))
        return;

    // 자식 노드가 있다면 재귀적으로 호출
    if (Children[0] != nullptr)
    {
        for (auto& Child : Children)
        {
            Child->QueryFrustum(Frustum, OutComponents);
        }
    }
    else
    {
        // 리프 노드이면 해당 컴포넌트들을 결과에 추가
        for (auto& Comp : Components)
        {
            OutComponents.Add(Comp);
        }
    }
}

void OctreeNode::QueryFrustumUnique(const FFrustum& Frustum, TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
{
    // 현재 노드의 AABB가 프러스텀과 교차하지 않으면 가지치기
    if (!Frustum.Intersects(Bounds)) { return; }

    // 자식 노드가 있으면 재귀적으로 호출
    if (Children[0] != nullptr)
    {
        for (auto& Child : Children)
        {
            Child->QueryFrustumUnique(Frustum, OutComponents, UniqueUUIDs);
        }
    }
    else
    {
        // 리프 노드이면 각 컴포넌트의 UUID를 검사하여 중복 없이 추가
        for (auto& Comp : Components)
        {
            uint32 UUID = Comp->GetUUID(); // UPrimitiveComponent의 고유 식별자
            if (!UniqueUUIDs.Contains(UUID))
            {
                UniqueUUIDs.Add(UUID);
                OutComponents.Add(Comp);
            }
        }
    }
}
#pragma endregion
#if _LOOSEOCTREE
void OctreeNode::Insert(UPrimitiveComponent* Comp)
{
    // 객체의 월드 AABB 계산
    FMatrix Model = JungleMath::CreateModelMatrix(
        Comp->GetWorldLocation(),
        Comp->GetWorldRotation(),
        Comp->GetWorldScale()
    );
    FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
        Comp->AABB, Comp->GetWorldLocation(), Model
    );

    // 현재 노드의 loose bounds와 교차하지 않으면 이 노드에는 삽입하지 않음
    if (!WorldBBox.Intersects(GetLooseBounds()))
        return;

    // Leaf 노드이고, 저장된 객체 수가 MAX_OBJECTS를 초과하며 아직 MAX_DEPTH에 도달하지 않았다면 분할
    if (!Children[0] && Components.Num() >= MAX_OBJECTS && Depth < MAX_DEPTH)
    {
        Subdivide();
    }

    // 자식 노드가 존재하면, 각 자식의 loose bounds 내에 완전히 포함되는지 검사하여 자식에 삽입 시도
    if (Children[0] != nullptr)
    {
        bool bInsertedIntoChild = false;
        for (auto& Child : Children)
        {
            if (Child->GetLooseBounds().Contains(WorldBBox))
            {
                Child->Insert(Comp);
                bInsertedIntoChild = true;
                break;
            }
        }
        // 어느 자식에도 완전히 포함되지 않으면 현재 노드에 보관
        if (!bInsertedIntoChild)
        {
            Components.Add(Comp);
        }
    }
    else
    {
        // 자식 노드가 없는 경우 현재 노드에 추가
        Components.Add(Comp);
    }
}

// 노드를 8분할하여 자식 노드 생성. 분할은 부모의 strict bounds를 기준으로 합니다.
void OctreeNode::Subdivide()
{
    FVector Center = (Bounds.min + Bounds.max) * 0.5f;
    FVector HalfExtent = (Bounds.max - Bounds.min) * 0.5f;

    // 8개 자식의 최소 좌표 계산
    FVector ChildMins[8] = {
        Bounds.min,                                    // 좌하단 앞
        FVector(Center.x, Bounds.min.y, Bounds.min.z), // 우하단 앞
        FVector(Bounds.min.x, Center.y, Bounds.min.z), // 좌상단 앞
        FVector(Center.x, Center.y, Bounds.min.z),     // 우상단 앞
        FVector(Bounds.min.x, Bounds.min.y, Center.z), // 좌하단 뒤
        FVector(Center.x, Bounds.min.y, Center.z),     // 우하단 뒤
        FVector(Bounds.min.x, Center.y, Center.z),     // 좌상단 뒤
        Center                                         // 우상단 뒤
    };

    // 자식 노드의 strict bounds는 부모의 strict bounds를 8분할하여 결정
    for (int i = 0; i < 8; ++i)
    {
        FVector ChildMax = ChildMins[i] + HalfExtent;
        Children[i] = new OctreeNode(FBoundingBox(ChildMins[i], ChildMax), Depth + 1);
    }

    // 기존에 이 노드에 저장되어 있던 객체들을 재분배
    TArray<UPrimitiveComponent*> OldComponents;
    std::swap(OldComponents, Components);

    for (auto& Comp : OldComponents)
    {
        bool bInserted = false;
        FMatrix Model = JungleMath::CreateModelMatrix(
            Comp->GetWorldLocation(),
            Comp->GetWorldRotation(),
            Comp->GetWorldScale()
        );
        FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
            Comp->AABB, Comp->GetWorldLocation(), Model
        );
        // 각 자식의 loose bounds 내에 완전히 포함되면 해당 자식으로 삽입
        for (auto& Child : Children)
        {
            if (Child->GetLooseBounds().Contains(WorldBBox))
            {
                Child->Insert(Comp);
                bInserted = true;
                break;
            }
        }
        // 자식 어느 곳에도 완전히 포함되지 않으면 부모(현재 노드)에 보관
        if (!bInserted)
        {
            Components.Add(Comp);
        }
    }
}

// Ray 쿼리: 현재 노드의 loose bounds를 기준으로 판정하고,
// 부모에 보관된 객체와 자식 노드의 객체 모두를 결과에 추가합니다.
void OctreeNode::QueryRay(const FVector& Origin, const FVector& Dir, TArray<UPrimitiveComponent*>& OutComponents)
{
    float Distance;
    if (!GetLooseBounds().Intersect(Origin, Dir, Distance))
        return;

    // 현재 노드에 저장된 객체 추가
    for (auto& Comp : Components)
    {
        OutComponents.Add(Comp);
    }

    // 자식 노드가 있으면 재귀적으로 탐색
    if (Children[0] != nullptr)
    {
        for (auto& Child : Children)
        {
            Child->QueryRay(Origin, Dir, OutComponents);
        }
    }
}

// 중복 없이 Ray 쿼리: Unique UUID를 사용하여 중복 제거하면서 결과를 추가합니다.
void OctreeNode::QueryRayUnique(const FVector& Origin, const FVector& Dir,
    TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
{
    float Distance;
    if (!GetLooseBounds().Intersect(Origin, Dir, Distance))
        return;

    // 현재 노드의 객체 추가 (중복 검사)
    for (auto& Comp : Components)
    {
        uint32 UUID = Comp->GetUUID();
        if (!UniqueUUIDs.Contains(UUID))
        {
            UniqueUUIDs.Add(UUID);
            OutComponents.Add(Comp);
        }
    }

    // 자식 노드가 있으면 재귀적으로 탐색
    if (Children[0] != nullptr)
    {
        for (auto& Child : Children)
        {
            Child->QueryRayUnique(Origin, Dir, OutComponents, UniqueUUIDs);
        }
    }
}

#else 
void OctreeNode::Insert(UPrimitiveComponent* Comp)
{
    // AABB 교차 검사
    FMatrix Model = JungleMath::CreateModelMatrix(
        Comp->GetWorldLocation(),
        Comp->GetWorldRotation(),
        Comp->GetWorldScale()
    );
    FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
        Comp->AABB, Comp->GetWorldLocation(), Model
    );
    if (!WorldBBox.Intersects(Bounds)) return;

    // 자식이 없거나, 들고 있는 최대 수보다 많고 아직 depth가 넉넉할 때
    if (!Children[0] && Components.Num() >= MAX_OBJECTS && Depth < MAX_DEPTH)
    {
        Subdivide();
    }

    if (Children[0] != nullptr) {
        // 여러 자식과 교차할 수 있으므로, 모든 해당 자식에 삽입  
        for (auto& Child : Children) {
            if (Child->Bounds.Intersects(WorldBBox))
                Child->Insert(Comp);
        }
    }
    else {
        Components.Add(Comp);
    }
}

void OctreeNode::Subdivide()
{
    // 작은 객체가 많을 경우 조기 분할 중지
    // float NodeSize = (Bounds.max.X - Bounds.min.X);
    // if (NodeSize < 100.0f) return; // 최소 크기 제한

    FVector Center = (Bounds.min + Bounds.max) * 0.5f;
    FVector HalfExtent = (Bounds.max - Bounds.min) * 0.5f;

    // 8개 영역 계산
    FVector ChildMins[8] = {
        Bounds.min,                                    // 좌하단 앞
        FVector(Center.X, Bounds.min.Y, Bounds.min.Z), // 우하단 앞
        FVector(Bounds.min.X, Center.Y, Bounds.min.Z), // 좌상단 앞
        FVector(Center.X, Center.Y, Bounds.min.Z),     // 우상단 앞
        FVector(Bounds.min.X, Bounds.min.Y, Center.Z), // 좌하단 뒤
        FVector(Center.X, Bounds.min.Y, Center.Z),     // 우하단 뒤
        FVector(Bounds.min.X, Center.Y, Center.Z),     // 좌상단 뒤
        Center                                         // 우상단 뒤
    };

    for (size_t i{ 0 }; i < 8; ++i)
    {
        FVector ChildMax = ChildMins[i] + HalfExtent;
        Children[i] = new OctreeNode(FBoundingBox(ChildMins[i], ChildMax), Depth + 1);
    }

    // 기존 컴포넌트 재분배 후, 부모 배열 초기화
    TArray<UPrimitiveComponent*> OldComponents;
    std::swap(OldComponents, Components);
    // 컴포넌트가 해당 자식 노드와 교차하면 삽입
    for (auto& Comp : OldComponents)
    {
        for (const auto& Child : Children)
        {
            FMatrix Model = JungleMath::CreateModelMatrix(
                Comp->GetWorldLocation(),
                Comp->GetWorldRotation(),
                Comp->GetWorldScale()
            );
            FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
                Comp->AABB, Comp->GetWorldLocation(), Model
            );
            if (Child->Bounds.Intersects(WorldBBox))
            {
                Child->Insert(Comp);
            }
        }
    }
}
void OctreeNode::QueryRay(const FVector& Origin, const FVector& Dir, TArray<UPrimitiveComponent*>& OutComponents)
{
    float Distance;
    if (!Bounds.Intersect(Origin, Dir, Distance)) return;

    //UE_LOG(LogLevel::Display, "Bounds Min : %.1f, %.1f, %.1f", Bounds.min.X, Bounds.min.Y, Bounds.min.Z);
    //UE_LOG(LogLevel::Display, "Bounds Max : %.1f, %.1f, %.1f", Bounds.max.X, Bounds.max.Y, Bounds.max.Z);

    // 자식 노드가 있으면 재귀적 탐색
    if (Children[0] != nullptr) {
        for (auto& Child : Children) {
            Child->QueryRay(Origin, Dir, OutComponents);
            //UE_LOG(LogLevel::Display, "Find More Child");
        }
    }
    else {
        // leaf 노드 (더 이상 분할할 자식이 없으면) 자기 자신을 Output 컴포넌트 추가
        for (auto& MyComp : Components) {
            int idx = 0;
            OutComponents.Add(MyComp);
            //UE_LOG(LogLevel::Display, "Leaf Node No.%d", ++idx);
        }
    }
}
void OctreeNode::QueryRayUnique(const FVector& Origin, const FVector& Dir, 
    TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
{
    float Distance;
    if (!Bounds.Intersect(Origin, Dir, Distance)) return;

    // 자식 노드가 있으면 재귀적 탐색
    if (Children[0] != nullptr) {
        for (auto& Child : Children) {
            Child->QueryRayUnique(Origin, Dir, OutComponents, UniqueUUIDs);
        }
    }
    else {
        // leaf 노드 (더 이상 분할할 자식이 없으면) 자기 자신을 Output 컴포넌트 추가
        for (auto& MyComp : Components) {
            uint32 UUID = MyComp->GetUUID(); // UPrimitiveComponent의 고유 식별자
            if (!UniqueUUIDs.Contains(UUID))
            {
                UniqueUUIDs.Add(UUID);
                OutComponents.Add(MyComp);
            }
        }
    }
}
#endif


void OctreeNode::UpdateComponent(UPrimitiveComponent* Comp)
{
    // 컴포넌트가 현재 노드의 Bounds에 포함되지 않으면 삭제
    FMatrix Model = JungleMath::CreateModelMatrix(
        Comp->GetWorldLocation(),
        Comp->GetWorldRotation(),
        Comp->GetWorldScale()
    );
    FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
        Comp->AABB, Comp->GetWorldLocation(), Model
    );
    if (!WorldBBox.Intersects(Bounds)) {
        RemoveComponent(Comp);
        return;
    }

    // 자식 노드가 있는 경우, 해당 자식 노드로 전달
    if (Children[0]) {
        for (auto& Child : Children) {
            if (Child->Bounds.Intersects(WorldBBox)) {
                Child->UpdateComponent(Comp);
                return;
            }
        }
    }

    // 리프 노드인 경우, 컴포넌트를 업데이트
    int32 Index = Components.Find(Comp);
    if (Index != INDEX_NONE) {
        Components[Index] = Comp; // 컴포넌트 업데이트
    }
}

void OctreeNode::RemoveComponent(UPrimitiveComponent* Comp)
{
    // 현재 노드의 Components 배열에서 모든 항목 제거
    while (Components.Contains(Comp))
    {
        int32 Index = Components.Find(Comp);
        Components.RemoveAt(Index);
    }
    // 자식 노드가 있다면 재귀적으로 모두 제거
    if (Children[0]) {
        for (auto& Child : Children) {
            Child->RemoveComponent(Comp);
        }
    }
}

void OctreeSystem::Build(const TArray<UPrimitiveComponent*>& Components)  
{  
    // 기존 트리의 모든 컴포넌트 수집
    TArray<UPrimitiveComponent*> AllComponents;
    if (Root) {
        Root->CollectComponents(AllComponents);
    }
    for (auto& NewComp : Components) AllComponents.Add(NewComp);
   // 입력된 새로운 컴포넌트들 AABB 계산  
   FVector SceneMin(FLT_MAX), SceneMax(-FLT_MAX);  
   for (auto& Comp : AllComponents)
   {  
       FMatrix Model = JungleMath::CreateModelMatrix(
           Comp->GetWorldLocation(),
           Comp->GetWorldRotation(),
           Comp->GetWorldScale()
       );
       FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
           Comp->AABB, Comp->GetWorldLocation(), Model
       );
       //auto& AABB = Comp->AABB;  
       SceneMin = SceneMin.ComponentMin(WorldBBox.min);
       SceneMax = SceneMax.ComponentMax(WorldBBox.max);
   } 
   if (Root) {
       delete Root;
   }
   Root = new OctreeNode(FBoundingBox(SceneMin, SceneMax));

   // 현재 Root의 월드 박스와 비교하여 다시 설정
   //UE_LOG(LogLevel::Display, "Min Bounding Box : %.2f %.2f %.2f", SceneMin.X, SceneMin.Y, SceneMin.Z);
   //UE_LOG(LogLevel::Display, "Max Bounding Box : %.2f %.2f %.2f", SceneMax.X, SceneMax.Y, SceneMax.Z);

   for (auto& Comp : AllComponents) {
       Root->Insert(Comp);  
   }  
}  

void OctreeSystem::AddComponent(UPrimitiveComponent* Comp)  
{  
    FMatrix Model = JungleMath::CreateModelMatrix( Comp->GetWorldLocation(), Comp->GetWorldRotation(), Comp->GetWorldScale() );
    FBoundingBox WorldBBox = UPrimitiveBatch::GetWorldBoundingBox(
        Comp->AABB, Comp->GetWorldLocation(), Model
    );

    if (Root && Root->Bounds.Contains(WorldBBox)) //부모가 감쌀 수 있으면 삽입  
       Root->Insert(Comp);  
    else
       Build({ Comp }); // 루트 범위 재설정 (지우고 다시 범위 설정)  
}


// 위치 변화가 있는 컴포넌트를 옥트리에서 제거 or 삽입
void OctreeSystem::UpdateComponentPosition(UPrimitiveComponent* Comp)
{
    // 컴포넌트를 옥트리에서 제거
    if (Root) {
        Root->RemoveComponent(Comp);
        //UE_LOG(LogLevel::Display, "Remove Component");
    }

    // 컴포넌트를 다시 삽입
    AddComponent(Comp);
}




TArray<FVector> GetBoxVertices(const FBoundingBox& Box)
{
    TArray<FVector> Vertices;
    Vertices.Add(FVector(Box.min.X, Box.min.Y, Box.min.Z));
    Vertices.Add(FVector(Box.max.X, Box.min.Y, Box.min.Z));
    Vertices.Add(FVector(Box.min.X, Box.max.Y, Box.min.Z));
    Vertices.Add(FVector(Box.max.X, Box.max.Y, Box.min.Z));
    Vertices.Add(FVector(Box.min.X, Box.min.Y, Box.max.Z));
    Vertices.Add(FVector(Box.max.X, Box.min.Y, Box.max.Z));
    Vertices.Add(FVector(Box.min.X, Box.max.Y, Box.max.Z));
    Vertices.Add(FVector(Box.max.X, Box.max.Y, Box.max.Z));
    return Vertices;
}

// Octree 노드에 새로운 함수 추가: 거리 및 occlusion 기반 컬링 (자식 노드들에 대해 8개 정점 광선 검사)
// 이 함수는 카메라 위치와 시점(카메라 위치로부터 각 정점까지의 광선)이 주어졌을 때,
// 해당 노드의 자식들 중 완전히 가려진 노드는 순회하지 않고, 그렇지 않은 경우에만 재귀적으로 탐색하여
// 중복 없이(Unique UUID) 가시한 UPrimitiveComponent들을 OutComponents에 추가합니다.
void OctreeNode::QueryFrustumOcclusionCulling(const FFrustum& Frustum, const FVector& CameraPos,
    TSet<UPrimitiveComponent*>& OutComponents, TSet<uint32>& UniqueUUIDs)
{
    // 1. 프러스텀 컬링: 현재 노드의 AABB가 프러스텀과 교차하지 않으면 조기 리턴
    if (!Frustum.Intersects(Bounds))
        return;

    // 2. 내부 노드인지 Leaf 노드인지 판정
    if (Children[0] == nullptr)
    {
        // Leaf 노드: 해당 노드의 모든 컴포넌트를 중복 없이 추가
        for (auto& Comp : Components)
        {
            uint32 UUID = Comp->GetUUID();
            if (!UniqueUUIDs.Contains(UUID))
            {
                UniqueUUIDs.Add(UUID);
                OutComponents.Add(Comp);
            }
        }
        return;
    }

    // 3. 내부 노드의 경우, 각 자식 노드에 대해 occlusion 검사 수행
    //    각 자식 노드에 대해, 자식의 AABB를 이루는 8개 코너 정점에 대해 검사합니다.
    for (int i = 0; i < 8; ++i)
    {
        if (Children[i] == nullptr)
            continue;

        TArray<FVector> childVertices = GetBoxVertices(Children[i]->Bounds);
        bool childFullyOccluded = true;

        // 각 정점마다 검사: 하나라도 광선이 형제 노드들(자신을 제외한 다른 자식)의 AABB와 교차하지 않으면, 해당 자식은 가려지지 않은 것으로 판단
        for (const FVector& vertex : childVertices)
        {
            // 카메라에서 정점까지의 광선 방향과 거리 계산
            FVector rayDir = (vertex - CameraPos).GetSafeNormal();
            float dVertex = (vertex - CameraPos).Length();
            bool vertexOccluded = false;

            // 현재 자식(i) 외의 모든 자식(j)에 대해 검사
            for (int j = 0; j < 8; ++j)
            {
                if (j == i || Children[j] == nullptr)
                    continue;

                float tIntersect;
                // 만약 카메라에서 정점으로 향하는 ray가 자식 j의 AABB와 교차하고 그 교차 거리가 정점까지의 거리보다 짧으면
                if (Children[j]->Bounds.Intersect(CameraPos, rayDir, tIntersect) && tIntersect < dVertex)
                {
                    vertexOccluded = true;
                    break;
                }
            }

            // 하나라도 occluded되지 않은 정점이 있다면, 해당 자식은 완전히 가려지지 않음
            if (!vertexOccluded)
            {
                childFullyOccluded = false;
                break;
            }
        }

        // 4. 자식 노드가 완전히 가려졌다면, 해당 자식은 무시 (Early-out)
        if (childFullyOccluded)
        {
            continue;
        }
        else
        {
            // 재귀 호출: 해당 자식 노드를 순회
            Children[i]->QueryFrustumOcclusionCulling(Frustum, CameraPos, OutComponents, UniqueUUIDs);
        }
    }
}