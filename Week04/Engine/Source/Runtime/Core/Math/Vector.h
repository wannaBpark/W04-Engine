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
    float x, y, z;

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

    float& operator[](int index)
    {
        switch (index)
        {
        case 0:  return x;
        case 1:  return y;
        case 2:  return z;
        default: assert(false && "Index out of range"); return x; // 예외 처리
        }
    }

    FVector operator-(const FVector& other) const
    {
        return FVector(x - other.x, y - other.y, z - other.z);
    }

    FVector operator+(const FVector& other) const
    {
        return FVector(x + other.x, y + other.y, z + other.z);
    }

    // 벡터 내적  
    float Dot(const FVector& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    // 벡터 크기  
    float Magnitude() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    // 벡터 정규화  
    FVector Normalize() const
    {
        float mag = Magnitude();
        return (mag > 0) ? FVector(x / mag, y / mag, z / mag) : FVector(0, 0, 0);
    }

    FVector Cross(const FVector& Other) const
    {
        return FVector{
            y * Other.z - z * Other.y,
            z * Other.x - x * Other.z,
            x * Other.y - y * Other.x
        };
    }

    // 스칼라 곱셈  
    FVector operator*(float scalar) const
    {
        return FVector(x * scalar, y * scalar, z * scalar);
    }

    bool operator==(const FVector& other) const
    {
        return (x == other.x && y == other.y && z == other.z);
    }

    float Distance(const FVector& other) const
    {
        // 두 벡터의 차 벡터의 크기를 계산  
        return ((*this - other).Magnitude());
    }

    DirectX::XMFLOAT3 ToXMFLOAT3() const
    {
        return DirectX::XMFLOAT3(x, y, z);
    }

    // 추가된 함수
    FVector ComponentMin(const FVector& other) const
    {
        return FVector(
            FMath::Min(x, other.x),
            FMath::Min(y, other.y),
            FMath::Min(z, other.z)
        );
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
