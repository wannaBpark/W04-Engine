#include "ActorComponent.h"

#include "GameFramework/Actor.h"


void UActorComponent::InitializeComponent()
{
}

void UActorComponent::BeginPlay()
{
}

void UActorComponent::TickComponent(float DeltaTime)
{
}

void UActorComponent::OnComponentDestroyed()
{
}

void UActorComponent::Release()
{
}

void UActorComponent::Render()
{
}

void UActorComponent::DestroyComponent()
{
    if (bIsBeingDestroyed)
    {
        return;
    }

    bIsBeingDestroyed = true;

    // Owner에서 Component 제거하기
    if (AActor* MyOwner = GetOwner())
    {
        MyOwner->RemoveOwnedComponent(this);
        if (MyOwner->GetRootComponent() == this)
        {
            MyOwner->SetRootComponent(nullptr);
        }
    }

    OnComponentDestroyed();
}
