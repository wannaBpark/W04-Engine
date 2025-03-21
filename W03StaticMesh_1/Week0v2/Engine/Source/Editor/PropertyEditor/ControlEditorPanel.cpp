#include "ControlEditorPanel.h"

void ControlEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);
    
    float PanelWidth = (Width) * 0.8f;
    float PanelHeight = 45.0f;

    float PanelPosX = 1.0f;
    float PanelPosY = 1.0f;

    ImVec2 MinSize(300, 50);
    ImVec2 MaxSize(FLT_MAX, 50);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
    
    /* Render Start */
    ImGui::Begin("Control Panel", nullptr, PanelFlags);
    
    ImGui::PushFont(IconFont);
    CreateMenuButton(IconSize);
    ImGui::PopFont();
    
    ImGui::SameLine();
    
    CreateFlagButton();
    
    ImGui::SameLine();

    /* Get Window Content Region */
    float ContentWidth = ImGui::GetWindowContentRegionMax().x;

    /* Move Cursor X Position */
    ImGui::SetCursorPosX(ContentWidth - (IconSize.x * 3.0f + 16.0f));
    
    ImGui::PushFont(IconFont);
    CreateSRTButton(IconSize);
    ImGui::PopFont();
    
    ImGui::End();
}

void ControlEditorPanel::CreateMenuButton(ImVec2 ButtonSize) const
{
    if (ImGui::Button("\ue9ad", ButtonSize)) // Menu
    {
        
    }
}

void ControlEditorPanel::CreateFlagButton() const
{
    static bool toggleViewState = true;
    if (ImGui::Button(toggleViewState ? "Perspective" : "Orthogonal", ImVec2(120, 32)))
    {
        toggleViewState = !toggleViewState;
    }

    ImGui::SameLine();
    
    if (ImGui::Button("Lit", ImVec2(60, 32)))
    {
        ImGui::OpenPopup("LightControl");
    }
    
    if (ImGui::BeginPopup("LightControl"))
    {
        if (ImGui::Selectable("Lit"))
        {
            
        }

        if (ImGui::Selectable("UnLit"))
        {
            
        }

        if (ImGui::Selectable("WireFrame"))
        {
            
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    
    if (ImGui::Button("Show", ImVec2(60, 32)))
    {
        ImGui::OpenPopup("ShowControl");
    }

    if (ImGui::BeginPopup("ShowControl"))
    {
        const char* items[] = { "AABB", "Primitive","BillBoard","UUID"};
        static bool selected[IM_ARRAYSIZE(items)] = { true, true, true, true };  // 각 항목의 체크 상태 저장

        for (int i = 0; i < IM_ARRAYSIZE(items); i++)
        {
            ImGui::Checkbox(items[i], &selected[i]);
        }

        ImGui::EndPopup();
    }
}

void ControlEditorPanel::CreateSRTButton(ImVec2 ButtonSize) const
{
    if (ImGui::Button("\ue9bc", ButtonSize)) // Move
    {
        
    }
	
    ImGui::SameLine();

    if (ImGui::Button("\ue9d3", ButtonSize)) // Rotate
    {

    }
	
    ImGui::SameLine();

    if (ImGui::Button("\ue9ab", ButtonSize)) // Scale
    {
        
    }
}

void ControlEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
