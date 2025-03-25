#pragma once
#include "Define.h"
#include "Container/Set.h"
#include "UObject/ObjectFactory.h"
#include "UObject/ObjectTypes.h"
#include "UObject/UObjectArray.h"

class FObjectFactory;
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

    void Render();

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

    /** World에서 관리되는 모든 Actor의 목록 */
    TSet<AActor*> ActorsArray;

    /** Actor가 Spawn되었고, 아직 BeginPlay가 호출되지 않은 Actor들 */
    TArray<AActor*> PendingBeginPlayActors;

    /** Actor 제거 대기열 */
    TArray<AActor*> PendingDestroyActors;

    AActor* PickedActor = nullptr;

    USceneComponent* pickingGizmo = nullptr;
    UCameraComponent* camera = nullptr;
    UPlayer* localPlayer = nullptr;

public:
    UObject* worldGizmo = nullptr;

    const TSet<AActor*>& GetActors() const { return ActorsArray; }

    UTransformGizmo* LocalGizmo = nullptr;
    UCameraComponent* GetCamera() const { return camera; }
    UPlayer* GetPlayer() const { return localPlayer; }


    // EditorManager 같은데로 보내기
    AActor* GetPickedActor() const { return PickedActor; }
    void SetPickedActor(AActor* InActor) { PickedActor = InActor; }

    UObject* GetWorldGizmo() const { return worldGizmo; }
    USceneComponent* GetPickingGizmo() const { return pickingGizmo; }
    void SetPickingGizmo(UObject* Object);
};


template <typename T>
    requires std::derived_from<T, AActor>
T* UWorld::SpawnActor()
{
    T* Actor = FObjectFactory::ConstructObject<T>();
    ActorsArray.Add(Actor);
    PendingBeginPlayActors.Add(Actor);
    return Actor;
}
