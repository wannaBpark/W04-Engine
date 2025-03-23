#include "SWindow.h"

SWindow::SWindow()
{
}

SWindow::SWindow(FRect initRect) : Rect(initRect)
{
}

SWindow::~SWindow()
{
}

void SWindow::Initialize(FRect initRect)
{
    Rect = initRect;
}

void SWindow::OnResize(float width, float height)
{
    Rect.width = width;
    Rect.height = height;
}


bool SWindow::IsHover(FPoint coord) const
{
    return coord.x >= Rect.leftTopX && coord.x < Rect.leftTopX + Rect.width &&
        coord.y >= Rect.leftTopY && coord.y < Rect.leftTopY + Rect.height;
}
