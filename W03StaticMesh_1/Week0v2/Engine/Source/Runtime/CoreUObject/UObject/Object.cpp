#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"

#include "ObjectMacros.h"
#include "UClass.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"


UClass* UObject::StaticClass()
{
    static UClass ClassInfo{TEXT("UObject"), sizeof(UObject), alignof(UObject), nullptr};
    return &ClassInfo;
}

UObject::UObject()
    : UUID(0)
    // BUG: Array가 변경될경우 Index가 잘못될 가능성이 있음
    , InternalIndex(static_cast<uint32>(GetWorld()->GetObjectArr().Num() - 1))
    , NamePrivate("None")
{
}

UObject::~UObject()
{
}

void UObject::Render()
{
}

void UObject::RenderUUID()
{
}

bool UObject::IsA(const UClass* SomeBase) const
{
    const UClass* ThisClass = GetClass();
    return ThisClass->IsChildOf(SomeBase);
}
