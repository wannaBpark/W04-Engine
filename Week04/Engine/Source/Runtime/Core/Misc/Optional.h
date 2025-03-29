#pragma once


template <typename OptionalType>
struct TOptional
{
public:
    using ElementType = OptionalType;

    TOptional(const OptionalType& Other)
        
    {
    }

private:
    alignas(OptionalType) OptionalType Value;
};
