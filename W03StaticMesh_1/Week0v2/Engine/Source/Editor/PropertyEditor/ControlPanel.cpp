#include "ControlPanel.h"
#include "World.h"
#include "Camera/CameraComponent.h"
#include "UnrealEd/SceneMgr.h"
#include "Components/Player.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/EditorWindow.h"
#include "LevelEditor/SLevelEditor.h"
// #include "ImGUI\imgui.h"
//#include "Font\IconDefs.h"
//#include "Font/RawFonts.h"
extern FEngineLoop GEngineLoop;
ControlPanel::ControlPanel()
{
	
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::Draw(UWorld* world, double elapsedTime )
{	
	float controllWindowWidth = static_cast<float>(width) * 0.3f;
	float controllWindowHeight = static_cast<float>(height) * 0.4f;

	float controllWindowPosX = (static_cast<float>(width) - controllWindowWidth) * 0.f;
	float controllWindowPosY = (static_cast<float>(height) - controllWindowHeight) * 0.f;

	// 창 크기와 위치 설정
	ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
	ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);


	ImGui::Begin("Jungle Control Panel");
	ImGui::Text("Hello Jungle World!");
	double fps = 1000.0 / elapsedTime;
	ImGui::Text("FPS %.2f (%.2fms)", fps, elapsedTime);
	ImGui::Separator();
	static int32 primitiveType = 0;
	const char* primitives[] = { "Sphere", "Cube", "SpotLight","Particle","Text"};
	ImGui::Combo("Primitive", &primitiveType, primitives, IM_ARRAYSIZE(primitives));

	int SpawnCount = static_cast<int>(world->GetObjectArr().Num());
	ImGui::InputInt("Number of Spawn", &SpawnCount, 0, 0);
	if (ImGui::Button("Spawn"))
	{
		world->SpawnObject(static_cast<OBJECTS>(primitiveType));
	}

	ImGui::Separator();
	ImGuiIO& io = ImGui::GetIO();
	ImFont* UnicodeFont = io.Fonts->Fonts[FEATHER_FONT];

	ImVec2 ControlButtonSize = ImVec2(32, 32);
	ImGui::PushFont(UnicodeFont);
	ImVec4 ActiveColor = ImVec4(0, 0.5, 0, 0.6f);

	UPlayer* player = static_cast<UPlayer*>(world->GetPlayer());
	// 현재 모드 저장 변수 ( 이 변수는 지역 변수로만 사용한다)
	if (!player) return;
	static ControlMode selectedMode = CM_TRANSLATION;

	//PropertyPanel* propPanel = world->GetPropertyPanel(); // PropertyPanel 가져오기
	//bool isTranslationActive = (PrimaryGizmo && PrimaryGizmo->GetCurrentGizmo() == EGizmoType::Translation);
	//if (isTranslationActive)
	//	ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor); // 활성 상태 색상
	if (ImGui::Button("\ue9bc", ControlButtonSize))
	{
		selectedMode = CM_TRANSLATION;
		player->SetMode(CM_TRANSLATION); // 현재 모드를 TRANSLATION 으로 변경
	}
	
	ImGui::SameLine();

	if (ImGui::Button("\ue9d3", ControlButtonSize))
	{
		selectedMode = CM_ROTATION;
		player->SetMode(CM_ROTATION);

	}
	
	ImGui::SameLine();

	if (ImGui::Button("\ue9ab", ControlButtonSize))
	{
		selectedMode = CM_SCALE;
		player->SetMode(CM_SCALE);
	}

	ImGui::Separator();

	if (ImGui::Button("\ue9b7"))
	{
		Console::GetInstance().bWasOpen = !Console::GetInstance().bWasOpen;
	}
	
	ImGui::PopFont();

	ImGui::Separator();

	static char sceneName[64] = "Default";
	ImGui::InputText("Scene Name", sceneName, IM_ARRAYSIZE(sceneName));

	if (ImGui::Button("New scene")) {
		world->NewScene();
	}
	if (ImGui::Button("Save scene")) {

		FString SceneName(sceneName);
		SceneData SaveData = world->SaveData();
		FSceneMgr::SaveSceneToFile(SceneName, SaveData);
	}
	if (ImGui::Button("Load scene")) {
		FString SceneName(sceneName);
		FString LoadJsonData = FSceneMgr::LoadSceneFromFile(SceneName);
		SceneData LoadData = FSceneMgr::ParseSceneData(LoadJsonData);
		world->LoadData(LoadData);
	}
	ImGui::Separator();
	
	float sp = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetGridSize();
	ImGui::SliderFloat("Grid Spacing", &sp, 1.0f, 20.0f);
	UPrimitiveBatch::GetInstance().GenerateGrid(sp, 5000);
	GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(sp);
	
	sp = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar();
	ImGui::SliderFloat("Camera Speed", &sp, 0.198f, 192.0f);
	GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetCameraSpeedScalar(sp);

	ImGui::Separator();

	ImGui::Text("Orthogonal");
	ImGui::SliderFloat("FOV", &world->GetCamera()->GetFOV(), 30.0f, 120.0f);

	float cameraLocation[3] = { world->GetCamera()->GetWorldLocation().x, world->GetCamera()->GetWorldLocation().y, world->GetCamera()->GetWorldLocation().z };
	ImGui::InputFloat3("Camera Location", cameraLocation);

	float cameraRotation[3] = { world->GetCamera()->GetWorldRotation().x, world->GetCamera()->GetWorldRotation().y, world->GetCamera()->GetWorldRotation().z };
	ImGui::InputFloat3("Camera Rotation", cameraRotation);

	world->GetCamera()->SetLocation(FVector(cameraLocation[0], cameraLocation[1], cameraLocation[2]));
	world->GetCamera()->SetRotation(FVector(cameraRotation[0], cameraRotation[1], cameraRotation[2]));

	ImGui::End();
}

void ControlPanel::OnResize(HWND hWindow)
{
	RECT clientRect;
	GetClientRect(hWindow, &clientRect);
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;
}

