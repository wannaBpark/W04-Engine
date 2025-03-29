#include "Octree.h"  
#include "Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"   
#include "Editor/UnrealEd/PrimitiveBatch.h"
#include "Runtime/Core/Math/JungleMath.h"

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
    // float NodeSize = (Bounds.max.x - Bounds.min.x);
    // if (NodeSize < 100.0f) return; // 최소 크기 제한

    FVector Center = (Bounds.min + Bounds.max) * 0.5f;
    FVector HalfExtent = (Bounds.max - Bounds.min) * 0.5f;

    // 8개 영역 계산
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

    for (size_t i { 0 }; i < 8; ++i)
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

    //UE_LOG(LogLevel::Display, "Bounds Min : %.1f, %.1f, %.1f", Bounds.min.x, Bounds.min.y, Bounds.min.z);
    //UE_LOG(LogLevel::Display, "Bounds Max : %.1f, %.1f, %.1f", Bounds.max.x, Bounds.max.y, Bounds.max.z);

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
    //// 자식 노드가 있는 경우, 해당 자식 노드로 전달
    //if (Children[0]) {
    //    for (auto& Child : Children) {
    //        Child->RemoveComponent(Comp);
    //    }
    //}
    //else {
    //    // 리프 노드에서 컴포넌트를 제거
    //    int32 Index = Components.Find(Comp);
    //    if (Index != INDEX_NONE) {
    //        Components.RemoveAt(Index);
    //    }
    //}
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
       //SceneMin = SceneMin.ComponentMin(Root->Bounds.min);
       //SceneMax = SceneMax.ComponentMax(Root->Bounds.max);
       delete Root;
       //Root->Bounds = { SceneMin, SceneMax };
   }
   //else {
       Root = new OctreeNode(FBoundingBox(SceneMin, SceneMax));
   //}
   // 현재 Root의 월드 박스와 비교하여 다시 설정
   //UE_LOG(LogLevel::Display, "Min Bounding Box : %.2f %.2f %.2f", SceneMin.x, SceneMin.y, SceneMin.z);
   //UE_LOG(LogLevel::Display, "Max Bounding Box : %.2f %.2f %.2f", SceneMax.x, SceneMax.y, SceneMax.z);

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