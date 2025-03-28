#include "FpsIndicator.h"
#include "ImGUI/imgui.h"


void FFpsIndicator::Render()
{
    // 윈도우 위치 설정 (우상단)
    constexpr ImVec2 IndicatorLocation {
        5.0f,
        40.0f,
    };
    ImGui::SetNextWindowPos(IndicatorLocation, ImGuiCond_Always);

    // 상호작용 비활성화 플래그 설정
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    ImGui::Begin("FPS Indicator", nullptr, 
                 ImGuiWindowFlags_NoTitleBar
                 | ImGuiWindowFlags_NoResize
                 | ImGuiWindowFlags_NoMove
                 | ImGuiWindowFlags_NoInputs
                 | ImGuiWindowFlags_AlwaysAutoResize
                 | ImGuiWindowFlags_NoBackground
    );

    // FPS 정보 출력
    const float Fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f (%.1f ms)", Fps, 1000.0f / Fps);
    ImGui::PopStyleColor();
    ImGui::End();
}

void FFpsIndicator::OnResize(HWND hWnd)
{
}
