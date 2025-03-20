#pragma once
#include "Components/PrimitiveComponent.h"
class UGizmoBaseComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UGizmoBaseComponent, UPrimitiveComponent)
public:
	virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
};

