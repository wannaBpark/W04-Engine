#include "Octree.h"  
#include "Engine/Source/Runtime/Engine/Classes/Components/PrimitiveComponent.h"   

void OctreeNode::Insert(UPrimitiveComponent* Comp)  
{  
   // AABB 교차 검사  
   if (!Comp->AABB.Intersects(Bounds)) return;  

   if (!Children[0] && Components.Num() >= MAX_OBJECTS && Depth < MAX_DEPTH)  
   {  
       Subdivide();  
   }  

   if (Children[0]) {  
       for (auto& Child : Children) { Child->Insert(Comp); }  
   } else {  
       Components.Add(Comp);  
   }  
}  

void OctreeNode::Subdivide()  
{
    // 작은 객체가 많을 경우 조기 분할 중지
    float NodeSize = (Bounds.max.x - Bounds.min.x);
    if (NodeSize < 100.0f) return; // 최소 크기 제한

   FVector Center = (Bounds.min + Bounds.max) * 0.5f;  
   FVector HalfExtent = (Bounds.max - Bounds.min) * 0.5f;  

   // 8개 영역 계산  
   FVector ChildMins[8] = {  
       Bounds.min,                            // 좌하단 앞  
       FVector(Center.x, Bounds.min.y, Bounds.min.z), // 우하단 앞  
       FVector(Bounds.min.x, Center.y, Bounds.min.z), // 좌상단 앞  
       FVector(Center.x, Center.y, Bounds.min.z),     // 우상단 앞  
       FVector(Bounds.min.x, Bounds.min.y, Center.z), // 좌하단 뒤  
       FVector(Center.x, Bounds.min.y, Center.z),     // 우하단 뒤  
       FVector(Bounds.min.x, Center.y, Center.z),     // 좌상단 뒤  
       Center                                   // 우상단 뒤  
   };  

   for (register size_t i = 0; i < 8; ++i)  
   {  
       FVector ChildMax = ChildMins[i] + HalfExtent;  
       Children[i] = new OctreeNode(FBoundingBox(ChildMins[i], ChildMax), Depth+1);  
   }  

   // 기존 컴포넌트 재분배  
   for (auto& Comp : Components) {  
       for (auto& Child : Children) {  
           Child->Insert(Comp);  
       }  
   }  
}  

void OctreeNode::QueryRay(const FVector& Origin, const FVector& Dir, TArray<UPrimitiveComponent*>& OutComponents)  
{
    float Distance;
    if (!Bounds.Intersect(Origin, Dir, Distance)) return;

    // 자식 노드가 있으면 재귀적 탐색
    if (Children[0] != nullptr)
    {
        for (auto& Child : Children)
        {
            Child->QueryRay(Origin, Dir, OutComponents);
        }
    }
    else
    {
        // leaf 노드 (더 이상 분할할 자식이 없으면) 자기 자신을 Output 컴포넌트 추가
        for (auto& MyComp : Components) {
            OutComponents.Add(MyComp);
        }
    }
}  

void OctreeSystem::Build(const TArray<UPrimitiveComponent*>& Components)  
{  
   // 전체 컴포넌트 AABB 계산  
   FVector SceneMin(FLT_MAX), SceneMax(-FLT_MAX);  
   for (auto& Comp : Components)  
   {  
       auto& AABB = Comp->AABB;  
       SceneMin = SceneMin.ComponentMin(AABB.min);  
       SceneMin = SceneMin.ComponentMin(AABB.max);  
   }  

   if (Root) delete Root;  
   Root = new OctreeNode(FBoundingBox(SceneMin, SceneMax));  

   for (auto& Comp : Components) {  
       Root->Insert(Comp);  
   }  
}  

void OctreeSystem::AddComponent(UPrimitiveComponent* Comp)  
{  
   if (Root && Root->Bounds.Intersects(Comp->AABB)) //부모가 포함할 수 있으면 삽입  
       Root->Insert(Comp);  
   else  
       Build({ Comp }); // 루트 범위 재설정 (지우고 다시 범위 설정)  
}
