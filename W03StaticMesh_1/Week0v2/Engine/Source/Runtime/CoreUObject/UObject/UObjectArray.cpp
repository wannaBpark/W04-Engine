#include "UObjectArray.h"
#include "Object.h"


void FUObjectArray::AddObject(UObject* Object)
{
    ObjObjects.Add(Object);
}

void FUObjectArray::MarkRemoveObject(UObject* Object)
{
    ObjObjects.Remove(Object);
    PendingDestroyObjects.AddUnique(Object);
}

void FUObjectArray::ProcessPendingDestroyObjects()
{
    for (const UObject* Object : PendingDestroyObjects)
    {
        delete Object;
    }
    PendingDestroyObjects.Empty();
}

FUObjectArray GUObjectArray;
