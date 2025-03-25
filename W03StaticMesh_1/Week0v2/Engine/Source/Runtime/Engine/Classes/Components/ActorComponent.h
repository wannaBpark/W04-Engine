#pragma once
#include "Engine/EngineTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class AActor;

class UActorComponent : public UObject
{
    DECLARE_CLASS(UActorComponent, UObject)

private:
    friend class AActor;

public:
    UActorComponent() = default;

    /** AActor가 World에 Spawn되어 BeginPlay이전에 호출됩니다. */
    virtual void InitializeComponent();

    /** 모든 초기화가 끝나고, 준비가 되었을 때 호출됩니다. */
    virtual void BeginPlay();

    /** 매 틱마다 호출됩니다. */
    virtual void TickComponent(float DeltaTime);

    /** Component가 제거되었을 때 호출됩니다. */
    virtual void OnComponentDestroyed();

    /**
     * Ends gameplay for this component.
     * Called from AActor::EndPlay only
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
    /** 이 컴포넌트를 소유하고 있는 Actor를 반환합니다. */
    AActor* GetOwner() const { return Owner; }

    /** 이 컴포넌트를 제거합니다. */
    virtual void DestroyComponent();

    /** Component의 BeginPlay가 호출 되었는지 여부를 반환합니다. */
    bool HasBegunPlay() const { return bHasBegunPlay; }

private:
    AActor* Owner;

    /** BeginPlay가 호출 되었는지 여부 */
    uint8 bHasBegunPlay : 1;

    /** 현재 컴포넌트가 삭제 처리중인지 여부 */
    uint8 bIsBeingDestroyed : 1;
};
