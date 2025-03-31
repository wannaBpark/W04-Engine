# 기술문서 - 1팀

[팀원] : 형시우 , 이서영, 이헌우

### 개요

다양한 종류의 정적 메시(Static Mesh) 렌더링 시스템 최적화를 구현하였다.렌더링 프레임 시간과 피킹 성능에 최적화를 중점적으로 진행하였다.

### 구현 사항

- Performance Counter
: 프레임, 경과 시간(ms) - 템플릿 제공
: 가장 마지막(최근) Picking 소요 시간 - 피킹충돌 검사 전후 기준
: 누적된 시도 횟수
: 누적된 Picking 소요 시간

### Matrix SIMD 적용

> 벤치마킹 결과
> 

```
            --------------------------------------------------------
            Benchmark              Time             CPU   Iterations
            --------------------------------------------------------
            BM_MulDefault       9.49 ns         9.42 ns     74666667
            BM_MulSIMD          2.93 ns         2.92 ns    235789474
            BM_Inverse          66.1 ns         67.0 ns     11200000
            BM_NewInverse       12.5 ns         12.6 ns     56000000
```

- `BM_MulDefault`: SIMD 적용 전 기본 행렬 곱셈
- `BM_MulSIMD`: SIMD 적용 후 행렬 곱셈 [Define.cpp:32]
- `BM_Inverse`: 최적화 전 기본 역행렬 계산
- `BM_NewInverse`: 최적화 적용 후 역행렬 계산 [Define.cpp:78]

> 사용한 주요 SIMD 함수는 다음과 같습니다.
> 
- `_mm_load_ps`: SIMD 레지스터에 4개의 float를 로드합니다.
- `_mm_store_ps`: SIMD 레지스터에 있는 4개의 float를 메모리에 저장합니다.
- `_mm_mul_ps`: SIMD 레지스터에 있는 두 벡터를 곱합니다.
- `_mm_add_ps`: SIMD 레지스터에 있는 두 벡터를 더합니다.
- `_mm_shuffle_ps`: SIMD 레지스터에 있는 두 벡터를 섞습니다. (레지스터를 복사할 때 사용)

### Material Sorting

렌더링되는 StaticMesh->Submesh를 순회하면서, Material을 기준으로 HashMap에 저장합니다.

이후, HashMap을 순회하면서 Material을 기준으로 정렬된 Submesh를 렌더링합니다.

렌더링 시 Material이 바뀔 때마다 DrawCall이 발생하기 때문에, Material을 기준으로 정렬하여 DrawCall을 줄이는 것이 목적입니다.

> [FRenderer::RenderStaticMeshes.cpp:1040]
> 

```cpp
MaterialSubsetRenderData SubsetRenderData;
for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
{
    const UStaticMesh* StaticMesh = StaticMeshComp->GetStaticMesh();
        if (!StaticMesh) continue;
    const OBJ::FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
        if (!RenderData) continue;

    const auto MeshRenderInfo = std::make_shared<FStaticMeshRenderInfo>();
    {
        // MeshRenderInfo 설정 과정...
    }

    // Material에 맞는 Subset을 추가
    const TArray<FStaticMaterial*>& Materials = StaticMesh->GetMaterials();
    const TArray<UMaterial*>& OverrideMaterial = // ...

    for (uint32 Index = 0; const FMaterialSubset& Subset : RenderData->MaterialSubsets)
    {
        TArray<FSubsetRenderInfo>& SubsetArrayRef = SubsetRenderData.FindOrAdd(
                OverrideMaterial[Subset.MaterialIndex]
                            ? OverrideMaterial[Subset.MaterialIndex]
                            : Materials[Subset.MaterialIndex]->Material
        );

        SubsetArrayRef.Emplace(
            MeshRenderInfo,                             // StaticMeshInfo
            Subset.IndexCount,                          // IndexCount
            Subset.IndexStart,                          // StartIndex
            0,                                          // BaseVertexLocation
            Index ==
	            StaticMeshComp->GetSelectedSubMeshIndex() // bIsSubsetSelected
        );
        ++Index;
    }
}
```

> [FRenderer::RenderPrimitive.cpp:216]
> 

```cpp
for (const auto& [Material, RenderDataArray] : SubsetRenderData)
{
    // UpdateMaterial은 Material당 한 번만 호출됨
    UpdateMaterial(Material->GetMaterialInfo());
    std::shared_ptr<FStaticMeshRenderInfo> CurrentMesh = nullptr;
    for (const FSubsetRenderInfo& SubsetInfo : RenderDataArray)
    {
        // SubsetInfo를 이용하여 렌더링...
    }
}
```

### 기하 형상 정렬 개념 - Game Engine Architecture / RTR

애플리케이션 단계에선 3가지를 결정하여 GPU를 구동한다

1. 가시성 결정
2. 기하 형상을 GPU에 제출해 렌더링
3. 셰이더 전달 인자와 렌더 상태를 제어

가시성은 아래와 같은 방법으로 결정할 수 있다.

- 절두체 선별 : 바운딩 볼륨이 절두체 안인지 판단하여 걸러줄 수 있다
    - 평면 식에 점의 좌표를 넣으면 절두체 평면과 구의 중심의 거리를 구할 수 있다
- Occulusion Culling : 물체들끼리 서로 가리는 것을 선별
    - 규모가 큰 환경에선 미리 계산된 잠재적 가시 그룹 (PVS)으로 보일만할 물체를 골라낸다
        - 보통 지정한 지역에 대해 자동으로 PVS를 만든다
- Occulusion Volume
    - 포털(문)의 개념과 비슷함
    - 피라미드 형 입체로 가리는 물체의 경계 모서리를 찾은 후, 카메라 초점부터 각 모서리를 지나는 평면을 만든다
        - 물체가 차폐 볼륨 안에 들어오면 그리지 않아도 된다

---

> 렌더 상태 설정은 전역이다. 렌더 상태를 변경하려면 GPU의 파이프라인을 모두 비우고 새로운 설정을 적용해야 한다. 즉, 렌더 상태를 최소한으로 바꾸는 것이 gain이다.
> 
- Material Sorting : 기하 형상들을 머티리얼로 정렬시켜 최소한으로 렌더 상태를 바꾸는 것
    - 단, 중복 Draw가 증가하므로 렌더 성능이 저하될 수 있다
        - 중복 Draw : 한 픽셀이 겹치는 여러 삼각형이 여러번 채워지는 경우
- Early z-test (대안)
    - PS가 시간이 오래 걸리는데, PS가 실행되기 전 가려지는 단편을 버리는 용도로 만든 것
    - 가장 가까운 삼각형들부터 앞에서 뒤로 z-버퍼를 채움
        - 멀리 떨어진 삼각형들의 단편들은 중복 그리기 없이 버릴 수 있음
- GPU의 z-prepass (대안)
    - 장면을 두번 렌더링 하는 것
    - 1: 빠르고 효율적으로 z-버퍼의 내용을 만듦
        - 2: 색 정보로 프레임 버퍼를 채움
        - GPU에는 픽셀 셰이더를 끄고 z-버퍼의 내용만 업데이트
        - 앞에서 뒤로 그림
        - 그 다음 머티리얼 소팅으로 상태 변화 최소화

---

- 평탄한 실외, 주로 위에서 내려다보는 장면이 많은 게임 - 쿼드트리만 써도 속도 ⬆️
    - 밀폐된 실내 환경 - BSP 트리, 포털 시스템이 유용
- 밀도가 높은 장면 - 가리는 물체가 많기 때문에 차폐 볼륨이 도움이 됨
    - 낮은 장면 - 안티포털이 별 이점이 없음 (오히려 속도 저하)

### Octree

- 옥트리
    - 각 노드(공간)이 8개의 자식 노드를 가지며 3차원 공간을 x,y,z축 기준으로 반씩 나누어 8개의 하위 공간을 생성
        - 루트 노드 = 전체 공간
    - 장점
        - 공간 복잡도를 비교적 간단한 구현으로 3D 데이터를 관리
        - 메모리 사용 효율이 높고, 노드 경계가 명확
    - 단점
        - 하나의 객체가 매우 큰 경계 입체를 가진 옥트리 노드 안에 저장될 수 있다. -> 공간 분할의 효율성 감소
        - 정적 데이터에 적합 (Static Mesh Comp)
- 느슨한 옥트리 (보완책) - [출처 : Game Programming Gems]
    - 노드의 경계 입체 자체를 조정
    - 계통 구조와 노드의 중심은 그대로 두고 입방체의 평면들만 이동
        
        ( = 한 노드의 경계 입방체는 이웃 노드의 경계 입방체와 겹칠 수 있다. )
        
    - 입방체 모서리 길이
        
        $$
        L(\text{depth}) = k \times W \times (2^{\text{depth}})
        $$
        
        - ( k-1 ) * L / 4 보다 작은 객체는 두 분할 평면에 걸리는 일이 절대 없다.
        - 이때 k의 값은 2가 적절
    - 객체를 부착할 노드를 찾는 방법
        
        1) 노드의 깊이를 알아야 한다. (객체의 반지름이 경계입방체 모서리 길이의 1/4보다 작거나 같아야 담을 수 있다.)
        
        $$
        R_{\text{max}}(\text{depth}) = \frac{1}{4} \times L(\text{depth}) = \frac{1}{4} \times \left(2 \times \frac{W}{2^{\text{depth}}}\right) = \frac{1}{2} \times \frac{W}{2^{\text{depth}}}
        $$
        
        2) 노드를 찾은 후 그 노드의 자식 노드들 중 들어갈 수 있는 노드가 있는지 점검
        
    - **단점** : 느슨한 경계로 인해 메모리 사용량이 증가 가능성

[코드]

```cpp
// 일반 옥트리 (Octree)
// 노드의 경계(Box)는 정확히 구역을 나눔
void OctreeNode::Insert(UPrimitiveComponent* Comp) {
    FBoundingBox WorldBBox = GetWorldBoundingBox(Comp);
    if (!WorldBBox.Intersects(Bounds)) return;

    // 자식이 없고, 객체 수 초과 및 최대 깊이 미도달 시 분할
    if (!Children[0] && Components.Num() >= MAX_OBJECTS && Depth < MAX_DEPTH) {
        Subdivide();
    }

    if (Children[0] != nullptr) {
        for (auto& Child : Children) {
            if (Child->Bounds.Intersects(WorldBBox)) {
                Child->Insert(Comp);
            }
        }
    } else {
        Components.Add(Comp);
    }
}

// 느슨한 옥트리 (Loose Octree)
// 노드의 경계(Box)가 확장되어 인접 노드와 겹칠 수 있음 (Loose Factor 적용)
void OctreeNode::Insert(UPrimitiveComponent* Comp) {
    FBoundingBox WorldBBox = GetWorldBoundingBox(Comp);

    // 느슨한 경계(loose bounds)를 사용하여 교차 여부 확인
    if (!WorldBBox.Intersects(GetLooseBounds())) return;

    if (!Children[0] && Components.Num() >= MAX_OBJECTS && Depth < MAX_DEPTH) {
        Subdivide();
    }

    if (Children[0] != nullptr) {
        bool bInsertedIntoChild = false;
        for (auto& Child : Children) {
            if (Child->GetLooseBounds().Contains(WorldBBox)) {
                Child->Insert(Comp);
                bInsertedIntoChild = true;
                break;
            }
        }
        // 자식에 들어가지 못하면 부모 노드에 저장
        if (!bInsertedIntoChild) {
            Components.Add(Comp);
        }
    } else {
        Components.Add(Comp);
    }
}

```

### KD-Tree

> 3D 공간 상을 이진 탐색 하듯이 각 축을 기준으로 분할한 Tree입니다.
> 

아래의 동작을 수행하는 코드를 구현했습니다

1. 삽입 (Insert):
    - 객체의 위치를 기준으로 KD-Tree의 분할 축에 따라 좌/우 자식 노드에 삽입.
    - 최대 깊이에 도달하거나 객체 수가 일정 이하인 경우 현재 노드에 저장.
2. 노드 분할 (Subdivide):
    - 현재 노드의 경계를 최장 축을 기준으로 두 영역으로 나누어 좌/우 자식 노드를 생성.
3. 프러스텀 검사 (QueryFrustum):
    - 주어진 프러스텀과 노드의 경계가 교차할 때, 모든 자식 노드에 대해 재귀적으로 검사를 수행하여 교차하는 객체들을 모음.
4. 광선 감사 (QueryRay):
    - 주어진 광선과 노드의 경계가 교차하는 경우, 자식 노드를 재귀적으로 검사하여 광선과 교차하는 객체들을 반환.
5. 갱신 (UpdateComponent):
    - 객체의 위치가 노드의 경계 안에 없는 경우 삭제하고 다시 삽입.
6. 가장 가까운 교차 검사 (QueryRayClosestBestFirst):
    - Best-First Traversal을 이용하여 광선과 가장 가까운 객체를 탐색.

[코드]

```cpp
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
```

### BVH

> BVH 노드의 광선 충돌 탐색 알고리즘이 가장 가까운(minimum t) 컴포넌트를 반환하도록 구현했습니다
> 
- 최적의 분할 기준을 선택하여 탐색 효율을 높이고, 가시성 판정 시 빠른 가지치기를 수행할 수 있다는 장점이 있습니다
    - 동적으로 움직이는 오브젝트들에 강점을 보입니다

### SAT

> 분할 시 비용함수로 사용한 SAT는 각 분할 후보에서 Left, Right 객체들의 바운딩 박스 표면적을 계산하고, 해당 영역에 포함된 객체 수를 곱한 값으로 비용을 정의합니다.
> 
- 비용이 최소가 되는 축(axis)과 분할 좌표(split value)를 선택하여 좌우로 객체를 분할합니다
    - BVH의 최대한의 “Balanced Tree”조건을 만족하기 위해 쓰인다고 해석했습니다
- 노드의 전체 Bounds 계산
    
    ```cpp
    // 모든 객체의 World 바운딩박스들의 합집합을 구해서 노드의 Bounds를 결정합니다.
    FBoundingBox unionBox = GetWorldBox(Objects[0]);
    for (int i = 1; i < Objects.Num(); ++i)
    {
        unionBox = unionBox.Union(GetWorldBox(Objects[i]));
    }
    Bounds = unionBox;
    ```
    
- SAT 기반 분할 축 및 분할 값 선택
    - 각 축에 대해 객체들의 중심(centroid)을 기준으로 정렬
    - 왼쪽, 오른쪽 배열로 각 분할 지점까지의 누적 AABB를 계산
    - 각 분할 지점에서 왼/오른쪽 부분의 표면적 * 객체 수의 합을 계산하여 비용(overlap)을 평가
        - 비용이 최소인 축, 분할 값을 선택
    - 분할 후 부모 노드의 Bounds는 두 자식 노드(Bounds)의 합집합으로 갱신
        
        ```cpp
        Left = new BVHNode(leftObjects, InDepth + 1);
        Right = new BVHNode(rightObjects, InDepth + 1);
        
        // 부모 노드의 Bounds는 자식 노드들의 Union으로 갱신
        Bounds = Left->Bounds.Union(Right->Bounds);
        ```
        
    
    [ 코드 ]
    
    ```cpp
    int bestAxis = 0;
    float bestOverlap = FLT_MAX;
    float bestSplit = 0.0f;
    
    for (int axis = 0; axis < 3; ++axis)
    {
        // 1. 객체들을 해당 축의 중심(centroid) 기준으로 정렬
        Objects.Sort([axis](UPrimitiveComponent* A, UPrimitiveComponent* B) {
            return GetCentroid(A)[axis] < GetCentroid(B)[axis];
        });
    
        // 2. 왼쪽 누적 바운딩 박스 계산
        TArray<FBoundingBox> leftBoxes, rightBoxes;
        leftBoxes.SetNum(Objects.Num());
        rightBoxes.SetNum(Objects.Num());
    
        FBoundingBox leftBox = GetWorldBox(Objects[0]);
        leftBoxes[0] = leftBox;
        for (int i = 1; i < Objects.Num(); ++i)
        {
            leftBox = leftBox.Union(GetWorldBox(Objects[i]));
            leftBoxes[i] = leftBox;
        }
    
        // 3. 오른쪽 누적 바운딩 박스 계산
        FBoundingBox rightBox = GetWorldBox(Objects[Objects.Num() - 1]);
        rightBoxes[Objects.Num() - 1] = rightBox;
        for (int i = Objects.Num() - 2; i >= 0; --i)
        {
            rightBox = rightBox.Union(GetWorldBox(Objects[i]));
            rightBoxes[i] = rightBox;
        }
    
        // 4. 비용 함수 계산: 왼쪽과 오른쪽의 표면적에 객체 수를 곱한 값의 합
        for (int i = 0; i < Objects.Num() - 1; ++i)
        {
            int leftCount = i + 1;
            int rightCount = Objects.Num() - leftCount;
            float overlap = leftBoxes[i].SurfaceArea() * leftCount + rightBoxes[i + 1].SurfaceArea() * rightCount;
            if (overlap < bestOverlap)
            {
                bestOverlap = overlap;
                bestAxis = axis;
                bestSplit = GetCentroid(Objects[i])[axis];
            }
        }
    }
    
    ```
    

### 단계

> **P(n):** "높이가 n 이하인 BVH 트리에서, QueryRayClosestBestFirst 함수는 모든 노드(내부 노드와 리프 노드)를 탐색하여, 광선과의 교차 거리가 최소인 컴포넌트를 반환
> 
- Base Case: n=1(높이가 1인 BVH, 리프노드의 경우 P(1)이 성립)
    - Leaf 노드의 경우, 루트의 AABB와 광선이 교차되면 min-heap에 삽입
    - Root 노드의 AABB와 광선의 교차 거리를 계산하여, min-heap에 루트 삽입
- 가정
    - 임의의 BVH트리 T가 n개의 노드로 구성되어 있고, 모든 Leaf노드에서 광선과 가장 작은 교차거리를 갖는 컴포넌트를 반환한다 가정
- 과정
    - 현재 노드의 진입 t값 (바운딩 박스)과 “현재까지 발견된 최단 t값 (bestT)”를 비교
    - 항상 성립 : 바운딩 박스의 진입 t값 ≤ 해당 박스 안 컴포넌트의 t값
    - 그러므로 진입 t값이 최단 t값보다 작다면 무시할 수 있습니다
    - 높이가 k+1인 BVH 트리에 대해 min-heap은 언제나 현재까지의 최소 교차 거리를 저장하고 있음
        - 해당 교차거리보다 작은 교차거리를 가지는 바운딩 볼륨 노드에 대한 탐색을 pruning 가능
    - worst case:
        - 바운딩 박스의 모서리에 위치한 하나의 노드의 영향으로 해당 바운딩 볼륨(노드) 내의 모든 컴포넌트와의 광선 교차 검사를 수행하는 경우
    
    [ 코드 ]
    
    ```cpp
    UPrimitiveComponent* BVHNode::QueryRayClosestBestFirst(const FVector& Origin, const FVector& Dir)
    {
        FVector normDir = Dir.GetSafeNormal();
        
        std::priority_queue<PQEntry, std::vector<PQEntry>, PQEntryComparator> pq;
        float bestT = FLT_MAX;
        UPrimitiveComponent* bestComp = nullptr;
    
        float tEntry;
        if (Bounds.Intersect(Origin, normDir, tEntry)) {
            pq.push({ this, tEntry });
        }
    
        while (!pq.empty())
        {
            PQEntry current = pq.top();
            pq.pop();
    
            // 만약 현재 노드의 진입 거리가 이미 최단 교차보다 멀다면 더 이상 탐색할 필요가 없음.
            if (current.tEntry >= bestT)
                continue;
    
            BVHNode* node = current.Node;
    
            // Leaf 노드인 경우, 이 노드에 있는 모든 컴포넌트에 대해 Ray-Sphere 교차 검사 수행
            if (!node->Left && !node->Right)
            {
                for (UPrimitiveComponent*& comp : node->Components)
                {
                    float tInter;
                    FBoundingBox box = GetWorldBox(comp);
                    // AABB를 구로 근사 - 중심과 반지름 계산
                    FVector center = (box.min + box.max) * 0.5f;
                    float radius = (box.max - box.min).Length() * 0.5f;
    
                    // 비트마스킹 빠른 충돌 검사
                    if (FastIsRayIntersectingSphere(Origin, normDir, center, radius, tInter))
                    {
                        if (tInter < bestT)
                        {
                            bestT = tInter;
                            bestComp = comp;
                        }
                    }
                }
            }
            else // 내부 노드인 경우, 자식 노드를 검사
            {
                if (node->Left)
                {
                    float tLeft;
                    if (node->Left->Bounds.Intersect(Origin, normDir, tLeft) && tLeft < bestT)
                    {
                        pq.push({ node->Left, tLeft });
                    }
                }
                if (node->Right)
                {
                    float tRight;
                    if (node->Right->Bounds.Intersect(Origin, normDir, tRight) && tRight < bestT)
                    {
                        pq.push({ node->Right, tRight });
                    }
                }
            }
        }    
        return bestComp ? bestComp : nullptr;
    }
    ```
    

### **최상 우선 탐색 - BVH와 KD-Tree의 차이**

- 실외, sparse하게 물체가 퍼져있다는 가정을 합니다
    - KD Tree는 경계 볼륨이 사각형 형태로 나뉘므로 광선 방향과 무관하게 더 많은 경계를 탐색할 가능성이 있습니다.
    - 공간 분할이 직교 축(X,Y,Z)를 기준으로 이뤄지기 때문에, 불필요한 경계 교차 검사가 늘어날 수 있습니다
- 반면 BVH의 최상 우선 탐색은
    - 경계 볼륨을 중심으로 실제 객체의 경계를 바운딩하기 때문에 불필요한 공간 탐색이 줄어드는 장점이 있습니다
    - 객체 중심에 맞춰진 경계가 비대칭일 때도 효율적입니다
- 두 Scene Graph가 이점을 보이는 분야
    - **KD-Tree**: 균일한 공간 분포 (예: 포인트 클라우드).
    - **BVH**: 비균형 구조의 복잡한 모델 (예: 3D 모델의 충돌 검사, 광선 추적).

### 기타 최적화

- SIMD를 적용시킨 광선 충돌 검사의 효과는 유의미하진 않았습니다
- 조건문, 비트마스킹을 적용하자 약간의 시간이 단축되었습니다 (아주 약간…0.001ms가량)