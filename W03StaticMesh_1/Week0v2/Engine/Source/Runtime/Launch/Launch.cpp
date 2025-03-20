#pragma once
#include <Windows.h>
#include "EngineLoop.h"

FEngineLoop GEngineLoop;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GEngineLoop.Init(hInstance);
    GEngineLoop.Tick();
    GEngineLoop.Exit();
    return 0;
}
