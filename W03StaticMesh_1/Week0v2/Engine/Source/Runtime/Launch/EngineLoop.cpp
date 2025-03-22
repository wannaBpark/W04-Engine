#include "EngineLoop.h"
#include "ImGuiManager.h"
#include "World.h"
#include "Camera/CameraComponent.h"
#include "Math/JungleMath.h"
#include "PropertyEditor/ControlPanel.h"
#include "PropertyEditor/PropertyPanel.h"
#include "PropertyEditor/ViewModeDropdown.h"
#include "PropertyEditor/ShowFlags.h"
#include "Outliner.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealClient.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern FEngineLoop GEngineLoop;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}
	int zDelta = 0;
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) {
			//UGraphicsDevice 객체의 OnResize 함수 호출
			if (FEngineLoop::graphicDevice.SwapChain) {
				FEngineLoop::graphicDevice.OnResize(hWnd);
			}
            for (int i = 0;i < 4;i++) {
                if (GEngineLoop.GetViewports()[i]) {
                    GEngineLoop.GetViewports()[i]->ResizeViewport(FEngineLoop::graphicDevice.SwapchainDesc);
                }
            }
		}
		Console::GetInstance().OnResize(hWnd);
		ControlPanel::GetInstance().OnResize(hWnd);
		PropertyPanel::GetInstance().OnResize(hWnd);
		Outliner::GetInstance().OnResize(hWnd);
		ViewModeDropdown::GetInstance().OnResize(hWnd);
		ShowFlags::GetInstance().OnResize(hWnd);
		break;
	case WM_MOUSEWHEEL:
		zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // 휠 회전 값 (+120 / -120)
		if (GEngineLoop.GetWorld()->GetCamera()->IsCameraMode()) {
			GEngineLoop.GetCurViewportClient()->SetCameraSpeedScalar(static_cast<float>(GEngineLoop.GetCurViewportClient()->GetCameraSpeedScalar() + zDelta * 0.01));
		}
		else
		{
			GEngineLoop.GetWorld()->GetCamera()->MoveForward(zDelta*0.1f);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

FGraphicsDevice FEngineLoop::graphicDevice;
FRenderer FEngineLoop::renderer;
FResourceMgr FEngineLoop::resourceMgr;
uint32 FEngineLoop::TotalAllocationBytes= 0;
uint32 FEngineLoop::TotalAllocationCount = 0;
FEngineLoop::FEngineLoop()
{
}

FEngineLoop::~FEngineLoop()
{
}

int32 FEngineLoop::PreInit()
{
	return 0;
}

int32 FEngineLoop::Init(HINSTANCE hInstance)
{
	WindowInit(hInstance);
	graphicDevice.Initialize(hWnd);
	renderer.Initialize(&graphicDevice);

	
	
	UIMgr = new UImGuiManager;
	UIMgr->Initialize(hWnd,graphicDevice.Device, graphicDevice.DeviceContext);
	
	resourceMgr.Initialize(&renderer, &graphicDevice);
    for (size_t i = 0; i < 4; i++)
    {
        viewportClients[i] = std::make_shared<FEditorViewportClient>();
        viewportClients[i]->Initialize(i);
    }
    curViewportClient = viewportClients[0];

	GWorld = new UWorld;
	GWorld->Initialize();

	return 0;
}



void FEngineLoop::Tick()
{
	LARGE_INTEGER frequency;
	const double targetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER startTime, endTime;
	double elapsedTime = 1.0;

	while (bIsExit == false)
	{
		QueryPerformanceCounter(&startTime);

		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg); // 키보드 입력 메시지를 문자메시지로 변경
			DispatchMessage(&msg); // 메시지를 WndProc에 전달

			if (msg.message == WM_QUIT)
			{
				bIsExit = true;
				break;
			}
		}
        Input();
        curViewportClient->Tick(elapsedTime);

		GWorld->Tick(elapsedTime);

        graphicDevice.Prepare();
        std::shared_ptr<FEditorViewportClient> viewportClient = GEngineLoop.GetCurViewportClient();
        for(int i=0;i<4;++i)
        {
            SetViewportClient(i);
            graphicDevice.DeviceContext->RSSetViewports(1, &GetViewports()[i]->GetD3DViewport());
            renderer.PrepareShader();
            renderer.UpdateLightBuffer();
            Render();
        }
        curViewportClient = viewportClient;
		//graphicDevice.Prepare();
		//renderer.PrepareShader();
		//renderer.UpdateLightBuffer();
		////GWorld->Render();
		//Render();

		UIMgr->BeginFrame();

		Console::GetInstance().Draw();
		ControlPanel::GetInstance().Draw(GetWorld(),elapsedTime);
		PropertyPanel::GetInstance().Draw(GetWorld());
		Outliner::GetInstance().Draw(GetWorld());
		ShowFlags::GetInstance().Draw(GetWorld());
		ViewModeDropdown::GetInstance().Draw(GetWorld());
		UIMgr->EndFrame();

		GWorld->CleanUp();

		graphicDevice.SwapBuffer();
		do
		{
			Sleep(0);
			QueryPerformanceCounter(&endTime);
			elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
		} while (elapsedTime < targetFrameTime);
	}
}
float a = 5;
void FEngineLoop::Render()
{
	GWorld->Render();
	GWorld->RenderBaseObject();
	UPrimitiveBatch::GetInstance().RenderBatch(GetCurViewportClient()->GetViewMatrix(), GetCurViewportClient()->GetProjectionMatrix());
    //UPrimitiveBatch::GetInstance().RenderBatch(View, Projection);
}
float FEngineLoop::GetAspectRatio(IDXGISwapChain* swapChain)
{
	DXGI_SWAP_CHAIN_DESC desc;
	swapChain->GetDesc(&desc);
	return static_cast<float>(desc.BufferDesc.Width) / static_cast<float>(desc.BufferDesc.Height);
}

void FEngineLoop::Exit()
{
	GWorld->Release();
	delete GWorld;
	UIMgr->Shutdown();
	delete UIMgr;
	resourceMgr.Release(&renderer);
	renderer.Release();
	graphicDevice.Release();


}

void FEngineLoop::Input()
{
    if (GetAsyncKeyState('P') & 0x8000)
    {
        if (bPDown == false)
        {
            bPDown = true;
            AddViewportClient();
        }
    }
    else
    {
        bPDown = false;
    }
}

void FEngineLoop::WindowInit(HINSTANCE hInstance)
{
	WCHAR WindowClass[] = L"JungleWindowClass";

	WCHAR Title[] = L"Game Tech Lab";

	WNDCLASSW wndclass = { 0 };
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = WindowClass;

	RegisterClassW(&wndclass);

	hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1500, 1500,
		nullptr, nullptr, hInstance, nullptr);
}

