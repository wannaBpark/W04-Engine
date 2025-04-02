#include "UObject/Object.h"
#include "CoreUObject/UObject/UObjectArray.h"
#include "Engine/World.h"

struct FWorldContext
{
    EWorldType::Type WorldType;

    int32	PIEInstance;    /** The PIE instance of this world, -1 is default */
    FString	PIEPrefix;      /** The Prefix in front of PIE level names, empty is default */


    // If > 0, tick this world at a fixed rate in PIE
    float PIEFixedTickSeconds = 0.f;
    float PIEAccumulatedTickSeconds = 0.f;

    /** Set CurrentWorld and update external reference pointers to reflect this*/
    void SetCurrentWorld(UWorld* World) { ThisCurrentWorld = World; }


    FORCEINLINE UWorld* World() const
    {
        return ThisCurrentWorld;
    }

    FWorldContext()
        : WorldType(EWorldType::Editor) // 기본 설정 : Editor
        , PIEInstance(INDEX_NONE)
        , ThisCurrentWorld(nullptr)
    {
    }

    ~FWorldContext() { GUObjectArray.MarkRemoveObject(ThisCurrentWorld); }

private:
    UWorld*	ThisCurrentWorld;
};


class ULevel;

class UEditorEngine final : public UObject
{
    TArray<FWorldContext> WorldContexts;

    virtual void Tick(float DeltaSeconds);
    void StartPIE();
    void EndPIE();
};

void UEditorEngine::Tick(float DeltaTime)
{
    // Editor 전용 액터 Tick 처리
    for (FWorldContext& WorldContext : WorldContexts)
    {
        UWorld* EditorWorld = WorldContext.World();
        EWorldType CurrentWorldType = EditorWorld->GetWorldType();

        if (EditorWorld && CurrentWorldType == EWorldType::Editor)
        {
            EditorWorld->Tick(DeltaTime);
            
            const ULevel* Level = EditorWorld->GetPersistentLevel();
            {
                for (AActor* Actor : Level->GetActors())
                {
                    if (Actor && Actor->GetTickInEditor())
                    {
                        Actor->Tick(DeltaSeconds);
                    }
                }
            }
        }
        else if (EditorWorld && CurrentWorldType == EWorldType::PIE)
        {
            EditorWorld->Tick(DeltaTime);

            ULevel* Level = EditorWorld->GetPersistentLevel();
            {
                for (AActor* Actor : Level->GetActors())
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

void UEditorEngine::StartPIE()
{
    UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

    UWorld* PIEWorld = UWorld::DuplicateWorldForPIE(EditorWorld, /* PIE 월드 이름 */);

    GWorld = PIEWorld;

    // AActor::BeginPlay()
    PIEWorld->InitializeActorsForPlay();
}

void UEditorEngine::EndPIE()
{
    if (GWorld && GWorld->IsPIEWorld())
    {
        GWorld->CleanupWorld();
        delete GWorld;
    }

    GWorld = GEditor->GetEditorWorldContext().World();
}