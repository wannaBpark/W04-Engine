#include "Console.h"
#include <cstdarg>
#include <cstdio>


// 싱글톤 인스턴스 반환
Console& Console::GetInstance() {
    static Console instance;
    return instance;
}

// 생성자
Console::Console() {}

// 소멸자
Console::~Console() {}

// 로그 초기화
void Console::Clear() {
    items.clear();
}

// 로그 추가
void Console::AddLog(LogLevel level, const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    items.push_back({ level, std::string(buf) });
    scrollToBottom = true;
}


// 콘솔 창 렌더링
void Console::Draw() {
    if (!bWasOpen) return;
    // 창 크기 및 위치 계산
    float controllWindowWidth = static_cast<float>(width) * 0.5f;
    float expandedHeight = static_cast<float>(height) * 0.35f;
    float collapsedHeight = 25.0f; // 접었을 때 높이
    float controllWindowHeight = bExpand ? expandedHeight : collapsedHeight;

    float controllWindowPosX = (static_cast<float>(width) - controllWindowWidth);
    float controllWindowPosY = (static_cast<float>(height) - controllWindowHeight);

    // 창 크기와 위치 설정
    ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
    ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

    // 창을 표시하고 닫힘 여부 확인
    bExpand = ImGui::Begin("Console", &bWasOpen);
    if (!bExpand) {
        ImGui::End();
        return;
    }

    // 창을 접었을 경우 UI를 표시하지 않음
    if (!bExpand) {
        ImGui::End();
        return;
    }
    


    // 버튼 UI (로그 수준별 추가)
    if (ImGui::Button("Add LogTemp")) { AddLog(LogLevel::Display, "LogTemp message"); }
    ImGui::SameLine();
    if (ImGui::Button("Add Warning")) { AddLog(LogLevel::Warning, "Warning message"); }
    ImGui::SameLine();
    if (ImGui::Button("Add Error")) { AddLog(LogLevel::Error, "Error message"); }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) { Clear(); }
    ImGui::SameLine();
    if (ImGui::Button("Copy")) { ImGui::LogToClipboard(); }

    ImGui::Separator();

    // 필터 입력 창
    ImGui::Text("Filter:");
    ImGui::SameLine();

    filter.Draw("##Filter", 100);
    
    ImGui::SameLine();

    // 로그 수준을 선택할 체크박스
    ImGui::Checkbox("Show Display", &showLogTemp);
    ImGui::SameLine();
    ImGui::Checkbox("Show Warning", &showWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Show Error", &showError);

    ImGui::Separator();
    // 로그 출력 (필터 적용)
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : items) {
        if (!filter.PassFilter(entry.message.c_str())) continue;

        // 로그 수준에 맞는 필터링
        if ((entry.level == LogLevel::Display && !showLogTemp) ||
            (entry.level == LogLevel::Warning && !showWarning) ||
            (entry.level == LogLevel::Error && !showError)) {
            continue;
        }

        // 색상 지정
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        switch (entry.level) {
        case LogLevel::Display: color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;  // 기본 흰색
        case LogLevel::Warning: color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); break; // 노란색
        case LogLevel::Error:   color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break; // 빨간색
        }

        ImGui::TextColored(color, "%s", entry.message.c_str());
    }
    if (scrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        scrollToBottom = false;
    }
    ImGui::EndChild();

    ImGui::Separator();

    // 입력창
    bool reclaimFocus = false;
    if (ImGui::InputText("Input", inputBuf, IM_ARRAYSIZE(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (inputBuf[0]) {
            AddLog(LogLevel::Display, ">> %s", inputBuf);
            history.push_back(std::string(inputBuf));
            historyPos = -1;
            scrollToBottom = true; // 자동 스크롤
        }
        inputBuf[0] = '\0';
        reclaimFocus = true;
    }

    // 입력 필드에 자동 포커스
    if (reclaimFocus) {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}
void Console::OnResize(HWND hWindow)
{
    RECT clientRect;
    GetClientRect(hWindow, &clientRect);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;
}

