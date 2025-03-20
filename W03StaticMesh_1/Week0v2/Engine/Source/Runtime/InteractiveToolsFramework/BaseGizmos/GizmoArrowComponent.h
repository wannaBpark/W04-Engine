#pragma once
#include "GizmoBaseComponent.h"
class UGizmoArrowComponent : public UGizmoBaseComponent
{
    DECLARE_CLASS(UGizmoArrowComponent, UGizmoBaseComponent)

public:
    UGizmoArrowComponent();
    ~UGizmoArrowComponent();

    virtual void		Initialize()				override;
    virtual void		Update(double deltaTime)	override;
    virtual void		Release()					override;
    virtual	void		Render()					override;

private:
    ARROW_DIR Dir;
public:
    ARROW_DIR GetDir() {
        return Dir;
    };
    void SetDir(ARROW_DIR _Dir) { Dir = _Dir; }
};

