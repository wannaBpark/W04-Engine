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

    template <typename T>
        requires std::derived_from<T, AActor>
    T* SpawnActor();
    bool DestroyActor(AActor* Actor);
    
private:
    const FString defaultMapName = "Default";
    TArray<UObject*> GUObjectArray;
    TArray<UObject*> Trashbin;

    TSet<AActor*> ActorsArray;

    // Actor가 Spawn되었고, 아직 BeginPlay가 호출되지 않은 Actor들
    TArray<AActor*> PendingBeginPlayActors;

    // Actor 제거 대기열
    TArray<AActor*> PendingDestroyActors;

    USceneComponent* pickingObj = nullptr;
    USceneComponent* pickingGizmo = nullptr;
    UCameraComponent* camera = nullptr;
    UPlayer* localPlayer = nullptr;
public:
    UObject* worldGizmo = nullptr;
    TArray<UObject*>& GetObjectArr() { return GUObjectArray; }

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
