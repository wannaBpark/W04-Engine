#pragma once
#include "PrimitiveComponent.h"
#include "Engine/Texture.h"
class USkySphereComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(USkySphereComponent, UPrimitiveComponent)

public:
    USkySphereComponent();
    virtual ~USkySphereComponent() override;

    virtual void		Initialize()	override;
    virtual void		TickComponent(float DeltaTime)		override;
    virtual void		Release()					override;
    virtual	void		Render()					override;
protected:
    std::shared_ptr<FTexture> Texture;
public:
    void SetTexture(const FWString& filename)
    {
        Texture = FEngineLoop::resourceMgr.GetTexture(filename);
    }

};

