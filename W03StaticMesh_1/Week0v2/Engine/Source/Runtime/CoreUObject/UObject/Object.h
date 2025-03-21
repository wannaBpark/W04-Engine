#pragma once
#include "EngineLoop.h"
#include "NameTypes.h"
#include "Engine/Source/Runtime/Engine/World.h"

extern FEngineLoop GEngineLoop;

class UClass;
class UWorld;


class UObject
{
private:
    UObject(const UObject&) = delete;
    UObject& operator=(const UObject&) = delete;
    UObject(UObject&&) = delete;
    UObject& operator=(UObject&&) = delete;

public:
    using Super = UObject;
    using ThisClass = UObject;

    static UClass* StaticClass();

private:
    friend class FObjectFactory;
    friend class FSceneMgr;

    uint32 UUID;
    uint32 InternalIndex; // Index of GUObjectArray

    FName NamePrivate;
    UClass* ClassPrivate = nullptr;

public:
    UObject();
    virtual ~UObject();

    virtual void Initialize();
    virtual void Update(double deltaTime);
    virtual void Release();
    virtual void Render();
    virtual void RenderUUID();

    UWorld* GetWorld()
    {
        return GEngineLoop.GetWorld();
    }

    FEngineLoop& GetEngine()
    {
        return GEngineLoop;
    }

    FName GetFName() const { return NamePrivate; }
    FString GetName() const { return NamePrivate.ToString(); }

    uint32 GetUUID() const { return UUID; }
    uint32 GetInternalIndex() const { return InternalIndex; }

    UClass* GetClass() const { return ClassPrivate; }


    /** this가 SomeBase인지, SomeBase의 자식 클래스인지 확인합니다. */
    bool IsA(const UClass* SomeBase) const;

    template <typename T>
        requires std::derived_from<T, UObject>
    bool IsA() const
    {
        return IsA(T::StaticClass());
    }

public:
    void* operator new(size_t size)
    {
        UE_LOG(LogLevel::Display, "UObject Created : %d", size);
        FEngineLoop::TotalAllocationBytes += static_cast<uint32>(size);
        FEngineLoop::TotalAllocationCount++;

        UE_LOG(LogLevel::Display, "TotalAllocationBytes : %d, TotalAllocationCount : %d", FEngineLoop::TotalAllocationBytes, FEngineLoop::TotalAllocationCount);
        return std::malloc(size);
    }

    void operator delete(void* ptr)
    {
        std::free(ptr);
    }
};
