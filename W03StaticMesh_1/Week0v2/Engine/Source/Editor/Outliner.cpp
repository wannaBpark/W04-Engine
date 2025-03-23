#include "Editor/Outliner.h"
#include "World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

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

void Outliner::Draw(UWorld* World)
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

            // 오브젝트 리스트
            // for (int32 i = 0; i < World->GetObjectArr().Num();i++)
            // {
            //     if(!World->GetObjectArr()[i]->IsA(USceneComponent::StaticClass()))
            //         continue;
            //     // 선택 가능 항목 (Selectable)
            //     if (ImGui::Selectable(*World->GetObjectArr()[i]->GetName(), selected == i))
            //     {
            //         selected = i; // 선택된 아이템 업데이트
            //         World->SetPickingObj(World->GetObjectArr()[i]);
            //     }
            // }

            static int Selected = -1; // 선택된 항목 저장용 변수
            int Index = 0;
            for (AActor* Actor : World->GetActors())
            {
                if (ImGui::Selectable(*Actor->GetActorLabel(), Selected == Index))
                {
                    Selected = Index;
                    World->SetPickedActor(Actor);
                }
                ++Index;
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