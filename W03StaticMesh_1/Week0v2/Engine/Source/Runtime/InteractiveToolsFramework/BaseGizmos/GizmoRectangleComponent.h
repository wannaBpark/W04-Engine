#pragma once
#include "GizmoBaseComponent.h"
class UGizmoRectangleComponent : public UGizmoBaseComponent
{
    DECLARE_CLASS(UGizmoRectangleComponent, UGizmoBaseComponent)

public:
    UGizmoRectangleComponent();
    virtual ~UGizmoRectangleComponent() override;

    virtual void		Initialize()				override;
    virtual void		TickComponent(float DeltaTime)	override;
    virtual void		Release()					override;
    virtual	void		Render()					override;
};

