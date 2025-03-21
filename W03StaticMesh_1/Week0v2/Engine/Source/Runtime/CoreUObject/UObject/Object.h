#pragma once
#include "Define.h"
#include "EngineLoop.h"
#include "NameTypes.h"
#include "Engine/Source/Runtime/Engine/World.h"
#include "UClass.h"

extern FEngineLoop GEngineLoop;

class UClass;
class UWorld;


class UObject
{
public:
    UObject();
    virtual ~UObject();

    virtual void Initialize();
    virtual void Update(double deltaTime);
    virtual void Release();
    virtual void Render();
    virtual void RenderUUID();

    inline UWorld* GetWorld()
    {
        return GEngineLoop.GetWorld();
    }

    inline FEngineLoop& GetEngine()
    {
        return GEngineLoop;
    }

    inline void SetName(const FString& _Name)
    {
        Name = FName(_Name);
    }

    inline FName GetFName() const
    {
        return Name;
    }

    inline FString GetName() const
    {
        return Name.ToString();
    }

public:
    uint32 UUID;
    uint32 InternalIndex; // Index of GUObjectArray
    FName Name;

public:
    static void* operator new(size_t size)
    {
        UE_LOG(LogLevel::Display, "UObject Created : %d", size);
        FEngineLoop::TotalAllocationBytes += static_cast<uint32>(size);
        FEngineLoop::TotalAllocationCount++;

        UE_LOG(LogLevel::Display, "TotalAllocationBytes : %d, TotalAllocationCount : %d", FEngineLoop::TotalAllocationBytes, FEngineLoop::TotalAllocationCount);
        return std::malloc(size);
    }

    static void operator delete(void* ptr)
    {
        std::free(ptr);
    }

    virtual UClass* GetClass() const = 0;

    bool IsA(UClass* TargetClass) const;

    template <typename T>
        requires std::derived_from<T, UObject>
    bool IsA() const
    {
        return IsA(T::StaticClass());
    }

    static UClass* StaticClass()
    {
        static UClass ClassInfo("UObject", nullptr);
        return &ClassInfo;
    }

private:
};
