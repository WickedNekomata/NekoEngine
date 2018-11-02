#include "Application.h"
#include "Globals.h"
#include "ModuleGui.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Panel.h"
#include "PanelInspector.h"
#include "PanelAbout.h"
#include "PanelConsole.h"
#include "PanelSettings.h"
#include "PanelHierarchy.h"
#include "PanelAssets.h"
#include "ComponentCamera.h"
#include "ModuleTimeManager.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

ModuleGui::ModuleGui(bool start_enabled) : Module(start_enabled)
{
	name = "GUI";
}

ModuleGui::~ModuleGui()
{}

bool ModuleGui::Init(JSON_Object* jObject)
{
	panelInspector = new PanelInspector("Inspector");
	panelAbout = new PanelAbout("About");
	panelConsole = new PanelConsole("Console");
	panelSettings = new PanelSettings("Settings");
	panelHierarchy = new PanelHierarchy("Hierarchy");
	panelAssets = new PanelAssets("Assets");

	panels.push_back(panelInspector);
	panels.push_back(panelAbout);
	panels.push_back(panelConsole);
	panels.push_back(panelSettings);
	panels.push_back(panelHierarchy);
	panels.push_back(panelAssets);

	LoadStatus(jObject);

	return true;
}

bool ModuleGui::Start()
{
	bool ret = true;

	CONSOLE_LOG("Starting ImGui");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.IniFilename = nullptr;
	io.ConfigResizeWindowsFromEdges = true;

	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer3D->context);	
	ImGui_ImplOpenGL3_Init();

	// Setup style
	ImGui::StyleColorsLight();

	std::string outputFileName;
	timeButtonTex = new Texture();
	App->materialImporter->Import("timeButton.png", "UI/", outputFileName);
	App->materialImporter->Load(outputFileName.data(), timeButtonTex);

	return ret;
}

update_status ModuleGui::PreUpdate(float dt) 
{
	// Start the frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();

	return UPDATE_CONTINUE;
}

update_status ModuleGui::Update(float dt)
{
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN) { panelInspector->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN) { panelSettings->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN) { panelConsole->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_H) == KEY_DOWN) { panelHierarchy->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN) { panelAssets->OnOff(); }
	
	// Begin dock space
	DockSpace();

	ImVec2 mainMenuBarSize(0.0f, 0.0f);

	if (ImGui::BeginMainMenuBar())
	{
		mainMenuBarSize = ImGui::GetWindowSize();

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open In Explorer")) { OpenInExplorer(); }
			if (ImGui::MenuItem("Save")) { 
				App->SaveState(); }
			if (ImGui::MenuItem("Load")) { App->LoadState(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Save Scene")) { showSaveScenePopUp = true; }
			if (ImGui::MenuItem("Load Scene")) { showLoadScenePopUp = true; }
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
				App->CloseApp();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Inspector", "CTRL+I")) { panelInspector->OnOff(); }
			if (ImGui::MenuItem("Settings", "CTRL+S")) { panelSettings->OnOff(); }
			if (ImGui::MenuItem("Console", "CTRL+C")) { panelConsole->OnOff(); }
			if (ImGui::MenuItem("Hierarchy", "CTRL+H")) { panelHierarchy->OnOff(); }
			if (ImGui::MenuItem("Assets", "CTRL+A")) { panelAssets->OnOff(); }

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Others"))
		{
			if (ImGui::MenuItem("Documentation")) { OpenInBrowser("https://github.com/WickedNekomata/NekoEngine"); }
			if (ImGui::MenuItem("Latest Release")) { OpenInBrowser("https://github.com/WickedNekomata/NekoEngine/releases"); }
			if (ImGui::MenuItem("Bug Report")) { OpenInBrowser("https://github.com/WickedNekomata/NekoEngine/issues"); }
			if (ImGui::MenuItem("About")) { panelAbout->OnOff(); }

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	for (uint i = 0; i < panels.size(); ++i)
	{
		if (panels[i]->IsEnabled())
			panels[i]->Draw();
	}

	if (showSaveScenePopUp)
	{
		ImGui::OpenPopup("Save Scene as");
		SaveScenePopUp();
	}

	if (showLoadScenePopUp)
	{
		ImGui::OpenPopup("Load Scene");
		LoadScenePopUp();
	}

	//ImGui::SetNextWindowPos({ 0, mainMenuBarSize.y });
	//ImGui::SetNextWindowSize({ mainMenuBarSize.x, mainMenuBarSize.y });
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	static bool open = true;
	if (ImGui::Begin("##subMenu", &open))
	{
		/*
		if (App->camera->IsPlay())
		{
			ImGui::PushID("play");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::Button("PLAY");

			if (ImGui::IsItemClicked(0))
				App->camera->SetPlay(false);

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			if (ImGui::Button("PLAY"))
				App->camera->SetPlay(true);
		}

		ImGui::SameLine();
		*/

		float timeButtonScale = 0.05f;
		ImVec2 timeButtonSize(timeButtonTex->width * 0.2f * timeButtonScale, timeButtonTex->height * 1.0f * timeButtonScale);
		
		// Play button
		if (App->IsPlay())
		{
			ImGui::PushID("play");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)timeButtonTex->id, timeButtonSize, ImVec2(0.4f, 0.0f), ImVec2(0.6f, 1.0f));

			if (ImGui::IsItemClicked(0))
				App->Play();

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			if (ImGui::ImageButton((ImTextureID)timeButtonTex->id, timeButtonSize, ImVec2(0.4f, 0.0f), ImVec2(0.6f, 1.0f)))
				App->Play();
		}
		ImGui::SameLine();

		// Pause button
		if (App->IsPause())
		{
			ImGui::PushID("pause");
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			ImGui::ImageButton((ImTextureID)timeButtonTex->id, timeButtonSize, ImVec2(0.0f, 0.0f), ImVec2(0.2f, 1.0f));

			if (ImGui::IsItemClicked(0))
				App->Pause();

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		else
		{
			if (ImGui::ImageButton((ImTextureID)timeButtonTex->id, timeButtonSize, ImVec2(0.0f, 0.0f), ImVec2(0.2f, 1.0f)))
				App->Pause();
		}
		ImGui::SameLine();

		// Step button
		ImGui::ImageButton((ImTextureID)timeButtonTex->id, timeButtonSize, ImVec2(0.6f, 0.0f), ImVec2(0.8f, 1.0f)); ImGui::SameLine();

		// Game time scale slider
		ImGui::PushItemWidth(100);
		float timeScale = App->timeManager->GetTimeScale();
		if (ImGui::SliderFloat("Game Time Scale", &timeScale, 0.0f, MAX_TIME_SCALE)) { App->timeManager->SetTimeScale(timeScale); }

		// -----

		bool showGrid = App->scene->GetShowGrid();
		if (ImGui::Checkbox("Grid", &showGrid)) { App->scene->SetShowGrid(showGrid); }

		ImGui::SameLine();

		bool wireframeMode = App->renderer3D->IsWireframeMode();
		if (ImGui::Checkbox("Wireframe", &wireframeMode)) { App->renderer3D->SetWireframeMode(wireframeMode); }

		ImGui::SameLine();

		bool debugDraw = App->renderer3D->GetDebugDraw();
		if (ImGui::Checkbox("Debug Draw", &debugDraw)) { App->renderer3D->SetDebugDraw(debugDraw); }

		if (debugDraw)
		{
			ImGui::SameLine();

			bool drawBoundingBoxes = App->renderer3D->GetDrawBoundingBoxes();
			if (ImGui::Checkbox("Bounding Boxes", &drawBoundingBoxes)) { App->renderer3D->SetDrawBoundingBoxes(drawBoundingBoxes); }

			ImGui::SameLine();

			bool drawCamerasFrustum = App->renderer3D->GetDrawCamerasFrustum();
			if (ImGui::Checkbox("Cameras Frustum", &drawCamerasFrustum)) { App->renderer3D->SetDrawCamerasFrustum(drawCamerasFrustum); }

			ImGui::SameLine();

			bool drawQuadtree = App->renderer3D->GetDrawQuadtree();
			if (ImGui::Checkbox("Quadtree", &drawQuadtree)) { App->renderer3D->SetDrawQuadtree(drawQuadtree); }
		}
	}
	ImGui::End();

	// End dock space
	ImGui::End();

	return UPDATE_CONTINUE;
}

update_status ModuleGui::PostUpdate(float dt) 
{
	return UPDATE_CONTINUE;
}

bool ModuleGui::CleanUp()
{
	bool ret = true;

	for (uint i = 0; i < panels.size(); ++i)
	{
		if (panels[i] != nullptr)
			delete panels[i];
	}

	panelInspector = nullptr;
	panelAbout = nullptr;
	panelConsole = nullptr;
	panelSettings = nullptr;
	panelHierarchy = nullptr;
	panelAssets = nullptr;

	RELEASE(timeButtonTex);

	CONSOLE_LOG("Cleaning up ImGui");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return ret;
}

void ModuleGui::Draw() const 
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void ModuleGui::DockSpace() const
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	static bool p_open = true;
	ImGui::Begin("DockSpace Demo", &p_open, window_flags);
	ImGui::PopStyleVar(3);
	
	if (ImGui::DockBuilderGetNode(ImGui::GetID("MyDockspace")) == NULL)
	{		
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
		ImGui::DockBuilderAddNode(dockspace_id, viewport->Size); // Add empty node

		ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
		ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.12f, NULL, &dock_main_id);
		ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);
		ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
		ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.30f, NULL, &dock_main_id);	

		ImGui::DockBuilderDockWindow("##subMenu", dock_id_up);
		ImGui::DockBuilderDockWindow(panelHierarchy->GetName(), dock_id_left);
		ImGui::DockBuilderDockWindow(panelInspector->GetName(), dock_id_right);
		ImGui::DockBuilderDockWindow(panelAssets->GetName(), dock_id_bottom);
		ImGui::DockBuilderDockWindow(panelConsole->GetName(), dock_id_bottom);

		ImGui::DockBuilderFinish(dockspace_id);
	}

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruDockspace;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
}

void ModuleGui::SaveScenePopUp()
{
	if (ImGui::BeginPopupModal("Save Scene as", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Current scene will be save to the next directory:");
		ImGui::Separator();

		ImGui::Text("Assets/Scenes/");

		ImGui::PushItemWidth(100.0f);
		ImGui::InputText("##sceneName", App->GOs->nameScene, DEFAULT_BUF_SIZE);
		
		if (ImGui::Button("Save", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup(); App->GOs->MarkSceneToSerialize();
			showSaveScenePopUp = false;
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); showSaveScenePopUp = false; }
		ImGui::EndPopup();
	}
}

void ModuleGui::LoadScenePopUp()
{
	if (ImGui::BeginPopupModal("Load Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("The scene will be searched in the following directory:");
		ImGui::Separator();

		ImGui::Text("Assets/Scenes/");

		static char sceneToLoad[INPUT_BUF_SIZE];
		ImGuiInputTextFlags inputFlag = ImGuiInputTextFlags_EnterReturnsTrue;
		ImGui::PushItemWidth(100.0f);
		ImGui::InputText("##sceneName", sceneToLoad, IM_ARRAYSIZE(sceneToLoad), inputFlag);	

		if (ImGui::Button("Load", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			App->GOs->ClearScene();
			App->GOs->LoadScene(sceneToLoad);
			showLoadScenePopUp = false;
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); showLoadScenePopUp = false; }
		ImGui::EndPopup();
	}
}

void ModuleGui::SaveStatus(JSON_Object* jObject) const
{
	for (int i = 0; i < panels.size(); ++i)
		json_object_set_boolean(jObject, panels[i]->GetName(), panels[i]->IsEnabled());	
}

void ModuleGui::LoadStatus(const JSON_Object* jObject)
{
	for (int i = 0; i < panels.size(); ++i)
		panels[i]->SetOnOff(json_object_get_boolean(jObject, panels[i]->GetName()));
}

void ModuleGui::LogConsole(const char* log) const
{
	if (panelConsole != nullptr)
		panelConsole->AddLog(log);
}

void ModuleGui::AddInput(uint key, uint state) const
{
	static char input[512];
	static const char* states[] = { "IDLE", "DOWN", "REPEAT", "UP" };

	if (panelSettings != nullptr)
	{
		if (key < 1000)
			sprintf_s(input, 512, "Keybr: %02u - %s\n", key, states[state]);
		else
			sprintf_s(input, 512, "Mouse: %02u - %s\n", key - 1000, states[state]);
		panelSettings->AddInput(input);
	}
}

bool ModuleGui::IsMouseHoveringAnyWindow()
{
	return ImGui::IsMouseHoveringAnyWindow() || ImGui::IsAnyItemHovered();
}

bool ModuleGui::IsAnyItemFocused()
{
	return ImGui::IsAnyItemFocused();
}
