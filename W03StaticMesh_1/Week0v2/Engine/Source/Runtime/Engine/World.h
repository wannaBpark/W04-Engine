#pragma once
#include "Define.h"
#include "Container/Set.h"
#include "UnrealEd/SceneMgr.h"
#include "UObject/ObjectTypes.h"

class AActor;
class UObject;
class UGizmoArrowComponent;
class UCameraComponent;
class UPlayer;
class USceneComponent;
class UTransformGizmo;


class UWorld
{
public:
    UWorld();
    ~UWorld();


    void Initialize();
    void CreateBaseObject();
    void ReleaseBaseObject();
    void RenderBaseObject();
    void Tick(float DeltaTime);
    void Release();
    void Input();

    [[deprecated("Use SpawnActor")]]
    void SpawnObject(OBJECTS _Obj);

    void LoadData(SceneData& _Data);
    SceneData SaveData();
    void	NewScene();
    void	SetPickingObj(UObject* _Obj);
    void	ThrowAwayObj(UObject* _Obj);
    void	CleanUp();
    void	Render();

    /**
     * World에 Actor를 Spawn합니다.
     * @tparam T AActor를 상속받은 클래스
     * @return Spawn된 Actor의 포인터
     */
    template <typename T>
        requires std::derived_from<T, AActor>
    T* SpawnActor();

    /** World에 존재하는 Actor를 제거합니다. */
    bool DestroyActor(AActor* Actor);
    
private:
    const FString defaultMapName = "Default";
    TArray<UObject*> GUObjectArray;
    TArray<UObject*> Trashbin;

    /** World에서 관리되는 모든 Actor의 목록 */
    TSet<AActor*> ActorsArray;

    /** Actor가 Spawn되었고, 아직 BeginPlay가 호출되지 않은 Actor들 */
    TArray<AActor*> PendingBeginPlayActors;

    /** Actor 제거 대기열 */
    TArray<AActor*> PendingDestroyActors;

    USceneComponent* pickingObj = nullptr;
    USceneComponent* pickingGizmo = nullptr;
    UCameraComponent* camera = nullptr;
    UPlayer* localPlayer = nullptr;
public:
    UObject* worldGizmo = nullptr;
    const TArray<UObject*>& GetObjectArr() const { return GUObjectArray; }

    UTransformGizmo* LocalGizmo = nullptr;
    UCameraComponent* GetCamera() const { return camera; }
    UPlayer* GetPlayer() const { return localPlayer; }


    USceneComponent* GetPickingObj() const { return pickingObj; }
    UObject* GetWorldGizmo() const { return worldGizmo; }
    USceneComponent* GetPickingGizmo() const { return pickingGizmo; }
    void	SetPickingGizmo(UObject* _Obj);
};


template <typename T>
    requires std::derived_from<T, AActor>
T* UWorld::SpawnActor()
{
    T* Actor = new T;
    ActorsArray.Add(Actor);
    PendingBeginPlayActors.Add(Actor);
    return Actor;
}
