#include "ControlEditorPanel.h"
#include "tinyfiledialogs/tinyfiledialogs.h"

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
    
    CreateMenuButton(IconSize, IconFont);
    
    ImGui::SameLine();
    
    CreateFlagButton();
    
    ImGui::SameLine();

    CreateModifyButton(IconSize, IconFont);

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

void ControlEditorPanel::CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9ad", ButtonSize)) // Menu
    {
        bOpenMenu = !bOpenMenu;
    }
    ImGui::PopFont();
    
    if (bOpenMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 55), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(135, 170), ImGuiCond_Always);
        
        ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        if (ImGui::MenuItem("New Scene"))
        {
            
        }

        if (ImGui::MenuItem("Load Scene"))
        {
            
        }

        ImGui::Separator();
        
        if (ImGui::MenuItem("Save Scene"))
        {
            
        }

        ImGui::Separator();
        
        if (ImGui::BeginMenu("Import"))
        {
            if (ImGui::MenuItem("Wavefront (.obj)"))
            {
                char const * lFilterPatterns[1]={"*.obj"};
                const char* FileName =  tinyfd_openFileDialog("Open OBJ File", "", 1, lFilterPatterns,"Wavefront(.obj) file", 0);

                if (FileName != nullptr)
                {
                    std::cout << FileName << std::endl;
                }
            }
            
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit"))
        {
            ImGui::OpenPopup("프로그램 종료");   
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("프로그램 종료", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("정말 프로그램을 종료하시겠습니까?");
            ImGui::Separator();

            float ContentWidth = ImGui::GetWindowContentRegionMax().x;

            /* Move Cursor X Position */
            ImGui::SetCursorPosX(ContentWidth - (160.f + 10.0f));
            
            if (ImGui::Button("OK", ImVec2(80, 0))) { PostQuitMessage(0); }

            ImGui::SameLine();
            
            ImGui::SetItemDefaultFocus();
            ImGui::PushID("CancelButtonWithQuitWindow");
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            if (ImGui::Button("Cancel", ImVec2(80, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::EndPopup();
        }
        
        ImGui::End();
    }
}

void ControlEditorPanel::CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c4", ButtonSize)) // Slider
    {
        ImGui::OpenPopup("SliderControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("SliderControl"))
    {
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("Grid Scale", &GridScale, 0.1f, 0.1f, 100.0f, "%.1f"))
        {
            
        }

        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("Fov", &FOV, 0.1f, 0.0f, 120.0f, "%.1f"))
        {
            
        }
        ImGui::EndPopup();
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
