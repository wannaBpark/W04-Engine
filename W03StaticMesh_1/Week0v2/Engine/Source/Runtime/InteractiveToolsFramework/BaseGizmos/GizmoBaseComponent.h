#pragma once
#include "Components/StaticMeshComponent.h"


class UGizmoBaseComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(UGizmoBaseComponent, UStaticMeshComponent)
public:
    enum GizmoType
    {
        ArrowX,
        ArrowY,
        ArrowZ,
        CircleX,
        CircleY,
        CircleZ,
        ScaleX,
        ScaleY,
        ScaleZ
    };
public:
    UGizmoBaseComponent() = default;
    virtual ~UGizmoBaseComponent();

    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

    virtual void Initialize() override;
    virtual void Update(double deltaTime) override;
    virtual void Release() override;
    virtual void Render() override;
private:
    GizmoType gizmoType;
public:
    GizmoType GetGizmoType() { return gizmoType; }
    void SetGizmoType(GizmoType _gizmoType) { gizmoType = _gizmoType; }
};
