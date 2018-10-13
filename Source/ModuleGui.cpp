#include "Application.h"
#include "Globals.h"
#include "ModuleGui.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Panel.h"
#include "PanelInspector.h"
#include "PanelTestPCG.h"
#include "PanelAbout.h"
#include "PanelConsole.h"
#include "PanelSettings.h"
#include "PanelImport.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl2.h"

ModuleGui::ModuleGui(bool start_enabled) : Module(start_enabled)
{
	name = "GUI";
}

ModuleGui::~ModuleGui()
{}

bool ModuleGui::Init(JSON_Object* jObject)
{
	panelInspector = new PanelInspector("Inspector");
	panelRandomNumber = new PanelTestPCG("PCG performance test");
	panelAbout = new PanelAbout("About");
	panelConsole = new PanelConsole("Console");
	panelSettings = new PanelSettings("Settings");
	panelImport = new PanelImport("Import");

	panels.push_back(panelInspector);
	panels.push_back(panelRandomNumber);
	panels.push_back(panelAbout);
	panels.push_back(panelConsole);
	panels.push_back(panelSettings);
	panels.push_back(panelImport);

	this->LoadStatus(jObject);

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

	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer3D->context);	
	ImGui_ImplOpenGL2_Init();

	// Setup style
	ImGui::StyleColorsCustom();
	
	return ret;
}

update_status ModuleGui::PreUpdate(float dt) 
{
	// Start the frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();

	return UPDATE_CONTINUE;
}

update_status ModuleGui::Update(float dt)
{
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN) { panelInspector->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN) { panelSettings->OnOff(); }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN) { panelConsole->OnOff(); }

	ImVec2 mainMenuBarSize(0.0f, 0.0f);

	if (ImGui::BeginMainMenuBar())
	{
		mainMenuBarSize = ImGui::GetWindowSize();

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")) {}
			if (ImGui::MenuItem("Open")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Save")) { App->SaveState(); }
			if (ImGui::MenuItem("Load")) { App->LoadState(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Import")) { panelImport->OnOff(); }
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

	for (int i = 0; i < panels.size(); ++i)
	{
		if (panels[i]->IsEnabled())
			panels[i]->Draw();
	}

	ImGui::SetNextWindowPos({ 0, mainMenuBarSize.y });
	ImGui::SetNextWindowSize({ mainMenuBarSize.x, mainMenuBarSize.y });
	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoFocusOnAppearing;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoScrollbar;
	flags |= ImGuiWindowFlags_NoScrollWithMouse;

	static bool open = true;
	if (ImGui::Begin("subMenu", &open, flags))
	{
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

		static bool showGrid = App->scene->GetShowGrid();
		if (ImGui::Checkbox("Grid", &showGrid)) { App->scene->SetShowGrid(showGrid); }

		ImGui::SameLine();

		static bool debugDraw = App->renderer3D->GetDebugDraw();
		if (ImGui::Checkbox("Debug Draw", &debugDraw)) { App->renderer3D->SetDebugDraw(debugDraw); }
	}
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

	for (int i = 0; i < panels.size(); ++i)
	{
		if (panels[i] != nullptr)
			delete panels[i];
	}

	panelInspector = nullptr;
	panelRandomNumber = nullptr;
	panelAbout = nullptr;
	panelConsole = nullptr;
	panelSettings = nullptr;
	panelImport = nullptr;

	CONSOLE_LOG("Cleaning up ImGui");

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return ret;
}

void ModuleGui::Draw() const 
{
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
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
