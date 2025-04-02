#pragma once
#include "Define.h"
#include "Container/Set.h"
#include "UObject/ObjectFactory.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Level.h"

class FObjectFactory;
class AActor;
class UObject;
class UGizmoArrowComponent;
class UCameraComponent;
class AEditorPlayer;
class USceneComponent;
class ATransformGizmo;
class OctreeSystem;
class KDTreeSystem;
class BVHSystem;
class UPrimitiveComponent;

class UWorld : public UObject
{
    DECLARE_CLASS(UWorld, UObject)

public:
    UWorld() = default;

    void Initialize();
    void CreateBaseObject();
    void ReleaseBaseObject();
    void Tick(float DeltaTime);
    void Release();

    /**
     * World에 Actor를 Spawn합니다.
     * @tparam T AActor를 상속받은 클래스
     * @return Spawn된 Actor의 포인터
     */
    template <typename T>
        requires std::derived_from<T, AActor>
    T* SpawnActor();

    /** World에 존재하는 Actor를 제거합니다. */
    bool DestroyActor(AActor* ThisActor);

private:
    /* 현재 활성화된 레벨 : 1개로 가정 */
    ULevel* PersistentLevel;

    /* 월드의 타입*/
    EWorldType WorldType = EWorldType::Editor;

    const FString defaultMapName = "Default";

    /** World에서 관리되는 모든 Actor의 목록 */
    TSet<AActor*> ActorsArray;

    /** Actor가 Spawn되었고, 아직 BeginPlay가 호출되지 않은 Actor들 */
    TArray<AActor*> PendingBeginPlayActors;

    AActor* SelectedActor = nullptr;

    AEditorPlayer* EditorPlayer = nullptr;
    OctreeSystem* Octree = nullptr;
    KDTreeSystem* KDTree = nullptr;
    BVHSystem* BVH = nullptr;
public:
    UObject* worldGizmo = nullptr;

    void SetPersistentLevel(ULevel* InLevel) { PersistentLevel = InLevel; }
    ULevel* GetPersistentLevel() const { return PersistentLevel; }

    void SetWorldType(EWorldType InWorldType) { WorldType = InWorldType; }
    const EWorldType& GetWorldType() const { return WorldType; }

    const TSet<AActor*>& GetActors() const { return ActorsArray; }

    ATransformGizmo* LocalGizmo = nullptr;
    AEditorPlayer* GetEditorPlayer() const { return EditorPlayer; }

    // EditorManager 같은데로 보내기
    AActor* GetSelectedActor() const { return SelectedActor; }
    void SetPickedActor(AActor* InActor)
    {
        SelectedActor = InActor;
    }

    OctreeSystem* GetOctreeSystem() const { return Octree; }
    void SetOctreeSystem(const TArray<UPrimitiveComponent*>& Components);
    void SetOctreeSystem(OctreeSystem* InOctree) { Octree = InOctree; }

    KDTreeSystem* GetKDTreeSystem() const { return KDTree; }
    void SetKDTreeSystem(const TArray<UPrimitiveComponent*>& Components);
    void SetKDTreeSystem(KDTreeSystem* InKDTree) { KDTree = InKDTree; }

    BVHSystem* GetBVHSystem() const { return BVH; }
    void SetBVHSystem(TArray<UPrimitiveComponent*>& Components);
    void SetBVHSystem(BVHSystem* InBVH) { BVH = InBVH; }
};


template <typename T>
    requires std::derived_from<T, AActor>
T* UWorld::SpawnActor()
{
    T* Actor = FObjectFactory::ConstructObject<T>();
    // TODO: 일단 AddComponent에서 Component마다 초기화
    // 추후에 RegisterComponent() 만들어지면 주석 해제
    // Actor->InitializeComponents();
    ULevel* TargetLevel = GetPersistentLevel();
    Actor->SetLevel(TargetLevel);

    TargetLevel->ActorsArray.Add(Actor);
    TargetLevel->PendingBeginPlayActors.Add(Actor);
    
    return Actor;
}


