#include "Actor.h"

void AActor::BeginPlay()
{
}

void AActor::Tick(float DeltaTime)
{
}

void AActor::Destroyed()
{
}

void AActor::Destroy()
{
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
