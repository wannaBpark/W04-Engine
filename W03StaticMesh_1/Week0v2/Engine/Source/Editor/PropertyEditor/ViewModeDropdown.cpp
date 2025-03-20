#include "Engine/Source/Runtime/Engine/World.h"
#include "ImGuiManager.h"
#include "EngineLoop.h"
#include "Engine/Source/Editor/PropertyEditor/ViewModeDropdown.h"

ViewModeDropdown& ViewModeDropdown::GetInstance()
{
	static ViewModeDropdown Inst;
	return Inst;
}
void ViewModeDropdown::Draw(UWorld* world)
{
	float controllWindowWidth = static_cast<float>(width) * 0.07f;
	float controllWindowHeight = static_cast<float>(height) * 0.f;

	float controllWindowPosX = (static_cast<float>(width) - controllWindowWidth) * 0.3225f;
	float controllWindowPosY = (static_cast<float>(height) - controllWindowHeight) * 0.f;

	ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
	ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

	if (ImGui::Begin("View Mode"))
	{
		const char* viewModeNames[] = { "Lit", "Unlit", "Wireframe"};

		if (ImGui::BeginCombo("##ViewModeCombo", viewModeNames[(int)currentViewMode]))
		{
			for (int i = 0; i < IM_ARRAYSIZE(viewModeNames); i++)
			{
				bool isSelected = ((int)currentViewMode == i);
				if (ImGui::Selectable(viewModeNames[i], isSelected))
				{
					currentViewMode = (EViewModeIndex)i;
					FEngineLoop::graphicDevice.ChangeRasterizer(currentViewMode);
					FEngineLoop::renderer.ChangeViewMode(currentViewMode);
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
