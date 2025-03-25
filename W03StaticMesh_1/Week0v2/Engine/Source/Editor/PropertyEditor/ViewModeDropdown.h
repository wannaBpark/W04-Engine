#pragma once
#include "EngineBaseTypes.h"
#include "IWindowToggleable.h"
class UWorld;
class FEditorViewportClient;
class ViewModeDropdown : public IWindowToggleable
{
public:
    static ViewModeDropdown& GetInstance();

    void Draw(std::shared_ptr<FEditorViewportClient> ActiveViewport);
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



