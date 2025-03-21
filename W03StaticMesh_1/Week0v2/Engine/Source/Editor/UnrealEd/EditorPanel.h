#pragma once

#ifndef __ICON_FONT_INDEX__

#define __ICON_FONT_INDEX__
#define DEFAULT_FONT		0
#define	FEATHER_FONT		1

#endif // !__ICON_FONT_INDEX__

#include "HAL/PlatformType.h"


class UEditorPanel
{
public:
    virtual ~UEditorWindow() = default;
    virtual void Render() = 0;
    virtual void OnResize(HWND hWnd) = 0;
};
