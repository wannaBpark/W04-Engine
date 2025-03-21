#pragma once
#include "Define.h"
#include "UnrealEd/SceneMgr.h"
#include "UObject/ObjectTypes.h"
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
    void Tick(double deltaTime);
    void Release();
    void Input();
    void SpawnObject(OBJECTS _Obj);
    void LoadData(SceneData& _Data);
    SceneData SaveData();
    void	NewScene();
    void	SetPickingObj(UObject* _Obj); 
    void	DeleteObj(UObject* _Obj);
    void	ThrowAwayObj(UObject* _Obj);
    void	CleanUp();
    void	Render();
	
private:
    const FString defaultMapName = "Default";
    TArray<UObject*> GUObjectArray;
    TArray<UObject*> Trashbin;

    USceneComponent* pickingObj = nullptr;
    USceneComponent* pickingGizmo = nullptr;
    UCameraComponent* camera = nullptr;
    UPlayer* localPlayer = nullptr;
public:
    UObject* worldGizmo = nullptr;
    TArray<UObject*>& GetObjectArr() { return GUObjectArray; }

    UTransformGizmo* LocalGizmo = nullptr;
    UCameraComponent* GetCamera() { return camera; }
    UPlayer* GetPlayer() { return localPlayer; }


    USceneComponent* GetPickingObj() { return pickingObj; }
    UObject* GetWorldGizmo() { return worldGizmo; }
    USceneComponent* GetPickingGizmo() { return pickingGizmo; }
    void	SetPickingGizmo(UObject* _Obj);
};

