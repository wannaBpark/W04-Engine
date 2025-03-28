#pragma once
#include "UnrealEd/EditorPanel.h"


class FFpsIndicator : public IEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;
};
