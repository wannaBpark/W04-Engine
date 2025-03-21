#pragma once
#include "EngineBaseTypes.h"
#include "IWindowToggleable.h"
class UWorld;
class ViewModeDropdown : public IWindowToggleable
{
public:
    static ViewModeDropdown& GetInstance();

    void Draw(UWorld* world);
    void OnResize(HWND hWnd);
    void Toggle() override {
        if (bWasOpen) {
            bWasOpen = false;
        }
        else {
            bWasOpen = true;
        }
    }
private:
    EViewModeIndex currentViewMode;

    bool bWasOpen = true;
    UINT width;
    UINT height;
};



