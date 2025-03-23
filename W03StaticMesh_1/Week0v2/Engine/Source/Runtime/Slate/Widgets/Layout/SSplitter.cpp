#include "SSplitter.h"


void SSplitter::Initialize(FRect initRect)
{
    __super::Initialize(initRect);
    if (SideLT == nullptr)
    {
        SideLT = new SWindow();
    }
    if (SideRB == nullptr)
    {
        SideRB = new SWindow();
    }
}

void SSplitter::OnResize(float width, float height)
{
}

void SSplitterH::Initialize(FRect initRect)
{
    __super::Initialize(initRect);
    if (SideLT)
    {
        SideLT->Initialize(FRect(0.0f, 0.0f, initRect.leftTopX, initRect.height));
    }
    if (SideRB)
    {
        SideRB->Initialize(FRect(initRect.leftTopX + initRect.width, 0.0f, initRect.leftTopX, initRect.height));
    }
}

void SSplitterH::OnResize(float width, float height)
{
    Rect.height = height;
    Rect.leftTopX *= width;
    if (SideLT)
    {
        SideLT->Rect.height = height;
    }
    if (SideRB)
    {
        SideRB->Rect.leftTopX *= width;
        SideRB->Rect.width *= width;
        SideLT->Rect.height = height;

    }
    UpdateChildRects();
}

void SSplitterV::Initialize(FRect initRect)
{
    __super::Initialize(initRect);
    if (SideLT)
    {
        SideLT->Initialize(FRect(0.0f, 0.0f, initRect.width, initRect.leftTopY));
    }
    if (SideRB)
    {
        SideRB->Initialize(FRect(0.0f, initRect.leftTopY + initRect.height, initRect.width, initRect.leftTopY));
    }
}

void SSplitterV::OnResize(float width, float height)
{
    UE_LOG(LogLevel::Warning, "width %f height %f", width, height);
    Rect.width = width;
    Rect.leftTopY *= height;
    UE_LOG(LogLevel::Warning, "%f ", SideLT->Rect.width);
    if (SideLT)
    {
        SideLT->Rect.width = width;
    }
    if (SideRB)
    {
        SideRB->Rect.leftTopY *= height;
        SideRB->Rect.height *= height;
        SideRB->Rect.width = width;
    }
    UpdateChildRects();
}
