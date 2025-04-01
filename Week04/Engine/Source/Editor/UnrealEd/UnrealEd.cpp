#include "UnrealEd.h"
#include "EditorPanel.h"

#include "PropertyEditor/ControlEditorPanel.h"
#include "PropertyEditor/FpsIndicator.h"
#include "PropertyEditor/OutlinerEditorPanel.h"
#include "PropertyEditor/PropertyEditorPanel.h"

void UnrealEd::Initialize()
{
    const auto ControlPanel = std::make_shared<ControlEditorPanel>();
    Panels["ControlPanel"] = ControlPanel;
    //
    //const auto OutlinerPanel = std::make_shared<OutlinerEditorPanel>();
    //Panels["OutlinerPanel"] = OutlinerPanel;
    //
    //const auto PropertyPanel = std::make_shared<PropertyEditorPanel>();
    //Panels["PropertyPanel"] = PropertyPanel;

    const auto FPSIndicator = std::make_shared<FFpsIndicator>();
    Panels["FPSIndicator"] = FPSIndicator;
    
}

void UnrealEd::Render() const
{
    for (const auto& Panel : Panels)
    {
        Panel.Value->Render();
    }
}

void UnrealEd::AddEditorPanel(const FString& PanelId, const std::shared_ptr<IEditorPanel>& EditorPanel)
{
    Panels[PanelId] = EditorPanel;
}

void UnrealEd::OnResize(HWND hWnd) const
{
    for (auto& Panel : Panels)
    {
        Panel.Value->OnResize(hWnd);
    }
}

std::shared_ptr<IEditorPanel> UnrealEd::GetEditorPanel(const FString& PanelId)
{
    return Panels[PanelId];
}
