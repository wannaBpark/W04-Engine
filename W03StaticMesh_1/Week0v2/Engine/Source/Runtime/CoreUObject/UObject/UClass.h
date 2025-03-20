#pragma once
#include "Define.h"
class UClass
{
public:
    FString Name;
    UClass* ParentClass;

    UClass(FString InName, UClass* InParent)
        : Name(InName), ParentClass(InParent) {
    }
};