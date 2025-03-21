#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class AActor;

class UActorComponent : public UObject
{
    DECLARE_CLASS(UActorComponent, UObject)

public:
    UActorComponent() = default;

    virtual void Initialize();
    virtual void BeginPlay();
    virtual void TickComponent(float DeltaTime);
    // virtual void 
    virtual void Release();
    virtual void Render();


    AActor* GetOwner() const { return Owner; }

private:
    AActor* Owner;
};
