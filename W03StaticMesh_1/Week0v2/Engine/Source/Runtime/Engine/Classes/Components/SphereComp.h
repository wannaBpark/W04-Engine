#pragma once
#include "PrimitiveComponent.h"


class USphereComp : public UPrimitiveComponent
{
    DECLARE_CLASS(USphereComp, UPrimitiveComponent)

public:
    USphereComp();
    virtual ~USphereComp() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void Release() override;
    virtual void Render() override;
};
