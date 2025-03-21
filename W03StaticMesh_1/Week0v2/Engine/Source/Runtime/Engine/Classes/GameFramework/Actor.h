#pragma once
#include "Components/SceneComponent.h"
#include "Container/Set.h"
#include "UObject/Object.h"
#include "UObject/ObjectFactory.h"
#include "UObject/ObjectMacros.h"


class UActorComponent;

class AActor : public UObject
{
    DECLARE_CLASS(AActor, UObject)

public:
    AActor() = default;

    virtual void BeginPlay();
    virtual void Tick(float DeltaTime);
    virtual void Destroyed();

    virtual void Destroy();

public:
    template <typename T>
        requires std::derived_from<T, UActorComponent>
    T* AddComponent()
    {
        T* ObjectInstance = FObjectFactory::ConstructObject<T>();
        Components.Add(ObjectInstance);
        ObjectInstance->SetOwner(this);

        USceneComponent* NewSceneComp = dynamic_cast<USceneComponent*>(ObjectInstance);
        if (NewSceneComp != nullptr)
        {
            if (RootComponent == nullptr)
            {
                RootComponent = NewSceneComp;
            }
            else
            {
                NewSceneComp->SetupAttachment(RootComponent);
            }

        }

        return ObjectInstance;
    }

public:
    USceneComponent* GetRootComponent() const { return RootComponent; }
    void SetRootComponent(USceneComponent* NewRootComponent);

private:
    USceneComponent* RootComponent = nullptr;

    AActor* Owner = nullptr;
    TSet<UActorComponent*> Components;

#if 1 // TODO: WITH_EDITOR 추가
public:
    /** Actor의 기본 Label을 가져옵니다. */
    FString GetDefaultActorLabel() const;

    /** Actor의 Label을 가져옵니다. */
    FString GetActorLabel() const;

    /** Actor의 Label을 설정합니다. */
    void SetActorLabel(const FString& NewActorLabel);

private:
    /** 에디터상에 보이는 액터의 이름 */
    FString ActorLabel;
#endif
};
