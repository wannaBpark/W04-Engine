#pragma once
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture.h"


class USkySphereComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(USkySphereComponent, UStaticMeshComponent)

public:
    USkySphereComponent();
    virtual ~USkySphereComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

protected:
    std::shared_ptr<FTexture> Texture;

public:
    void SetTexture(const FWString& filename)
    {
        Texture = FEngineLoop::resourceMgr.GetTexture(filename);
    }
};
