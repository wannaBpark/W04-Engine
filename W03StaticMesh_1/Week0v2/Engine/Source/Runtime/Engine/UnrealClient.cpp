#include "UnrealClient.h"
#include "UnrealEd/EditorViewportClient.h"
FViewport::FViewport()
{
}



FViewport::~FViewport()
{
}

void FViewport::Initialize()
{
}

void FViewport::ResizeViewport(const DXGI_SWAP_CHAIN_DESC& swapchaindesc)
{
    //float width = (float)swapchaindesc.BufferDesc.Width;
    //float height = (float)swapchaindesc.BufferDesc.Height;
    //float halfWidth = width * 0.5f;
    //float halfHeight = height * 0.5f;
    //switch (viewLocation)
    //{
    //case EViewScreenLocation::TopLeft:
    //    viewport.TopLeftX = 0.0f;
    //    viewport.TopLeftY = 0.0f;
    //    viewport.Width = halfWidth;
    //    viewport.Height = halfHeight;
    //    break;
    //case EViewScreenLocation::TopRight:
    //    viewport.TopLeftX = halfWidth;
    //    viewport.TopLeftY = 0.0f;
    //    viewport.Width = halfWidth;
    //    viewport.Height = halfHeight;
    //    break;
    //case EViewScreenLocation::BottomLeft:
    //    viewport.TopLeftX = 0.0f;
    //    viewport.TopLeftY = halfHeight;
    //    viewport.Width = halfWidth;
    //    viewport.Height = halfHeight;
    //    break;
    //case EViewScreenLocation::BottomRight:
    //    viewport.TopLeftX = halfWidth;
    //    viewport.TopLeftY = halfHeight;
    //    viewport.Width = halfWidth;
    //    viewport.Height = halfHeight;
    //    break;
    //default:
    //    break;
    //}
    //viewport.MinDepth = 0.0f;
    //viewport.MaxDepth = 1.0f;
}

