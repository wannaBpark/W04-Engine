#pragma once
#include "Container/Array.h"

class UClass;
class UObject;


class FUObjectArray
{
public:
    void AddObject(UObject* Object);
    void MarkRemoveObject(UObject* Object);

    void ProcessPendingDestroyObjects();

    TArray<UObject*>& GetObjectItemArrayUnsafe()
    {
        return ObjObjects;
    }

    const TArray<UObject*>& GetObjectItemArrayUnsafe() const
    {
        return ObjObjects;
    }

private:
    TArray<UObject*> ObjObjects;
    TArray<UObject*> PendingDestroyObjects;
};

extern FUObjectArray GUObjectArray;
