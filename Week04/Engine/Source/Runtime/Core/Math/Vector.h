#pragma once

#include <DirectXMath.h>

#include "MathUtility.h"

struct FVector2D
{
    float x, y;

    FVector2D(float _x = 0, float _y = 0)
        : x(_x)
        , y(_y)
    {
    }

    FVector2D operator+(const FVector2D& rhs) const
    {
        return FVector2D(x + rhs.x, y + rhs.y);
    }

    FVector2D operator-(const FVector2D& rhs) const
    {
        return FVector2D(x - rhs.x, y - rhs.y);
    }

    FVector2D operator*(float rhs) const
    {
        return FVector2D(x * rhs, y * rhs);
    }

    FVector2D operator/(float rhs) const
    {
        return FVector2D(x / rhs, y / rhs);
    }

    FVector2D& operator+=(const FVector2D& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
};

struct FVector
{
    float X, Y, Z;

    FVector() : X(0), Y(0), Z(0) {}
    FVector(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
    FVector(float Scalar) : X(Scalar), Y(Scalar), Z(Scalar) {}

    // Vector(0, 0, 0)
    static const FVector ZeroVector;
    static const FVector OneVector;
    static const FVector UpVector;
    static const FVector ForwardVector;
    static const FVector RightVector;

    FVector(float _x = 0, float _y = 0, float _z = 0)
        : x(_x)
        , y(_y)
        , z(_z)
    {
    }

    FVector operator-(const FVector& other) const
    {
        return FVector(x - other.x, y - other.y, z - other.z);
    }


    float Length() const;
    float LengthSquared() const;
    }


inline float FVector::Length() const
{
    return FMath::Sqrt(X * X + Y * Y + Z * Z);
}

inline float FVector::LengthSquared() const
{
    return X * X + Y * Y + Z * Z;
}

    }

    FVector ComponentMax(const FVector& other) const
    {
        return FVector(
            FMath::Max(x, other.x),
            FMath::Max(y, other.y),
            FMath::Max(z, other.z)
        );
    }
};
