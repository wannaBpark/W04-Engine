#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UActorComponent : public UObject
{
    DECLARE_CLASS(UActorComponent, UObject)

public:
    UActorComponent();
    virtual ~UActorComponent();

    virtual void Initialize();
    virtual void Update(double deltaTime);
    virtual void Release();
    virtual void Render();
};
