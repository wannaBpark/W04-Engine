#pragma once
#include "Engine/Source/Runtime/Engine/Classes/Components/SceneComponent.h"

class UGizmoArrowComponent;
class UGizmoCircleComponent;
class UGizmoRectangleComponent;

class UTransformGizmo : public USceneComponent
{
    DECLARE_CLASS(UTransformGizmo, USceneComponent)

    UTransformGizmo();
    virtual ~UTransformGizmo() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void Release() override;
    virtual void Render() override;

    TArray<UGizmoArrowComponent*>& GetArrowArr() { return ArrowArr; }
    TArray<UGizmoCircleComponent*>& GetDiscArr() { return CircleArr; }
    TArray<UGizmoRectangleComponent*>& GetScaleArr() { return RectangleArr; }

private:
    TArray<UGizmoArrowComponent*> ArrowArr;
    TArray<UGizmoCircleComponent*> CircleArr;
    TArray<UGizmoRectangleComponent*> RectangleArr;
};
