#pragma once
#include "Object.h"
#include "Container/Array.h"

class UClass;

/**
 * ClassToLookFor와 일치하는 UObject를 반환합니다.
 * @param ClassToLookFor 반환할 Object의 CLass정보
 * @param Results Objects
 */
void GetObjectsOfClass(const UClass* ClassToLookFor, TArray<UObject*>& Results);
