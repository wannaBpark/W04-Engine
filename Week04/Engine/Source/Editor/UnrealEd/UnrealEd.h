#pragma once
#include "Container/Map.h"
#include "Container/String.h"

class IEditorPanel;

class UnrealEd
{
public:
    UnrealEd() = default;
    ~UnrealEd() = default;
    void Initialize();
    
     void Render() const;
     void OnResize(HWND hWnd) const;
    
    void AddEditorPanel(const FString& PanelId, const std::shared_ptr<IEditorPanel>& EditorPanel);
    std::shared_ptr<IEditorPanel> GetEditorPanel(const FString& PanelId);

private:
    TMap<FString, std::shared_ptr<IEditorPanel>> Panels;
};
