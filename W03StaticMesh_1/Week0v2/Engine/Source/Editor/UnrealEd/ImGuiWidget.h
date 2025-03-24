#pragma once
#include "ImGUI/imgui_internal.h"

struct FImGuiWidget
{
    static bool VectorFloat3(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        bool value_changed = false;
        ImGui::BeginGroup();
        ImGui::PushID(label);
        ImGui::PushMultiItemsWidths(components, ImGui::CalcItemWidth());
        size_t type_size = GDataTypeInfo[data_type].Size;
        for (int i = 0; i < components; i++)
        {
            ImGui::PushID(i);
            if (i > 0)
                ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
            value_changed |= ImGui::DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags);
            ImGui::PopID();
            ImGui::PopItemWidth();
            p_data = (void*)((char*)p_data + type_size);
        }
        ImGui::PopID();

        const char* label_end = ImGui::FindRenderedTextEnd(label);
        if (label != label_end)
        {
            ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
            ImGui::TextEx(label, label_end);
        }

        ImGui::EndGroup();
        return value_changed;
    }
};
