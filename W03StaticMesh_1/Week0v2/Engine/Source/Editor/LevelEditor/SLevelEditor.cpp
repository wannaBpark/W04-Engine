#pragma once
#include "SLevelEditor.h"
#include "SlateCore/Widgets/SWindow.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "UnrealClient.h"
#include "UnrealEd/EditorViewportClient.h"
#include "EngineLoop.h"

extern FEngineLoop GEngineLoop;

SLevelEditor::SLevelEditor() : bInitialize(false), HSplitter(nullptr), VSplitter(nullptr),
World(nullptr)
{
}

SLevelEditor::~SLevelEditor()
{
}

void SLevelEditor::Initialize()
{
    for (size_t i = 0; i < 4; i++)
    {
        viewportClients[i] = std::make_shared<FEditorViewportClient>();
        viewportClients[i]->Initialize(i);
    }
    ActiveViewportClient = viewportClients[0];
    OnResize();
    VSplitter = new SSplitterV();
    VSplitter->Initialize(FRect(0.0f, EditorHeight * 0.5f - 10, EditorHeight, 20));
    VSplitter->OnDrag(FPoint(0, 0));
    HSplitter = new SSplitterH();
    HSplitter->Initialize(FRect(EditorWidth * 0.5f - 10, 0.0f, 20, EditorWidth));
    HSplitter->OnDrag(FPoint(0, 0));
    ResizeViewports();
    bInitialize = true;
}

void SLevelEditor::Tick(double deltaTime)
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GEngineLoop.hWnd, &pt);
    if (VSplitter->IsHover(FPoint(pt.x, pt.y)) || HSplitter->IsHover(FPoint(pt.x, pt.y)))
    {
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    }
    else
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
    OnResize();
    //Test Code Cursor icon End
    Input();

    ActiveViewportClient->Tick(deltaTime);
}

void SLevelEditor::Input()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (bLButtonDown == false)
        {
            bLButtonDown = true;
            POINT pt;
            GetCursorPos(&pt);
            GetCursorPos(&lastMousePos);
            ScreenToClient(GEngineLoop.hWnd, &pt);

            SelectViewport(pt);

            VSplitter->OnPressed(FPoint(pt.x, pt.y));
            HSplitter->OnPressed(FPoint(pt.x, pt.y));
        }
        else
        {
            POINT currentMousePos;
            GetCursorPos(&currentMousePos);

            // 마우스 이동 차이 계산
            int32 deltaX = currentMousePos.x - lastMousePos.x;
            int32 deltaY = currentMousePos.y - lastMousePos.y;

            if (VSplitter->IsPressing())
            {
                VSplitter->OnDrag(FPoint(deltaX, deltaY));
            }
            if (HSplitter->IsPressing())
            {
                HSplitter->OnDrag(FPoint(deltaX, deltaY));
            }
            ResizeViewports();
            lastMousePos = currentMousePos;
        }
    }
    else
    {
        bLButtonDown = false;
        VSplitter->OnReleased();
        HSplitter->OnReleased();
    }
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    {
        if (!bRButtonDown)
        {
            bRButtonDown = true;
            POINT pt;
            GetCursorPos(&pt);
            GetCursorPos(&lastMousePos);
            ScreenToClient(GEngineLoop.hWnd, &pt);

            SelectViewport(pt);
        }
    }
    else
    {
        bRButtonDown = false;
    }
}

void SLevelEditor::Release()
{
    delete VSplitter;
    delete HSplitter;
}

void SLevelEditor::SelectViewport(POINT point)
{
    for (int i = 0; i < 4; i++)
    {
        if (viewportClients[i]->IsSelected(point))
        {
            SetViewportClient(i);
            break;
        }
    }
}

void SLevelEditor::OnResize()
{
    float PrevWidth = EditorWidth;
    float PrevHeight = EditorHeight;
    EditorWidth = GEngineLoop.graphicDevice.screenWidth;
    EditorHeight = GEngineLoop.graphicDevice.screenHeight;
    if (bInitialize) {
        //HSplitter 에는 바뀐 width 비율이 들어감 
        HSplitter->OnResize(EditorWidth/PrevWidth, EditorHeight);
        //HSplitter 에는 바뀐 Height 비율이 들어감 
        VSplitter->OnResize(EditorWidth, EditorHeight/PrevHeight);
        ResizeViewports();
    }
}

void SLevelEditor::ResizeViewports()
{
    if (GetViewports()[0]) {
        for (int i = 0;i < 4;++i)
        {
            GetViewports()[i]->ResizeViewport(VSplitter->SideLT->Rect, VSplitter->SideRB->Rect,
                HSplitter->SideLT->Rect, HSplitter->SideRB->Rect);
        }
    }
}
