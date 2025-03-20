#pragma once
#include "GizmoBaseComponent.h"
class UGizmoRectangleComponent : public UGizmoBaseComponent
{
	DECLARE_CLASS(UGizmoRectangleComponent, UGizmoBaseComponent)

public:
	UGizmoRectangleComponent();
	~UGizmoRectangleComponent();

	virtual void		Initialize()				override;
	virtual void		Update(double deltaTime)	override;
	virtual void		Release()					override;
	virtual	void		Render()					override;
};

