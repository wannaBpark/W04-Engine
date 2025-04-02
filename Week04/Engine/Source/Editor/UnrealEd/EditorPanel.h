#pragma once
#include "Core/HAL/PlatformType.h"


#ifndef __ICON_FONT_INDEX__

#define __ICON_FONT_INDEX__
#define DEFAULT_FONT		0
#define	FEATHER_FONT		1
#define AWESOME_FONT        2

#endif // !__ICON_FONT_INDEX__

class IEditorPanel
{
public:
    virtual ~IEditorPanel() = default;

    virtual void Render() = 0;
    virtual void OnResize(HWND hWnd) = 0;
};
