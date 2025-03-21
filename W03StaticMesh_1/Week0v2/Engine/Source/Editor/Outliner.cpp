#include "Editor/Outliner.h"
#include "World.h"
#include "Components/PrimitiveComponent.h"

Outliner::Outliner()
{
}

Outliner::~Outliner()
{
}

Outliner& Outliner::GetInstance()
{
    static Outliner instance;
    return instance;
}

void Outliner::Draw(UWorld* world)
{
    float controllWindowWidth = static_cast<float>(width) * 0.178f;
    float controllWindowHeight = static_cast<float>(height) * 0.15f;

    float controllWindowPosX = (static_cast<float>(width) - controllWindowWidth) * 0.f;
    float controllWindowPosY = (static_cast<float>(height) - controllWindowHeight) * 0.47f;

    // 창 크기와 위치 설정
    ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
    ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

    if (ImGui::Begin("OutLiner")) // 윈도우 시작
    {
        if (ImGui::TreeNode("Primitives")) // 트리 노드 생성
        {
            static int selected = -1; // 선택된 항목 저장용 변수

            // 오브젝트 리스트
            for (int32 i = 0; i < world->GetObjectArr().Num();i++)
            {
                if(!world->GetObjectArr()[i]->IsA(USceneComponent::StaticClass()))
                    continue;
                // 선택 가능 항목 (Selectable)
                if (ImGui::Selectable(*world->GetObjectArr()[i]->GetName(), selected == i))
                {
                    selected = i; // 선택된 아이템 업데이트
                    world->SetPickingObj(world->GetObjectArr()[i]);
                }
            }
            ImGui::TreePop(); // 트리 닫기
        }
    }
    ImGui::End(); // 윈도우 종료
}

void Outliner::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;

}