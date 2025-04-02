#include "LightComponent.h"
#include "UBillboardComponent.h"
#include "Math/JungleMath.h"
#include "UnrealEd/PrimitiveBatch.h"

ULightComponentBase::ULightComponentBase()
{
    // FString name = "SpotLight";
    // SetName(name);
    InitializeLight();
}

ULightComponentBase::~ULightComponentBase()
{
}
void ULightComponentBase::SetColor(FVector4 newColor)
{
    color = newColor;
}

FVector4 ULightComponentBase::GetColor() const
{
    return color;
}

float ULightComponentBase::GetRadius() const
{
    return radius;
}

void ULightComponentBase::SetRadius(float r)
{
    radius = r;
}

void ULightComponentBase::InitializeLight()
{
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    color = { 1,1,1,1 };
    radius = 5;
}


int ULightComponentBase::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res =AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

