#pragma once

#include "IWindowToggleable.h"
#include <Windows.h>
class UWorld;
class ControlPanel : public IWindowToggleable
{
private:
	ControlPanel();

public:
	~ControlPanel();
	static ControlPanel& GetInstance()
	{
		static ControlPanel Inst;
		return Inst;
	}
	void Draw(UWorld* world, double elapsedTime);
	void OnResize(HWND hWnd);
	void Toggle() override {
		if (bWasOpen) {
			bWasOpen = false;
		} else {
			bWasOpen = true;
		}
	}

private:
	bool bWasOpen = true;
	UINT width;
	UINT height;
};

