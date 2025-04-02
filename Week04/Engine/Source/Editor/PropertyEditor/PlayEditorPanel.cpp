#include "PlayEditorPanel.h"
#include "Font/IconDefs.h"

void PlayEditorPanel::Render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[AWESOME_FONT];
    ImVec2 IconSize = ImVec2(32, 32);

    float PanelWidth = (Width) * 0.8f;
    float PanelHeight = 45.0f;

    float PanelPosX = 525.0f;
    float PanelPosY = 1.0f;

    ImVec2 MinSize(300, 50);
    ImVec2 MaxSize(FLT_MAX, 50);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    /* Render Start */
    ImGui::Begin("Play Mode Panel", nullptr, PanelFlags);

    CreatePlayButton(IconSize, IconFont);

    ImGui::SameLine();

    CreatePauseButton(IconSize, IconFont);

    ImGui::SameLine();

    CreateStopButton(IconSize, IconFont);

    ImGui::SameLine();
    ImGui::End();
}

void PlayEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
void PlayEditorPanel::CreatePlayButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);

    // 아이콘 폰트 색상 (초록색)
    ImVec4 greenColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, greenColor);  // 아이콘 폰트 색상 지정

    if (ImGui::Button(ICON_FA_PLAY, ButtonSize)) {
        // PIEMODE 전환
    }

    ImGui::PopStyleColor();  // 텍스트 색상 복원
    ImGui::PopFont();
}

void PlayEditorPanel::CreateStopButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);

    // 아이콘 폰트 색상 (빨간색)
    ImVec4 redColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, redColor);  // 아이콘 폰트 색상 지정

    if (ImGui::Button(ICON_FA_STOP, ButtonSize)) {
        // PIE MODE -> Editor 모드로 전환
    }

    ImGui::PopStyleColor();  // 텍스트 색상 복원
    ImGui::PopFont();
}

void PlayEditorPanel::CreatePauseButton(ImVec2 ButtonSize, ImFont* IconFont) const
{
    ImGui::PushFont(IconFont);

    // 아이콘 폰트 색상 (회색)
    ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, grayColor);  // 아이콘 폰트 색상 지정

    if (ImGui::Button(ICON_FA_PAUSE, ButtonSize)) {
        // PIEMODE에서 일시 정지
    }

    ImGui::PopStyleColor();  // 텍스트 색상 복원
    ImGui::PopFont();
}