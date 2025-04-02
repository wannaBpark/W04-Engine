#include "UClass.h"
#include <cassert>

#include "EngineStatics.h"
#include "UObjectArray.h"


UClass::UClass(
    const char* InClassName,
    uint32 InClassSize,
    uint32 InAlignment,
    UClass* InSuperClass,
    ObjectCreator InCreator
)
    : ClassSize(InClassSize)
    , ClassAlignment(InAlignment)
    , SuperClass(InSuperClass)
    , ClassConstructor(InCreator)
{
    NamePrivate = InClassName;
}

bool UClass::IsChildOf(const UClass* SomeBase) const
{
    assert(this);
    if (!SomeBase) return false;

    // Super의 Super를 반복하면서 
    for (const UClass* TempClass = this; TempClass; TempClass=TempClass->GetSuperClass())
    {
        if (TempClass == SomeBase)
        {
            return true;
        }
    }
    return false;
}

UObject* UClass::CreateDefaultObject()
{
    if (!ClassDefaultObject && ClassConstructor)
    {
        // TODO: CDO 수정
        return nullptr;

        // ClassDefaultObject = ClassConstructor();
        // ClassDefaultObject->ClassPrivate = this;
        // ClassDefaultObject->NamePrivate = GetName() + "_CDO";
        // ClassDefaultObject->UUID = UEngineStatics::GenUUID();
        //
        // GUObjectArray.AddObject(ClassDefaultObject);
    }
    return ClassDefaultObject;
}
