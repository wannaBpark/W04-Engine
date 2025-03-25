#include "PropertyEditorPanel.h"

#include "World.h"
#include "Components/LightComponent.h"
#include "Actors/Player.h"
#include "Components/SceneComponent.h"
#include "Components/UText.h"
#include "Math/MathUtility.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UObject/Casts.h"

void PropertyEditorPanel::Render()
{
    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.65f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = (Height) * 0.3f + 15.0f;

    ImVec2 MinSize(140, 370);
    ImVec2 MaxSize(FLT_MAX, 900);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    /* Render Start */
    ImGui::Begin("Detail", nullptr, PanelFlags);
    
    UPlayer* player = GEngineLoop.GetWorld()->GetPlayer();
    AActor* PickedActor = GEngineLoop.GetWorld()->GetPickedActor();
    if (PickedActor)
    {
        ImGui::SetItemDefaultFocus();
        // TreeNode 배경색을 변경 (기본 상태)
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            Location = PickedActor->GetActorLocation();
            Rotation = PickedActor->GetActorRotation();
            Scale = PickedActor->GetActorScale();
            
            FImGuiWidget::DrawVec3Control("Location", Location, 0, 85);
            ImGui::Spacing();

            FImGuiWidget::DrawVec3Control("Rotation", Rotation, 0, 85);
            ImGui::Spacing();

            FImGuiWidget::DrawVec3Control("Scale", Scale, 0, 85);
            ImGui::Spacing();

            PickedActor->SetActorLocation(Location);
            PickedActor->SetActorRotation(Rotation);
            PickedActor->SetActorScale(Scale);
            
            std::string coordiButtonLabel;
            if (player->GetCoordiMode() == CoordiMode::CDM_WORLD)
                coordiButtonLabel = "World";
            else if (player->GetCoordiMode() == CoordiMode::CDM_LOCAL)
                coordiButtonLabel = "Local";
            
            if (ImGui::Button(coordiButtonLabel.c_str(), ImVec2(ImGui::GetWindowContentRegionMax().x * 0.9f, 32)))
            {
                player->AddCoordiMode();
            }
            ImGui::TreePop(); // 트리 닫기
        }
        ImGui::PopStyleColor();
    }

    // TODO: 추후에 RTTI를 이용해서 프로퍼티 출력하기
    if (ULightComponentBase* lightObj = Cast<ULightComponentBase>(PickedActor))
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("SpotLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            FVector4 currColor = lightObj->GetColor();

            float r = currColor.x;
            float g = currColor.y;
            float b = currColor.z;
            float a = currColor.a;
            float h, s, v;
            float lightColor[4] = { r, g, b, a };

            // SpotLight Color
            if (ImGui::ColorPicker4("##SpotLight Color", lightColor,
                ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_NoSidePreview |
                ImGuiColorEditFlags_NoInputs |
                ImGuiColorEditFlags_Float))

            {

                r = lightColor[0];
                g = lightColor[1];
                b = lightColor[2];
                a = lightColor[3];
                lightObj->SetColor(FVector4(r, g, b, a));
            }
            RGBToHSV(r, g, b, h, s, v);
            // RGB/HSV
            bool changedRGB = false;
            bool changedHSV = false;

            // RGB
            ImGui::PushItemWidth(50.0f);
            if (ImGui::DragFloat("R##R", &r, 0.001f, 0.f, 1.f)) changedRGB = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("G##G", &g, 0.001f, 0.f, 1.f)) changedRGB = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("B##B", &b, 0.001f, 0.f, 1.f)) changedRGB = true;
            ImGui::Spacing();
            
            // HSV
            if (ImGui::DragFloat("H##H", &h, 0.1f, 0.f, 360)) changedHSV = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("S##S", &s, 0.001f, 0.f, 1)) changedHSV = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("V##V", &v, 0.001f, 0.f, 1)) changedHSV = true;
            ImGui::PopItemWidth();
            ImGui::Spacing();
            
            if (changedRGB && !changedHSV)
            {
                // RGB -> HSV
                RGBToHSV(r, g, b, h, s, v);
                lightObj->SetColor(FVector4(r, g, b, a));
            }
            else if (changedHSV && !changedRGB)
            {
                // HSV -> RGB
                HSVToRGB(h, s, v, r, g, b);
                lightObj->SetColor(FVector4(r, g, b, a));
            }

            // Light Radius
            float radiusVal = lightObj->GetRadius();
            if (ImGui::SliderFloat("Radius", &radiusVal, 1.0f, 100.0f))
            {
                lightObj->SetRadius(radiusVal);
            }
            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }

    // TODO: 추후에 RTTI를 이용해서 프로퍼티 출력하기
    if (UText* textOBj = Cast<UText>(PickedActor))
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Text Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            if (textOBj) {
                textOBj->SetTexture(L"Assets/Texture/font.png");
                textOBj->SetRowColumnCount(106, 106);
                FWString wText = textOBj->GetText();
                int len = WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string u8Text(len, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, u8Text.data(), len, nullptr, nullptr);

                static char buf[256];
                strcpy_s(buf, u8Text.c_str());

                ImGui::Text("Text: ", buf);
                ImGui::SameLine();
                ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                if (ImGui::InputText("##Text", buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    textOBj->ClearText();
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
                    FWString newWText(wlen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, buf, -1, newWText.data(), wlen);
                    textOBj->SetText(newWText);
                }
                ImGui::PopItemFlag();
            }
            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
    ImGui::End();
}

void PropertyEditorPanel::RGBToHSV(float r, float g, float b, float& h, float& s, float& v)
{
    float mx = FMath::Max(r, FMath::Max(g, b));
    float mn = FMath::Min(r, FMath::Min(g, b));
    float delta = mx - mn;

    v = mx;

    if (mx == 0.0f) {
        s = 0.0f;
        h = 0.0f;
        return;
    }
    else {
        s = delta / mx;
    }

    if (delta < 1e-6) {
        h = 0.0f;
    }
    else {
        if (r >= mx) {
            h = (g - b) / delta;
        }
        else if (g >= mx) {
            h = 2.0f + (b - r) / delta;
        }
        else {
            h = 4.0f + (r - g) / delta;
        }
        h *= 60.0f;
        if (h < 0.0f) {
            h += 360.0f;
        }
    }
}

void PropertyEditorPanel::HSVToRGB(float h, float s, float v, float& r, float& g, float& b)
{
    // h: 0~360, s:0~1, v:0~1
    float c = v * s;
    float hp = h / 60.0f;             // 0~6 구간
    float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
    float m = v - c;

    if (hp < 1.0f) { r = c;  g = x;  b = 0.0f; }
    else if (hp < 2.0f) { r = x;  g = c;  b = 0.0f; }
    else if (hp < 3.0f) { r = 0.0f; g = c;  b = x; }
    else if (hp < 4.0f) { r = 0.0f; g = x;  b = c; }
    else if (hp < 5.0f) { r = x;  g = 0.0f; b = c; }
    else { r = c;  g = 0.0f; b = x; }

    r += m;  g += m;  b += m;
}

void PropertyEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
