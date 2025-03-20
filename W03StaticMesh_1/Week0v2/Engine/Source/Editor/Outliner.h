#pragma once
#include <Windows.h>
#include "PropertyEditor/IWindowToggleable.h"
class UWorld;
class Outliner : public IWindowToggleable
{
private:
    Outliner();

public:
    ~Outliner();

    static Outliner& GetInstance();

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
    bool bWasOpen = true;
    UINT width;
    UINT height;
};

