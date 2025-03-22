#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UActorComponent : public UObject
{
    DECLARE_CLASS(UActorComponent, UObject)

public:
    UActorComponent();
    virtual ~UActorComponent() override;

    virtual void Initialize() override;
    virtual void Update(double deltaTime) override;
    virtual void Release() override;
    virtual void Render() override;
};
