#include "PropertyEditor/PropertyPanel.h"
#include "World.h"
#include "ImGuiManager.h"
#include "UObject/Object.h"
#include "Actors/Player.h"
#include "Components/PrimitiveComponent.h"
#include "Components/LightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/UText.h"
#include "Math/MathUtility.h"
#include "UObject/Casts.h"


PropertyPanel::PropertyPanel()
{
}

PropertyPanel::~PropertyPanel()
{
}

void PropertyPanel::Draw(UWorld* world)
{
	float controllWindowWidth = static_cast<float>(width) * 0.178f;
	float controllWindowHeight = static_cast<float>(height) * 0.43f;

	float controllWindowPosX = (static_cast<float>(width) - controllWindowWidth) * 0.f;
	float controllWindowPosY = (static_cast<float>(height) - controllWindowHeight) *1.f;

	// â ũ��� ��ġ ����
	ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
	ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

	ImGui::Begin("Jungle Property Panel");

	UPlayer* player = static_cast<UPlayer*>(world->GetPlayer());	
	std::string buttonLabel;

	if(player->GetControlMode() == ControlMode::CM_TRANSLATION)
	  buttonLabel = "Translation";
	else if (player->GetControlMode() == ControlMode::CM_ROTATION)
		buttonLabel = "Rotation";
	else if (player->GetControlMode() == ControlMode::CM_SCALE)
		buttonLabel = "Scale";

	std::string coordiButtonLabel;

	if (player->GetCoordiMode() == CoordiMode::CDM_WORLD)
		coordiButtonLabel = "World";
	else if (player->GetCoordiMode() == CoordiMode::CDM_LOCAL)
		coordiButtonLabel = "Local";
	// ��ư ������ �����ϰ�

	ImVec2 buttonSize(120, 30);

	ImGui::SetNextItemWidth(buttonSize.x); // ��ư �ʺ� ����
	if (ImGui::Button(buttonLabel.c_str(), buttonSize)) {
		player->AddControlMode(); // ���� �ڵ� ����
	}
	ImGui::SameLine();
	if (ImGui::Button(coordiButtonLabel.c_str(), buttonSize)) {
		player->AddCoordiMode();
	}

    AActor* PickActor = world->GetPickedActor();
    if (PickActor)
    {
        ImGui::Text("%s", *PickActor->GetName());

        // ��ġ/ȸ��/�������� float[3]�� ��Ƶ�
        FVector PickedActorLocation = PickActor->GetActorLocation();
        FVector PickedActorRotation = PickActor->GetActorRotation();
        FVector PickedActorScale = PickActor->GetActorScale();

        // ---------- ��ġ (X/Y/Z) ----------
        ImGui::Text("Position");
        ImGui::PushItemWidth(50.0f);
        ImGui::DragFloat("##posX", &PickedActorLocation.x, 0.6f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine();
        ImGui::DragFloat("##posY", &PickedActorLocation.y, 0.6f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine();
        ImGui::DragFloat("##posZ", &PickedActorLocation.z, 0.6f, -FLT_MAX, FLT_MAX);

        // ---------- ȸ�� (X/Y/Z) ----------
        ImGui::Text("Rotation");
        ImGui::DragFloat("##rotX", &PickedActorRotation.x, 0.6f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine();
        ImGui::DragFloat("##rotY", &PickedActorRotation.y, 0.6f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine();
        ImGui::DragFloat("##rotZ", &PickedActorRotation.z, 0.6f, -FLT_MAX, FLT_MAX);

        // ---------- ������ (X/Y/Z) ----------
        ImGui::Text("Scale");
        ImGui::DragFloat("##scaleX", &PickedActorScale.x, 0.6f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine();
        ImGui::DragFloat("##scaleY", &PickedActorScale.y, 0.6f, -FLT_MAX, FLT_MAX);
        ImGui::SameLine();
        ImGui::DragFloat("##scaleZ", &PickedActorScale.z, 0.6f, -FLT_MAX, FLT_MAX);
        ImGui::PopItemWidth();

        // ����� �� ����
        PickActor->SetActorLocation(PickedActorLocation);
        PickActor->SetActorRotation(PickedActorRotation);
        PickActor->SetActorScale(PickedActorScale);
        bool reclaimFocus = false;

        // -------- SpotLight ó���� ----------
        if (ULightComponentBase* lightObj = Cast<ULightComponentBase>(PickActor))
        {
            FVector4 currColor = lightObj->GetColor();

            float r = currColor.x;
            float g = currColor.y;
            float b = currColor.z;
            float a = currColor.a;
            float h, s, v;
            float lightColor[4] = { r, g, b, a };

            // �÷� ��Ŀ (ColorPicker4)
            if (ImGui::ColorPicker4("SpotLight Color", lightColor,
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
            // RGB/HSV �����̴� ����ȭ
            bool changedRGB = false;
            bool changedHSV = false;

            // RGB �����̴�
            ImGui::PushItemWidth(50.0f);
            if (ImGui::DragFloat("R##R", &r, 0.001f, 0.f, 1.f)) changedRGB = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("G##G", &g, 0.001f, 0.f, 1.f)) changedRGB = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("B##B", &b, 0.001f, 0.f, 1.f)) changedRGB = true;

            // HSV �����̴�
            if (ImGui::DragFloat("H##H", &h, 0.1f, 0.f, 360)) changedHSV = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("S##S", &s, 0.001f, 0.f, 1)) changedHSV = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("V##V", &v, 0.001f, 0.f, 1)) changedHSV = true;
            ImGui::PopItemWidth();

            if (changedRGB && !changedHSV)
            {
                // RGB�� ���� -> HSV ����ȭ
                RGBToHSV(r, g, b, h, s, v);
                lightObj->SetColor(FVector4(r, g, b, a));
            }
            else if (changedHSV && !changedRGB)
            {
                // HSV�� ���� -> RGB ����ȭ
                HSVToRGB(h, s, v, r, g, b);
                lightObj->SetColor(FVector4(r, g, b, a));
            }

            // ����Ʈ �ݰ�(Radius)
            float radiusVal = lightObj->GetRadius();
            if (ImGui::SliderFloat("Radius", &radiusVal, 1.0f, 100.0f))
            {
                lightObj->SetRadius(radiusVal);
            }
        }
        if (UText* textOBj = Cast<UText>(PickActor))
        {
            if (textOBj) {
                textOBj->SetTexture(L"Assets/Texture/font.png");
                textOBj->SetRowColumnCount(106, 106);
                FWString wText = textOBj->GetText();
                int len = WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string u8Text(len, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, &u8Text[0], len, nullptr, nullptr);

                static char buf[256];
                strcpy_s(buf, u8Text.c_str());
               
                if (ImGui::InputText("����", buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    textOBj->ClearText();
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
                    FWString newWText(wlen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, buf, -1, &newWText[0], wlen);
                    textOBj->SetText(newWText.c_str());
                   
                    reclaimFocus = true;
                }
                if (reclaimFocus) {
                    ImGui::SetKeyboardFocusHere(1);
                }
            }
        }
    }

 	ImGui::End();
}
void PropertyPanel::RGBToHSV(float r, float g, float b, float& h, float& s, float& v)
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
void PropertyPanel::OnResize(HWND hWnd)
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;
	
}
