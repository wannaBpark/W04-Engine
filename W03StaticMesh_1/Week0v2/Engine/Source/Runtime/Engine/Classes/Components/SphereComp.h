#pragma once
#include "StaticMeshComponent.h"
class USphereComp :public UStaticMeshComponent
{
    DECLARE_CLASS(USphereComp, UStaticMeshComponent)

public:
    USphereComp();
    virtual ~USphereComp() override;

    virtual void		Initialize()	override;
    virtual void		Update(double deltaTime)		override;
    virtual void		Release()					override;
    virtual	void		Render()					override;

};

