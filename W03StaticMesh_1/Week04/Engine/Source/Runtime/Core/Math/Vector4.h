#pragma once

// 4D Vector
struct FVector4 {
    float x, y, z, a;
    FVector4(float _x = 0, float _y = 0, float _z = 0, float _a = 0) : x(_x), y(_y), z(_z), a(_a) {}

    FVector4 operator-(const FVector4& other) const {
        return FVector4(x - other.x, y - other.y, z - other.z, a - other.a);
    }
    FVector4 operator+(const FVector4& other) const {
        return FVector4(x + other.x, y + other.y, z + other.z, a + other.a);
    }
    FVector4 operator/(float scalar) const
    {
        return FVector4{ x / scalar, y / scalar, z / scalar, a / scalar };
    }
};
