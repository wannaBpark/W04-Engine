#include "EditorEngine.h"


void UEditorEngine::Initialize()
{
    if (WorldContexts.Num() == 0) {
        FWorldContext WorldContext;
        UWorld* GWorld = FObjectFactory::ConstructObject<UWorld>();
        GWorld->Initialize();
        WorldContext.SetCurrentWorld(GWorld);
        WorldContexts.Add(WorldContext);
    }
}

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
        }
        else if (EditorWorld && CurrentWorldType == EWorldType::PIE)
        {
            EditorWorld->Tick(DeltaTime);
        }
    }
}

void UEditorEngine::StartPIE()
{
    //UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

    //UWorld* PIEWorld = UWorld::DuplicateWorldForPIE(EditorWorld, /* PIE 월드 이름 */);

    //GWorld = PIEWorld;

    //// AActor::BeginPlay()
    //PIEWorld->InitializeActorsForPlay();
}

void UEditorEngine::EndPIE()
{
    /*if (GWorld && GWorld->IsPIEWorld())
    {
        GWorld->CleanupWorld();
        delete GWorld;
    }

    GWorld = GEditor->GetEditorWorldContext().World();*/
}

void UEditorEngine::Release()
{
    if (WorldContexts.Num())
    {
        for (auto& WorldContext : WorldContexts)
        {
            WorldContext.World()->Release();
            GUObjectArray.MarkRemoveObject(WorldContext.World());
        }
        WorldContexts.Empty();
    }
}

FWorldContext& UEditorEngine::GetEditorWorldContext() const
{
    for (const FWorldContext& Context : WorldContexts)
    {
        if (Context.WorldType == EWorldType::Editor)
        {
            return const_cast<FWorldContext&>(Context);
        }
    }

    // 에디터 월드 컨텍스트를 찾지 못한 경우
    UE_LOG(LogLevel::Display,TEXT("UEditorEngine::GetEditorWorldContext() - No Editor World found."));
    
    // 기본값으로 인한 컴파일 경고 방지 (도달 불가)
    return const_cast<FWorldContext&>(WorldContexts[0]);
}


UEditorEngine* GEditor = nullptr;