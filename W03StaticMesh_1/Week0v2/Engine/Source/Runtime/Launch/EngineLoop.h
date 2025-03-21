#pragma once
#include "Define.h"
#include <Windows.h>
#include "D3D11RHI/GraphicDevice.h"
#include "Renderer/Renderer.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "Engine/ResourceMgr.h"
//#include "Engine/Source/Editor/UnrealEd/EditorViewportClient.h"
class UImGuiManager;
class UWorld;
class FEditorViewportClient;
class UnrealEd;

class FEngineLoop
{
public:
    FEngineLoop();
    ~FEngineLoop();

    int32 PreInit();
    int32 Init(HINSTANCE hInstance);
    void Tick();
    void Exit();
	
private:
    void WindowInit(HINSTANCE hInstance);
    void Render();
    float GetAspectRatio(IDXGISwapChain* swapChain);
public:
    static FGraphicsDevice graphicDevice;
    static FRenderer renderer;
    static FResourceMgr resourceMgr;
    static uint32 TotalAllocationBytes;
    static uint32 TotalAllocationCount;

	
    HWND hWnd;
    FMatrix View;
    FMatrix Projection;
private:
    UImGuiManager* UIMgr;
    UnrealEd* UnrealEditor;
    UWorld* GWorld;
    bool bIsExit = false;
    const int32 targetFPS = 60;

    std::shared_ptr<FEditorViewportClient> viewportClient;
public:
    UWorld* GetWorld(){ return GWorld; }
    UnrealEd* GetUnrealEditor() const { return UnrealEditor; }
    std::shared_ptr<FEditorViewportClient> GetViewportClient() const
    {
        return viewportClient;
    }
};

