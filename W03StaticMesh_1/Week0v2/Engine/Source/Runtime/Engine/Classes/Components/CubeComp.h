#pragma once
#include "PrimitiveComponent.h"

class UCubeComp : public UPrimitiveComponent
{
    DECLARE_CLASS(UCubeComp, UPrimitiveComponent)

public:
    UCubeComp();
    virtual ~UCubeComp() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void Render() override;
};
