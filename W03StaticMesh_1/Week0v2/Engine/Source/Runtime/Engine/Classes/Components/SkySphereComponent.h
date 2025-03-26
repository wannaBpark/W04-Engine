#pragma once
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture.h"
class USkySphereComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(USkySphereComponent, UStaticMeshComponent)

public:
    USkySphereComponent();
    virtual ~USkySphereComponent() override;

    virtual void		Initialize()	override;
    virtual void		Update(double deltaTime)		override;
    virtual void		Release()					override;
    virtual	void		Render()					override;
    float UOffset = 0;
    float VOffset = 0;
protected:
};

