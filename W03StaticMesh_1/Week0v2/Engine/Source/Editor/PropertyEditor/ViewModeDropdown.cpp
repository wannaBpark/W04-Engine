#include "Engine/Source/Runtime/Engine/World.h"
#include "ImGuiManager.h"
#include "EngineLoop.h"
#include "Engine/Source/Editor/PropertyEditor/ViewModeDropdown.h"
#include "UnrealEd/EditorViewportClient.h"

ViewModeDropdown& ViewModeDropdown::GetInstance()
{
	static ViewModeDropdown Inst;
	return Inst;
}
void ViewModeDropdown::Draw(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    float controllWindowWidth = static_cast<float>(width) * 0.05f;
    float controllWindowHeight = static_cast<float>(height) * 0.f;

    float controllWindowPosX = (static_cast<float>(width) - controllWindowWidth) * 0.40f;
    float controllWindowPosY = (static_cast<float>(height) - controllWindowHeight) * 0.f;
    ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
    ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

    if (ImGui::Begin("ViewMode"))
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

void ViewModeDropdown::OnResize(HWND hWnd)
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;
}
