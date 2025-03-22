#include "Actor.h"

void AActor::BeginPlay()
{
    for (UActorComponent* Comp : OwnedComponents)
    {
        Comp->BeginPlay();
    }
}

void AActor::Tick(float DeltaTime)
{
    // TODO: 임시로 Actor에서 Tick 돌리기
    for (UActorComponent* Comp : OwnedComponents)
    {
        Comp->TickComponent(DeltaTime);
    }
}

void AActor::Destroyed()
{
    for (UActorComponent* Comp : OwnedComponents)
    {
        Comp->DestroyComponent();
    }
}

bool AActor::Destroy()
{
    if (UWorld* World = GetWorld())
    {
        World->DestroyActor(this);
    }

    return true;
}

void AActor::RemoveOwnedComponent(UActorComponent* Component)
{
    OwnedComponents.Remove(Component);
}

void AActor::SetRootComponent(USceneComponent* NewRootComponent)
{
    if (NewRootComponent != nullptr || NewRootComponent->GetOwner() == this)
    {
        if (RootComponent != NewRootComponent)
        {
            USceneComponent* OldRootComponent = RootComponent;
            RootComponent = NewRootComponent;

            OldRootComponent->SetupAttachment(RootComponent);
        }
    }
}
