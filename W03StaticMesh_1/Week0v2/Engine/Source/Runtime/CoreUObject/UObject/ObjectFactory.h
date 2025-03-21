#pragma once
#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"
#include "EngineStatics.h"

class FObjectFactory
{
public:
    template<typename T> 
    static T* ConstructObject()
    {
        uint32 id = UEngineStatics::GenUUID();
        FString Name = T::StaticClass()->GetName() + "_" + std::to_string(id);
        T* obj = new T;
        obj->SetName(Name);
        obj->UUID = id;
        obj->Initialize();

        UE_LOG(LogLevel::Display, "Created New Object : %s", Name);
        return obj;
    }
};
