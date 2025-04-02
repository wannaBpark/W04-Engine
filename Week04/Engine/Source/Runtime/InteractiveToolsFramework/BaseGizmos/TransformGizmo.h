#pragma once
#include "GameFramework/Actor.h"


class UStaticMeshComponent;
class ATransformGizmo : public AActor
{
    DECLARE_CLASS(ATransformGizmo, AActor)

public:
    ATransformGizmo();

    virtual void Tick(float DeltaTime) override;

    TArray<UStaticMeshComponent*>& GetArrowArr() { return ArrowArr; }
    TArray<UStaticMeshComponent*>& GetDiscArr() { return CircleArr; }
    TArray<UStaticMeshComponent*>& GetScaleArr() { return RectangleArr; }

private:
    TArray<UStaticMeshComponent*> ArrowArr;
    TArray<UStaticMeshComponent*> CircleArr;
    TArray<UStaticMeshComponent*> RectangleArr;
};
