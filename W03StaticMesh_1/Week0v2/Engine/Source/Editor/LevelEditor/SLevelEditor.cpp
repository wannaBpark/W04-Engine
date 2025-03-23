#pragma once
#include "SLevelEditor.h"
#include "SlateCore/Widgets/SWindow.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "UnrealClient.h"
#include "UnrealEd/EditorViewportClient.h"
#include "EngineLoop.h"

extern FEngineLoop GEngineLoop;

SLevelEditor::SLevelEditor()
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

    VSplitter = new SSplitterV();
    VSplitter->Initialize(FRect(0.0f, GEngineLoop.graphicDevice.screenHeight * 0.5f - 10, GEngineLoop.graphicDevice.screenWidth, 20));
    HSplitter = new SSplitterH();
    HSplitter->Initialize(FRect(GEngineLoop.graphicDevice.screenWidth * 0.5f - 10, 0.0f, 20, GEngineLoop.graphicDevice.screenHeight));

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
    VSplitter->OnResize(GEngineLoop.graphicDevice.screenWidth, GEngineLoop.graphicDevice.screenHeight);
    HSplitter->OnResize(GEngineLoop.graphicDevice.screenWidth, GEngineLoop.graphicDevice.screenHeight);
    //Test Code Cursor icon End
    Input();

    ActiveViewportClient->Tick(deltaTime);
}

void SLevelEditor::Input()
{
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000 || GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    {
        if (bLRButtonDown == false)
        {
            bLRButtonDown = true;
            POINT pt;
            GetCursorPos(&pt);
            GetCursorPos(&lastMousePos);
            ScreenToClient(GEngineLoop.hWnd, &pt);
            //UE_LOG(LogLevel::Error, TEXT("LButton Down %d %d"), pt.x, pt.y);

            SelectViewport(pt);
        }
        else
        {
            POINT currentMousePos;
            GetCursorPos(&currentMousePos);

            // 마우스 이동 차이 계산
            int32 deltaX = currentMousePos.x - lastMousePos.x;
            int32 deltaY = currentMousePos.y - lastMousePos.y;

            if (VSplitter->IsHover(FPoint(lastMousePos.x, lastMousePos.y)))
            {
                UE_LOG(LogLevel::Error, TEXT("VSplitter %f %f %f %f"), VSplitter->Rect.leftTopX
                    , VSplitter->Rect.leftTopY, VSplitter->Rect.leftTopX + VSplitter->Rect.width
                    , VSplitter->Rect.leftTopY + VSplitter->Rect.height);
                VSplitter->OnDrag(FPoint(deltaX, deltaY));
            }
            if (HSplitter->IsHover(FPoint(lastMousePos.x, lastMousePos.y)))
            {
                UE_LOG(LogLevel::Error, TEXT("HSplitter %f %f %f %f"), HSplitter->Rect.leftTopX
                    , HSplitter->Rect.leftTopY, HSplitter->Rect.leftTopX + HSplitter->Rect.width
                    , HSplitter->Rect.leftTopY + HSplitter->Rect.height);
                HSplitter->OnDrag(FPoint(deltaX, deltaY));
            }
            for (int i = 0;i < 4;++i)
            {
                GetViewports()[i]->ResizeViewport(VSplitter->SideLT->Rect, VSplitter->SideRB->Rect,
                    HSplitter->SideLT->Rect, HSplitter->SideRB->Rect);
                /*              GetViewports()[i]->ResizeViewport(FRect(0,0,1200,600), FRect(0,600,1200,600),
                                  FRect(0,0,600,1200), FRect(600,0,600,1200));*/
            }
            lastMousePos = currentMousePos;
        }
    }
    else
    {
        bLRButtonDown = false;
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
