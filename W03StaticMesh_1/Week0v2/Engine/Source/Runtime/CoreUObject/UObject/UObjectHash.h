#pragma once
#include "Container/Array.h"

class UObject;
class UClass;

/**
 * ClassToLookFor와 일치하는 UObject를 반환합니다.
 * @param ClassToLookFor 반환할 Object의 Class정보
 * @param Results ClassToLookFor와 일치하는 모든 Objects
 * @param bIncludeDerivedClasses ClassToLookFor의 파생 클래스까지 찾을 지 여부
 */
void GetObjectsOfClass(const UClass* ClassToLookFor, TArray<UObject*>& Results, bool bIncludeDerivedClasses);

void AddToClassMap(const UObject* Object);

void RemoveFromClassMap(const UObject* Object);
