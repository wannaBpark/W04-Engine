#pragma once
#include "CoreUObject/UObject/Object.h"

struct FWorldContext
{
    EWorldType WorldType;

    int32	PIEInstance;    /** The PIE instance of this world, -1 is default */
    FString	PIEPrefix;      /** The Prefix in front of PIE level names, empty is default */


    // If > 0, tick this world at a fixed rate in PIE
    float PIEFixedTickSeconds = 0.f;
    float PIEAccumulatedTickSeconds = 0.f;

    /** Set CurrentWorld and update external reference pointers to reflect this*/
    void SetCurrentWorld(UWorld* World) { ThisCurrentWorld = World; }


    FORCEINLINE UWorld* World() const
    {
        return ThisCurrentWorld;
    }

    FWorldContext()
        : WorldType(EWorldType::Editor) // 기본 설정 : Editor
        , PIEInstance(INDEX_NONE)
        , ThisCurrentWorld(nullptr)
    {
    }

    ~FWorldContext() { /*GUObjectArray.MarkRemoveObject(ThisCurrentWorld);*/ }

private:
    UWorld* ThisCurrentWorld;
};

class UEngine : public UObject
{
    DECLARE_CLASS(UEngine, UObject)
public:
    UEngine() = default;
protected:
    TArray<FWorldContext> WorldContexts;
};