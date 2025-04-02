#pragma once
#include <concepts>

#include "EngineStatics.h"
#include "UClass.h"
#include "UObjectArray.h"

class UObject;


class FObjectFactory
{
public:
    static UObject* ConstructObject(UClass* InClass)
    {
        uint32 Id = UEngineStatics::GenUUID();
        FString Name = InClass->GetName() + "_" + std::to_string(Id);
        
        // TODO: FPlatformMemory::Malloc으로 변경, placement new 사용시 Free방법 생각하기
        UObject* Obj = InClass->CreateObject();
        Obj->ClassPrivate = InClass;
        Obj->NamePrivate = Name;
        Obj->UUID = Id;
        
        GUObjectArray.AddObject(Obj);
        UE_LOG(LogLevel::Display, "Created New Object : %s", *Name);
        return Obj;
    }

    template<typename T>
        requires std::derived_from<T, UObject>
    static T* ConstructObject()
    {
        return static_cast<T*>(ConstructObject(T::StaticClass()));
    }
};
