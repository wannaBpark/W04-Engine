#pragma once
#include "UObject/Object.h"
class ActorComponent : public UObject
{
public:
    ActorComponent();
    virtual				~ActorComponent();

    virtual void		Initialize();
    virtual void		Update(double deltaTime);
    virtual void		Release();
    virtual void		Render();
};

