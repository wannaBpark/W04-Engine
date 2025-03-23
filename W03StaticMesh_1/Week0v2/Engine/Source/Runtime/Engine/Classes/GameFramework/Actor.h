#pragma once
#include "Components/SceneComponent.h"
#include "Container/Set.h"
#include "UObject/Casts.h"
#include "UObject/Object.h"
#include "UObject/ObjectFactory.h"
#include "UObject/ObjectMacros.h"


class UActorComponent;

class AActor : public UObject
{
    DECLARE_CLASS(AActor, UObject)

public:
    AActor() = default;

    /** Actor가 게임에 배치되거나 스폰될 때 호출됩니다. */
    virtual void BeginPlay();

    /** 매 Tick마다 호출됩니다. */
    virtual void Tick(float DeltaTime);

    /** Actor가 제거될 때 호출됩니다. */
    virtual void Destroyed();

    // TODO: 추후 제거해야 함
    virtual void Render();

public:
    /** 이 Actor를 제거합니다. */
    virtual bool Destroy();

    /**
     * Actor에 컴포넌트를 새로 추가합니다.
     * @tparam T UActorComponent를 상속받은 Component
     * @return 생성된 Component
     */
    template <typename T>
        requires std::derived_from<T, UActorComponent>
    T* AddComponent();

    /** Actor가 가지고 있는 Component를 제거합니다. */
    void RemoveOwnedComponent(UActorComponent* Component);

    /** Actor가 가지고 있는 모든 컴포넌트를 가져옵니다. */
    const TSet<UActorComponent*>& GetComponents() const { return OwnedComponents; }

    template<typename T>
        requires std::derived_from<T, UActorComponent>
    T* GetComponentByClass();

public:
    USceneComponent* GetRootComponent() const { return RootComponent; }
    void SetRootComponent(USceneComponent* NewRootComponent);

    AActor* GetOwner() const { return Owner; }
    void SetOwner(AActor* NewOwner) { Owner = NewOwner; }

public:
    FVector GetActorLocation() const { return RootComponent ? RootComponent->GetWorldLocation() : FVector::ZeroVector; }
    FVector GetActorRotation() const { return RootComponent ? RootComponent->GetWorldRotation() : FVector::ZeroVector; }
    FVector GetActorScale() const { return RootComponent ? RootComponent->GetWorldScale() : FVector::ZeroVector; }

    bool SetActorLocation(const FVector& NewLocation);
    bool SetActorRotation(const FVector& NewRotation);
    bool SetActorScale(const FVector& NewScale);

protected:
    USceneComponent* RootComponent = nullptr;

private:
    /** 이 Actor를 소유하고 있는 다른 Actor의 정보 */
    AActor* Owner = nullptr;

    TSet<UActorComponent*> OwnedComponents;

#if 1 // TODO: WITH_EDITOR 추가
public:
    /** Actor의 기본 Label을 가져옵니다. */
    FString GetDefaultActorLabel() const;

    /** Actor의 Label을 가져옵니다. */
    FString GetActorLabel() const;

    /** Actor의 Label을 설정합니다. */
    void SetActorLabel(const FString& NewActorLabel);

private:
    /** 에디터상에 보이는 Actor의 이름 */
    FString ActorLabel;
#endif
};


template <typename T> requires std::derived_from<T, UActorComponent>
T* AActor::AddComponent()
{
    T* Component = FObjectFactory::ConstructObject<T>();
    OwnedComponents.Add(Component);
    Component->Owner = this;

    // 만약 SceneComponent를 상속 받았다면
    if (USceneComponent* NewSceneComp = Cast<USceneComponent>(Component))
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

    Component->InitializeComponent();
    return Component;
}

template <typename T> requires std::derived_from<T, UActorComponent>
T* AActor::GetComponentByClass()
{
    for (UActorComponent* Component : OwnedComponents)
    {
        if (T* CastedComponent = Cast<T>(Component))
        {
            return CastedComponent;
        }
    }
    return nullptr;
}