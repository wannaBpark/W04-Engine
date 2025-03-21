#pragma once
#include "Container/String.h"

class UClass
{
private:
    FString Name;
    UClass* SuperClass;

public:

    UClass(FString InName, UClass* InParent)
        : Name(std::move(InName))
        , SuperClass(InParent) {
    }

    FString GetName() const { return Name; }
    UClass* GetSuperClass() const { return SuperClass; }
};