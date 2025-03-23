#pragma once

#include <DirectXMath.h>

struct FVector2D
{
	float x,y;
	FVector2D(float _x = 0, float _y = 0) : x(_x), y(_y) {}

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

// 3D ����
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

	// ���� ����
	float Dot(const FVector& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	// ���� ũ��
	float Magnitude() const {
		return sqrt(x * x + y * y + z * z);
	}

	// ���� ����ȭ
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
	// ��Į�� ����
	FVector operator*(float scalar) const {
		return FVector(x * scalar, y * scalar, z * scalar);
	}

	bool operator==(const FVector& other) const {
		return (x == other.x && y == other.y && z == other.z);
	}

	float Distance(const FVector& other) const {
		// �� ������ �� ������ ũ�⸦ ���
		return ((*this - other).Magnitude());
	}
	DirectX::XMFLOAT3 ToXMFLOAT3() const
	{
		return DirectX::XMFLOAT3(x, y, z);
	}

};