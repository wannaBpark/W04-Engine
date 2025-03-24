#pragma once
#include "Object.h"
#include "UObjectHash.h"
#include "Container/Array.h"

#undef GetObject // Windows.h 이름 겹침


template <typename T>
class TObjectIterator
{
public:
    enum EEndTagType : uint8
    {
        EndTag
    };

    /** Begin 생성자 */
    explicit TObjectIterator(bool bIncludeDerivedClasses = true)
        : Index(-1)
    {
        GetObjectsOfClass(T::StaticClass(), ObjectArray, bIncludeDerivedClasses);
        Advance();
    }

    /** End 생성자 */
    TObjectIterator(EEndTagType, const TObjectIterator& Begin)
        : Index(Begin.ObjectArray.Num())
    {
    }

    FORCEINLINE void operator++()
    {
        Advance();
    }

    FORCEINLINE T* operator* () const
    {
        return (T*)GetObject();
    }

    FORCEINLINE T* operator-> () const
    {
        return (T*)GetObject();
    }

    FORCEINLINE bool operator==(const TObjectIterator& Rhs) const { return Index == Rhs.Index; }
    FORCEINLINE bool operator!=(const TObjectIterator& Rhs) const { return Index != Rhs.Index; }

protected:
    UObject* GetObject() const 
    { 
        return ObjectArray[Index];
    }

    bool Advance()
    {
        while(++Index < ObjectArray.Num())
        {
            if (GetObject())
            {
                return true;
            }
        }
        return false;
    }

protected:
    /** Results from the GetObjectsOfClass query */
    TArray<UObject*> ObjectArray;
    int32 Index;
};

template <typename T>
struct TObjectRange
{
    TObjectRange(bool bIncludeDerivedClasses = true)
        : Begin(bIncludeDerivedClasses)
    {
    }

    friend TObjectIterator<T> begin(const TObjectRange& Range) { return Range.Begin; }
    friend TObjectIterator<T> end  (const TObjectRange& Range) { return TObjectIterator<T>(TObjectIterator<T>::EndTag, Range.Begin); }

    TObjectIterator<T> Begin;
};
