#include "Components/SceneComponent.h"
#include "World.h"
#include "Math/JungleMath.h"
#include "UObject/ObjectFactory.h"
#include "UUIDRenderComponent.h"

// 중복 제거됨

USceneComponent::USceneComponent()
    : RelativeLocation(FVector(0.f, 0.f, 0.f))
    , RelativeRotation(FVector(0.f, 0.f, 0.f))
    , RelativeScale3D(FVector(1.f, 1.f, 1.f))
{
    QuatRotation = JungleMath::EulerToQuaternion(RelativeRotation);
}

void USceneComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USceneComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int USceneComponent::CheckRayIntersection(FVector& RayOrigin, FVector& RayDirection, float& OutNearHitDistance)
{
    int nIntersections = 0;
    return nIntersections;
}

FVector USceneComponent::GetForwardVector()
{
    FVector Forward = FVector(1.f, 0.f, 0.0f);
    return JungleMath::FVectorRotate(Forward, QuatRotation);
}

FVector USceneComponent::GetRightVector()
{
    FVector Right = FVector(0.f, 1.f, 0.0f);
    return JungleMath::FVectorRotate(Right, QuatRotation);
}

FVector USceneComponent::GetUpVector()
{
    FVector Up = FVector(0.f, 0.f, 1.0f);
    return JungleMath::FVectorRotate(Up, QuatRotation);
}

void USceneComponent::AddLocation(FVector Added)
{
    RelativeLocation += Added;
}

void USceneComponent::AddRotation(FVector Added)
{
    RelativeRotation += Added;
    QuatRotation = JungleMath::EulerToQuaternion(RelativeRotation);
}

void USceneComponent::AddScale(FVector Added)
{
    RelativeScale3D += Added;
}

FVector USceneComponent::GetWorldRotation() const
{
    if (AttachParent)
    {
        return AttachParent->GetLocalRotation() + GetLocalRotation();
    }
    return GetLocalRotation();
}

FVector USceneComponent::GetWorldScale() const
{
    if (AttachParent)
    {
        return AttachParent->GetWorldScale() * GetLocalScale(); // Scale은 곱셈
    }
    return GetLocalScale();
}

FVector USceneComponent::GetWorldLocation() const
{
    if (AttachParent)
    {
        return AttachParent->GetWorldLocation() + GetLocalLocation();
    }
    return GetLocalLocation();
}

FVector USceneComponent::GetLocalRotation() const
{
    return JungleMath::QuaternionToEuler(QuatRotation);
}

void USceneComponent::SetRotation(FVector NewRotation)
{
    RelativeRotation = NewRotation;
    QuatRotation = JungleMath::EulerToQuaternion(NewRotation);
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    if (
        InParent != AttachParent &&
        InParent != this &&
        InParent != nullptr &&
        (AttachParent == nullptr || !AttachParent->AttachChildren.Contains(this))
        ) {
        AttachParent = InParent;
        InParent->AttachChildren.AddUnique(this);
    }
}

TArray<USceneComponent*> USceneComponent::GetAttachChildren()
{
    return AttachChildren;
}
