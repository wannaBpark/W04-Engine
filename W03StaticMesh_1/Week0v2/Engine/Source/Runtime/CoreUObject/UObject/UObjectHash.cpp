#include "UObjectHash.h"
#include <cassert>
#include "Object.h"
#include "UClass.h"
#include "Container/Map.h"
#include "Container/Set.h"

/**
 * 모든 UObject의 정보를 담고 있는 HashTable
 */
struct FUObjectHashTables
{
    static FUObjectHashTables& Get()
    {
        static FUObjectHashTables Singleton;
        return Singleton;
    }

    TMap<UClass*, TSet<UClass*>> ClassToChildListMap;
};

void AddToClassMap(const UObject* Object)
{
    assert(Object->GetClass());
    FUObjectHashTables& HashTable = FUObjectHashTables::Get();

    UClass* Class = Object->GetClass();
    for (UClass* SuperClass = Class->GetSuperClass(); SuperClass;)
    {
        TSet<UClass*>& ChildList = HashTable.ClassToChildListMap.FindOrAdd(SuperClass);
        ChildList.Add(Class);

        Class = SuperClass;
        SuperClass = SuperClass->GetSuperClass();
    }
}

void GetObjectsOfClass(const UClass* ClassToLookFor, TArray<UObject*>& Results, bool bIncludeDerivedClasses)
{
}
