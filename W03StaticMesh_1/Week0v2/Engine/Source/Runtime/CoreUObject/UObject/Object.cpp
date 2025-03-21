#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

UObject::UObject()
    : UUID(0)
    , InternalIndex(0)
    , Name(FName("DefaultObjectName"))
{
}

UObject::~UObject()
{
}

void UObject::Initialize()
{
    // BUG: Array가 변경될경우 Index가 잘못될 가능성이 있음
    InternalIndex = static_cast<uint32>(GetWorld()->GetObjectArr().Num() - 1);
}

void UObject::Update(double deltaTime)
{
}
void UObject::Release()
{
}
void UObject::Render()
{
}
void UObject::RenderUUID()
{
}
bool UObject::IsA(UClass* TargetClass) const
{
    UClass* CurrentClass = GetClass();
    while (CurrentClass) {
        if (CurrentClass == TargetClass)
            return true;
        CurrentClass = CurrentClass->ParentClass;
    }
    return false;
}
