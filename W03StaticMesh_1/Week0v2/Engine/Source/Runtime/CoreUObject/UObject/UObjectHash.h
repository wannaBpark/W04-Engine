#pragma once
#include "Container/Array.h"

class UObject;
class UClass;

/**
 * ClassToLookFor와 일치하는 UObject를 반환합니다.
 * @param ClassToLookFor 반환할 Object의 Class정보
 * @param Results Objects
 */
void GetObjectsOfClass(const UClass* ClassToLookFor, TArray<UObject*>& Results);

void AddToClassMap(const UObject* Object);
