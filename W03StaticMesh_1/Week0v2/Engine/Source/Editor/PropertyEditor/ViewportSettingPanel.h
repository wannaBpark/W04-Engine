#pragma once
#include "IWindowToggleable.h"
#include "Define.h"
#include "EngineBaseTypes.h"
#include "UnrealEd/EditorViewportClient.h"
class ViewportSettingPanel : public IWindowToggleable
{
private:
    ViewportSettingPanel();
public:
    ~ViewportSettingPanel();

    static ViewportSettingPanel& GetInstance();

    void Draw(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void OnResize(HWND hWnd);
    void Toggle() override {
        if (bWasOpen) {
            bWasOpen = false;
        }
    }

private:
    bool bWasOpen = true;
    UINT width;
    UINT height;
};

