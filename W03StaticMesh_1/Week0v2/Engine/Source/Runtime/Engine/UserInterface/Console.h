#pragma once
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_internal.h"
#include "ImGUI/imgui_impl_dx11.h"
#include "ImGUI/imgui_impl_win32.h"
#include "Define.h"
#include "Container/Array.h"
#include "UnrealEd/EditorWindow.h"
#include "PropertyEditor/IWindowToggleable.h"

enum class LogLevel { Display, Warning, Error };

class Console : public IWindowToggleable
{
private:
    Console();
    ~Console();
public:
    static Console& GetInstance(); // 참조 반환으로 변경

    void Clear();
    void AddLog(LogLevel level, const char* fmt, ...);
    void Draw();
    void OnResize(HWND hWnd);
    void Toggle() override { 
        if (bWasOpen) {
            bWasOpen = false;
        }
        else {
            bWasOpen = true;
        }
    } // Toggle() 구현 

public:
    struct LogEntry {
        LogLevel level;
        FString message;
    };

    TArray<LogEntry> items;
    TArray<FString> history;
    int32 historyPos = -1;
    char inputBuf[256] = "";
    bool scrollToBottom = false;

    ImGuiTextFilter filter;  // 필터링을 위한 ImGuiTextFilter

    // 추가된 멤버 변수들
    bool showLogTemp = true;   // LogTemp 체크박스
    bool showWarning = true;   // Warning 체크박스
    bool showError = true;     // Error 체크박스

    bool bWasOpen = true;
    // 복사 방지
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;
    Console(Console&&) = delete;
    Console& operator=(Console&&) = delete;
private:
    bool bExpand = true;
    UINT width;
    UINT height;

};
