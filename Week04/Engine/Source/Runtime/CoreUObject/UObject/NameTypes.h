﻿#pragma once
#include "Core/HAL/PlatformType.h"

class FString;

enum ENameNone : uint8 { NAME_None = 0 };

class FName
{
    friend struct FNameHelper;

    uint32 DisplayIndex;    // 원본 문자열의 Hash
    uint32 ComparisonIndex; // 비교시 사용되는 Hash

public:
    FName() : DisplayIndex(NAME_None), ComparisonIndex(NAME_None) {}
    FName(ENameNone) : DisplayIndex(NAME_None), ComparisonIndex(NAME_None) {}
    FName(const WIDECHAR* Name);
    FName(const ANSICHAR* Name);
    FName(const FString& Name);

    FString ToString() const;
    uint32 GetDisplayIndex() const { return DisplayIndex; }
    uint32 GetComparisonIndex() const { return ComparisonIndex; }

    bool operator==(const FName& Other) const;
};
