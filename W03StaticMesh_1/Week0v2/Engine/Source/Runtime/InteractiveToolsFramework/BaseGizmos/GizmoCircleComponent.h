#pragma once
#include "GizmoBaseComponent.h"
#include "UObject/ObjectTypes.h"

class UGizmoCircleComponent : public UGizmoBaseComponent
{
    DECLARE_CLASS(UGizmoCircleComponent, UGizmoBaseComponent)

public:
    UGizmoCircleComponent(EPrimitiveColor color, float innerRadius = 0.95f, FString type = "Disc");
    UGizmoCircleComponent();
    virtual ~UGizmoCircleComponent() override;
    //virtual void Update();
    //virtual bool Intersects(const FVector& rayOrigin, const FVector& rayDir, float& dist);
    //virtual bool PickObjectByRayIntersection(const FVector& pickPosition, const FMatrix& viewMatrix, float* hitDistance) override;
    virtual bool IntersectsRay(const FVector& rayOrigin, const FVector& rayDir, float& dist);
    virtual void Render() override;

    float inner;
};
