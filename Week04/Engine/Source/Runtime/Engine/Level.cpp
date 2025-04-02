#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"

void ULevel::InitializeLevel(UWorld* InOwningWorld)
{
    OwningWorld = InOwningWorld;
    bIsInitialized = true;
}

void ULevel::Release()
{
    for (AActor* Actor : ActorsArray)
    {
        Actor->EndPlay(EEndPlayReason::WorldTransition);
        TArray<UActorComponent*> CopiedComponents = Actor->GetComponents();
        for (UActorComponent* Component : CopiedComponents)
        {
            GUObjectArray.MarkRemoveObject(Component);
        }
        GUObjectArray.MarkRemoveObject(Actor);
    }
    ActorsArray.Empty();

    //GUObjectArray.ProcessPendingDestroyObjects();
}

void ULevel::DestoryActor(AActor* Actor)
{

}

void ULevel::Tick(float DeltaTime)
{
    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    for (AActor* Actor : PendingBeginPlayActors)
    {
        Actor->BeginPlay();
    }
    PendingBeginPlayActors.Empty();

    // 매 틱마다 Actor->Tick(...) 호출
    for (AActor* Actor : ActorsArray)
    {
        Actor->Tick(DeltaTime);
    }
}
