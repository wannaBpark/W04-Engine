#include "UObject/Object.h"

struct FWorldContext
{
    EWorldType::Type WorldType;

    int32	PIEInstance;    /** The PIE instance of this world, -1 is default */
    FString	PIEPrefix;      /** The Prefix in front of PIE level names, empty is default */


    // If > 0, tick this world at a fixed rate in PIE
    float PIEFixedTickSeconds = 0.f;
    float PIEAccumulatedTickSeconds = 0.f;

    /** Set CurrentWorld and update external reference pointers to reflect this*/
    void SetCurrentWorld(UWorld* World) { ThisCurrentWorld = std::make_shared<UWorld>(World); }


    FORCEINLINE UWorld* World() const
    {
        return ThisCurrentWorld.get();
    }

    FWorldContext()
        : WorldType(EWorldType::None)
        , PIEInstance(INDEX_NONE)
        , ThisCurrentWorld(nullptr)
    {
    }

private:
    std::shared_ptr<UWorld>	ThisCurrentWorld;
};


class UEditorEngine : public UObject
{
    TArray<FWorldContext> WorldContexts;

    virtual void Tick(float DeltaSeconds);
};

void UEditorEngine::Tick(float DeltaSeconds)
{
    // Editor 전용 액터 Tick 처리
    for (FWorldContext& WorldContext : WorldContexts)
    {
        UWorld* EditorWorld = WorldContext.World();
        if (EditorWorld && EditorWorld->WorldType == EWorldType::Editor)
        {
            ULevel* Level = EditorWorld->Level;
            {
                for (AActor* Actor : Level->Actors)
                {
                    if (Actor && Actor->bTickInEditor)
                    {
                        Actor->Tick(DeltaSeconds);
                    }
                }
            }
        }
        else if (EditorWorld && EditorWorld->WorldType == EWorldType::PIE)
        {
            ULevel* Level = EditorWorld->Level;
            {
                for (AActor* Actor : Level->Actors)
                {
                    if (Actor)
                    {
                        Actor->Tick(DeltaSeconds);
                    }
                }
            }
        }
    }
}

void StartPIE()
{
    UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

    UWorld* PIEWorld = UWorld::DuplicateWorldForPIE(EditorWorld, ...);

    GWorld = PIEWorld;

    // AActor::BeginPlay()
    PIEWorld->InitializeActorsForPlay();
}

void EndPIE()
{
    if (GWorld && GWorld->IsPIEWorld())
    {
        GWorld->CleanupWorld();
        delete GWorld;
    }

    GWorld = GEditor->GetEditorWorldContext().World();
}