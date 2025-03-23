#pragma once
#include "Define.h"
#include "Core/HAL/PlatformType.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Renderer/Renderer.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "Engine/ResourceMgr.h"
//#include "Engine/Source/Editor/UnrealEd/EditorViewportClient.h"

class UImGuiManager;
class UWorld;
class FEditorViewportClient;
class SSplitterV;
class SSplitterH;
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
    void SetViewportClient(std::shared_ptr<FEditorViewportClient> viewportClient)
    {
        curViewportClient = viewportClient;
    }
    void SetViewportClient(int index)
    {
        curViewportClient = viewportClients[index];
    }
    void Input();
private:
    void WindowInit(HINSTANCE hInstance);
    void Render();
    void SelectViewport(POINT point);
public:
    static FGraphicsDevice graphicDevice;
    static FRenderer renderer;
    static FResourceMgr resourceMgr;
    static uint32 TotalAllocationBytes;
    static uint32 TotalAllocationCount;

    bool bPDown = false;
    bool bLRButtonDown = false;
    POINT lastMousePos;

    HWND hWnd;

private:
    UImGuiManager* UIMgr;
    UWorld* GWorld;
    bool bIsExit = false;
    const int32 targetFPS = 60;
    int32 curViewportIndex = 0;
    std::shared_ptr<FEditorViewportClient> viewportClients[4];
    std::shared_ptr<FEditorViewportClient> curViewportClient;
public:
    UWorld* GetWorld(){ return GWorld; }
    std::shared_ptr<FEditorViewportClient>* GetViewports() { return viewportClients; }
    std::shared_ptr<FEditorViewportClient> GetCurViewportClient() const
    {
        return curViewportClient;
    }
    // test variable and function
private:
    SSplitterV* VSplitter;
    SSplitterH* HSplitter;
};

