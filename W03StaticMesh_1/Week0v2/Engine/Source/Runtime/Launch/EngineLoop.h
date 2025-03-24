#pragma once
#include "Define.h"
#include "Core/HAL/PlatformType.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Renderer/Renderer.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "Engine/ResourceMgr.h"

class UnrealEd;
class UImGuiManager;
class UWorld;
class FEditorViewportClient;
class SSplitterV;
class SSplitterH;
class SLevelEditor;
class FEngineLoop
{
public:
    FEngineLoop();
    ~FEngineLoop();

    int32 PreInit();
    int32 Init(HINSTANCE hInstance);
    void Tick();
    void Exit();
    float GetAspectRatio(IDXGISwapChain* swapChain);

private:
    void WindowInit(HINSTANCE hInstance);
    void Render();
public:
    static FGraphicsDevice graphicDevice;
    static FRenderer renderer;
    static FResourceMgr resourceMgr;
    static uint32 TotalAllocationBytes;
    static uint32 TotalAllocationCount;


    HWND hWnd;

private:
    UImGuiManager* UIMgr;
    UWorld* GWorld;
    SLevelEditor* LevelEditor;
    UnrealEd* UnrealEditor;
    bool bIsExit = false;
    const int32 targetFPS = 60;
    int32 curViewportIndex = 0;

public:
    UWorld* GetWorld(){ return GWorld; }
    SLevelEditor* GetLevelEditor() { return LevelEditor; }
    UnrealEd* GetUnrealEditor() { return UnrealEditor; }
};

