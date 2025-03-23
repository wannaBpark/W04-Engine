#pragma once
#include "GameFramework/Actor.h"


class UGizmoArrowComponent;
class UGizmoCircleComponent;
class UGizmoRectangleComponent;

class UTransformGizmo : public AActor
{
    DECLARE_CLASS(UTransformGizmo, AActor)

    UTransformGizmo();

    virtual void Tick(float DeltaTime) override;
    virtual void Render() override;

    TArray<UGizmoArrowComponent*>& GetArrowArr() { return ArrowArr; }
    TArray<UGizmoCircleComponent*>& GetDiscArr() { return CircleArr; }
    TArray<UGizmoRectangleComponent*>& GetScaleArr() { return RectangleArr; }

private:
    TArray<UGizmoArrowComponent*> ArrowArr;
    TArray<UGizmoCircleComponent*> CircleArr;
    TArray<UGizmoRectangleComponent*> RectangleArr;
};
