#pragma once
#include "StaticMeshComponent.h"

class UCubeComp : public UStaticMeshComponent
{
    DECLARE_CLASS(UCubeComp, UStaticMeshComponent)

public:
    UCubeComp();
    virtual ~UCubeComp() override;

    virtual void Initialize() override;
    virtual void Update(double deltaTime) override;
    virtual void Release() override;
    virtual void Render() override;
};
