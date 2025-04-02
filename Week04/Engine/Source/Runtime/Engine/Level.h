#pragma once
#include "UObject/ObjectMacros.h"


class AActor;
class UObject;
class UWorld;

class ULevel : public UObject
{
    DECLARE_CLASS(ULevel, UObject)
    friend class UWorld;
public:
    ULevel() = default;
    void InitializeLevel(UWorld* InOwingWorld);
    void Tick(float DeltaTime);
    void Release();
    
public:
    const TSet<AActor*>& GetActors() const { return ActorsArray; }
    void DestoryActor(AActor* Actor);
    void SetWorld(UWorld* InWorld) { OwningWorld = InWorld; }
    UWorld* GetWorld() const { return OwningWorld; }
    bool GetIsInitializedLevel() const { return bIsInitialized; }

private:
    bool bIsInitialized = false;

    UWorld* OwningWorld = nullptr;
    TSet<AActor*> ActorsArray;
    TArray<AActor*> PendingBeginPlayActors;
};

