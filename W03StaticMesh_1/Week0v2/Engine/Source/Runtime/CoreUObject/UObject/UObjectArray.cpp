#include "UObjectArray.h"
#include "Object.h"
#include "UObjectHash.h"


void FUObjectArray::AddObject(UObject* Object)
{
    ObjObjects.AddUnique(Object);
    AddToClassMap(Object);
}

void FUObjectArray::MarkRemoveObject(UObject* Object)
{
    ObjObjects.Remove(Object);
    PendingDestroyObjects.AddUnique(Object);
}

void FUObjectArray::ProcessPendingDestroyObjects()
{
    for (UObject* Object : PendingDestroyObjects)
    {
        RemoveFromClassMap(Object);
        delete Object;
    }
    PendingDestroyObjects.Empty();
}

FUObjectArray GUObjectArray;
