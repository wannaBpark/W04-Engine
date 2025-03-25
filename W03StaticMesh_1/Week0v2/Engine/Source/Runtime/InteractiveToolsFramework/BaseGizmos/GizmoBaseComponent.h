#pragma once
#include "Components/StaticMeshComponent.h"

class UGizmoBaseComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(UGizmoBaseComponent, UStaticMeshComponent)

public:
    UGizmoBaseComponent() = default;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
};
