#include "UObjectHash.h"


struct FUObjectHashTables
{
    static FUObjectHashTables& Get()
    {
        static FUObjectHashTables Singleton;
        return Singleton;
    }
};

void GetObjectsOfClass(const UClass* ClassToLookFor, TArray<UObject*>& Results)
{
}
