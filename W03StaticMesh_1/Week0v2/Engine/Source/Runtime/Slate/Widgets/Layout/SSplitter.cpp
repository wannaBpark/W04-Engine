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
    float prevHeight = Rect.height;
    Rect.height = height;
    float resize = height / prevHeight;
    if (SideLT)
    {
        SideLT->Rect.height *= resize;
    }
    if (SideRB)
    {
        SideRB->Rect.height *= resize;
    }
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
    float prevWidth = Rect.width;
    UE_LOG(LogLevel::Warning, "width %f height %f", width, height);
    Rect.width = width;
    float resize = width/prevWidth;

    if (SideLT)
    {
        SideLT->Rect.width *= resize;
    }
    if (SideRB)
    {
        SideRB->Rect.width *= resize;
    }
}
