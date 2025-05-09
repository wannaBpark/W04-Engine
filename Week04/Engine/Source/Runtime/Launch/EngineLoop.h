#pragma once
#include "Core/HAL/PlatformType.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Renderer/Renderer.h"
#include "Engine/ResourceMgr.h"

class UnrealEd;
class UImGuiManager;
class UWorld;
class FEditorViewportClient;
class SSplitterV;
class SSplitterH;
class SLevelEditor;
class UEditorEngine;

class FEngineLoop
{
public:
    FEngineLoop();

    int32 PreInit();
    int32 Init(HINSTANCE hInstance);
    void Render();
    void Tick();
    void Exit();
    float GetAspectRatio(IDXGISwapChain* swapChain) const;
    void Input();

private:
    void WindowInit(HINSTANCE hInstance);

public:
    static FGraphicsDevice graphicDevice;
    static FRenderer renderer;
    static FResourceMgr resourceMgr;


    HWND hWnd;

private:
    UImGuiManager* UIMgr;
    // TODO : GWorld 소유는 UEditorEngine소유로 한정?하면 UEditorEngine만 남겨야 함
    //UWorld* GWorld;
    SLevelEditor* LevelEditor;
    UnrealEd* UnrealEditor;
    bool bIsExit = false;
    const int32 targetFPS = 60;
    bool bTestInput = false;

public:
    UWorld* GetWorld() const;
    SLevelEditor* GetLevelEditor() const { return LevelEditor; }
    UnrealEd* GetUnrealEditor() const { return UnrealEditor; }
};
