#pragma once

#include <DirectXMath.h>

// 3D º¤ÅÍ
struct FVector
{
	float x, y, z;
	FVector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

	FVector operator-(const FVector& other) const {
		return FVector(x - other.x, y - other.y, z - other.z);
	}
	FVector operator+(const FVector& other) const {
		return FVector(x + other.x, y + other.y, z + other.z);
	}

	// º¤ÅÍ ³»Àû
	float Dot(const FVector& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	// º¤ÅÍ Å©±â
	float Magnitude() const {
		return sqrt(x * x + y * y + z * z);
	}

	// º¤ÅÍ Á¤±ÔÈ­
	FVector Normalize() const {
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
	// ½ºÄ®¶ó °ö¼À
	FVector operator*(float scalar) const {
		return FVector(x * scalar, y * scalar, z * scalar);
	}

	bool operator==(const FVector& other) const {
		return (x == other.x && y == other.y && z == other.z);
	}

	float Distance(const FVector& other) const {
		// µÎ º¤ÅÍÀÇ Â÷ º¤ÅÍÀÇ Å©±â¸¦ °è»ê
		return ((*this - other).Magnitude());
	}
	DirectX::XMFLOAT3 ToXMFLOAT3() const
	{
		return DirectX::XMFLOAT3(x, y, z);
	}

};