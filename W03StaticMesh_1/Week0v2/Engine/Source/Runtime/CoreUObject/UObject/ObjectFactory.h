#pragma once
#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"
#include "EngineStatics.h"

class FObjectFactory
{
public:
    template<class T> 
    static UObject* ConstructObject()
    {
        uint32 id = UEngineStatics::GenUUID();
        FString Name = "DefualtObjectName_" + std::to_string(id);
        UObject* obj = new T;
        obj->SetName(Name);
        obj->UUID = id;
        obj->Initialize();

        return obj;
    }

    template<class T>
    static UObject* ConstructObject(FString& Name)
    {
        uint32 id = UEngineStatics::GenUUID();
        Name = Name + "_" + std::to_string(id);
        UE_LOG(LogLevel::Display, "Created New Object : %s", Name);
        UObject* obj = new T;
        obj->SetName(Name);
        obj->UUID = id;
        obj->Initialize();

        return obj;
    }

    template<class T>
    static UObject* ConstructObject(const FString& Name)
    {
        uint32 id = UEngineStatics::GenUUID();
        FString newName = Name + "_" + std::to_string(id);
        UE_LOG(LogLevel::Display, "Created New Object : %s", newName);
        UObject* obj = new T;
        obj->SetName(Name);
        obj->UUID = id;
        obj->Initialize();

        return obj;
    }
    template<class T>
    static UObject* ConstructObject(const char* Name)
    {
        uint32 id = UEngineStatics::GenUUID();
        FString newName = FString(Name) + "_" + std::to_string(id);
        UE_LOG(LogLevel::Display, "Created New Object : %s", *newName);
        UObject* obj = new T;
        obj->SetName(newName);
        obj->UUID = id;
        obj->Initialize();

        return obj;
    }
};

