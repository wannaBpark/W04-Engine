#include "SkySphereComponent.h"

#include "World.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "LevelEditor/SLevelEditor.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"


USkySphereComponent::USkySphereComponent()
{
    SetType(StaticClass()->GetName());
}

USkySphereComponent::~USkySphereComponent()
{
}

void USkySphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USkySphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}