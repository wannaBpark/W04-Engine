#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

UObject::UObject() : Name(FName("DefaultObjectName"))
{
}

UObject::UObject(FString& name) : Name(FName(name))
{
}

UObject::UObject(FString& name, uint32 uuid) : Name(FName(name)), UUID(uuid)
{
}

UObject::~UObject()
{
}

void UObject::Initialize()
{
	InternalIndex = static_cast<uint32>(GetWorld()->GetObjectArr().size() - 1);

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
