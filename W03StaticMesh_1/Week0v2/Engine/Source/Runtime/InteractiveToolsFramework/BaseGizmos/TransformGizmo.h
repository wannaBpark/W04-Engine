#pragma once
#include "Engine/Source/Runtime/Engine/Classes/Components/SceneComponent.h"

class UStaticMeshComponent;
class UTransformGizmo : public USceneComponent
{
    DECLARE_CLASS(UTransformGizmo, USceneComponent)

    UTransformGizmo();
    virtual ~UTransformGizmo() override;

    virtual void Initialize() override;
    virtual void Update(double deltaTime) override;
    virtual void Release() override;
    virtual void Render() override;

    TArray<UStaticMeshComponent*>& GetArrowArr() { return ArrowArr; }
    TArray<UStaticMeshComponent*>& GetDiscArr() { return CircleArr; }
    TArray<UStaticMeshComponent*>& GetScaleArr() { return RectangleArr; }

private:
    TArray<UStaticMeshComponent*> ArrowArr;
    TArray<UStaticMeshComponent*> CircleArr;
    TArray<UStaticMeshComponent*> RectangleArr;
};
