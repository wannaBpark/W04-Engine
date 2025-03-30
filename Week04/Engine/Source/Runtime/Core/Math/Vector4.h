#pragma once

// 4D Vector
struct alignas(16) FVector4
{
    float X, Y, Z, W;

    FVector4(float _x = 0, float _y = 0, float _z = 0, float _a = 0)
        : X(_x), Y(_y), Z(_z), W(_a)
    {
    }

    FVector4 operator-(const FVector4& Other) const
    {
        return {X - Other.X, Y - Other.Y, Z - Other.Z, W - Other.W};
    }

    FVector4 operator+(const FVector4& Other) const
    {
        return {X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W};
    }

    FVector4 operator/(float Scalar) const
    {
        return {X / Scalar, Y / Scalar, Z / Scalar, W / Scalar};
    }
};
