#include "GameFramework/Actor.h"
#include "Engine/Level.h"

#if 1 // TODO: WITH_EDITOR 추가

void AActor::SetLevel(ULevel* InLevel)
{
    Level = InLevel; 
}

ULevel* AActor::GetLevel() const
{
    return Level;
}

FString AActor::GetDefaultActorLabel() const
{
    const UClass* ActorClass = GetClass();
    FString DefaultActorLabel = ActorClass->GetName();
    return DefaultActorLabel;
}

FString AActor::GetActorLabel() const
{
    if (ActorLabel.IsEmpty())
    {
        FString DefaultActorLabel = GetDefaultActorLabel();
        // TODO: Unique한 이름으로 만들기
        DefaultActorLabel += FString::FromInt(GetUUID());

        AActor* MutableThis = const_cast<AActor*>(this);
        MutableThis->ActorLabel = std::move(DefaultActorLabel);
    }

    return ActorLabel; 
}

void AActor::SetActorLabel(const FString& NewActorLabel)
{
    // NewActorLabel != GetActorLabel
    if (FCString::Strcmp(*NewActorLabel, *GetActorLabel()) != 0)
    {
        ActorLabel = NewActorLabel + "_" + FString::FromInt(GetUUID());
    }
}

#endif
