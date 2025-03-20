#pragma once
#include "Engine/Source/Runtime/Engine/Classes/Components/SceneComponent.h"

class UGizmoArrowComponent;
class UGizmoCircleComponent;
class UGizmoRectangleComponent;

class UTransformGizmo : public USceneComponent
{
    DECLARE_CLASS(UTransformGizmo, USceneComponent)

public:
    UTransformGizmo();
    virtual				~UTransformGizmo();

    virtual void		Initialize();
    virtual void		Update(double deltaTime);
    virtual void		Release();
    virtual void		Render();

    TArray<UGizmoArrowComponent*>& GetArrowArr() { return ArrowArr; }
    TArray<UGizmoCircleComponent*>& GetDiscArr() { return CircleArr; }
    TArray<UGizmoRectangleComponent*>& GetScaleArr() { return RectangleArr; }
private:
    TArray<UGizmoArrowComponent*> ArrowArr;
    TArray<UGizmoCircleComponent*> CircleArr;
    TArray<UGizmoRectangleComponent*> RectangleArr;
};

