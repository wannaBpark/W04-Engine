#include "ViewportSettingPanel.h"

ViewportSettingPanel::ViewportSettingPanel()
{
}

ViewportSettingPanel::~ViewportSettingPanel()
{
}

ViewportSettingPanel& ViewportSettingPanel::GetInstance()
{
    // TODO: 여기에 return 문을 삽입합니다.
    static ViewportSettingPanel instance;
    return instance;
}


void ViewportSettingPanel::Draw(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    float controllWindowWidth = static_cast<float>(width) * 0.2f;
    float controllWindowHeight = static_cast<float>(height) * 0.f;

    float controllWindowPosX = (static_cast<float>(width) - controllWindowWidth) * 0.46f;
    float controllWindowPosY = (static_cast<float>(height) - controllWindowHeight) * 0.f;
    ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
    ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

    if (ImGui::Begin("Viewport Setting"))
    {
        const char* viewModeNames[] = { "Lit", "Unlit", "Wireframe" };

        if (ImGui::BeginCombo("##ViewModeCombo", viewModeNames[(int)ActiveViewport->GetViewMode()]))
        {
            for (int i = 0; i < IM_ARRAYSIZE(viewModeNames); i++)
            {
                bool isSelected = ((int)ActiveViewport->GetViewMode() == i);
                if (ImGui::Selectable(viewModeNames[i], isSelected))
                {
                    ActiveViewport->SetViewMode((EViewModeIndex)i);
                    FEngineLoop::graphicDevice.ChangeRasterizer(ActiveViewport->GetViewMode());
                    FEngineLoop::renderer.ChangeViewMode(ActiveViewport->GetViewMode());
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    ImGui::End();
}

void ViewportSettingPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;
}
