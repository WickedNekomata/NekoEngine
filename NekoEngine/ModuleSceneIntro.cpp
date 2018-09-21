#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"

#include "Primitive.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_sdl.h"

#include <stdio.h>
#include <time.h>

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{}

ModuleSceneIntro::~ModuleSceneIntro()
{}

bool ModuleSceneIntro::Start()
{
	bool ret = true;

	float f;
	char* buf;

	App->camera->Move(vec3(1.0f, 1.0f, 0.0f));
	App->camera->LookAt(vec3(0, 0, 0));

	return ret;
}

update_status ModuleSceneIntro::Update(float dt)
{
	Plane p(0, 1, 0, 0);
	p.axis = true;
	p.Render();

	// Inputs
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN) { showInspector = !showInspector; }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_D) == KEY_DOWN) { showDemo = !showDemo; }
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT) && App->input->GetKey(SDL_SCANCODE_N) == KEY_DOWN) { showRandWindow = !showRandWindow; }

	// Gui
	ShowMenuBar();

	if (showDemo)
		ShowDemoWindow();

	if (showInspector)
		ShowInspectorWindow();

	if (showRandWindow)
		ShowRandWindow();

	return UPDATE_CONTINUE;
}

bool ModuleSceneIntro::CleanUp()
{
	bool ret = true;

	return ret;
}

void ModuleSceneIntro::ShowMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")) {}
			if (ImGui::MenuItem("Open")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
				App->CloseApp();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Inspector Window", "CTRL+I", showInspector)) { showInspector = !showInspector; }
			if (ImGui::MenuItem("Demo Window", "CTRL+D", showDemo)) { showDemo = !showDemo; }
			if (ImGui::MenuItem("Random Generator", "CTRL+N", showRandWindow)) { showRandWindow = !showRandWindow; }
			
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void ModuleSceneIntro::ShowDemoWindow() 
{
	ImGui::ShowDemoWindow();
}

void ModuleSceneIntro::ShowInspectorWindow()
{
	ImGui::SetNextWindowPos({ SCREEN_WIDTH - 400,20 });
	ImGui::SetNextWindowSize({ 400,400 });
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
	inspectorFlags |= ImGuiWindowFlags_NoResize;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	ImGui::Begin("Inspector", false, inspectorFlags);
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Transform"))
	{
		ImGui::Text("Position");
		ImGui::SameLine();
		static int posX = 10;
		static int posY = 10;
		static int posZ = 10;
		ImGui::PushItemWidth(100);
		ImGui::InputInt("##Line", &posX, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputInt("##Line", &posY, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputInt("##Line", &posZ, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("RigidBody"))
	{
		ImGui::Text("Coming soon...");
	}
	ImGui::End();
}

void ModuleSceneIntro::ShowRandWindow()
{
	ImGui::SetNextWindowSize({ 200,55 });
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoResize;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	inspectorFlags |= ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Random Number Gen", false, inspectorFlags);
	if (ImGui::Button("Generate")) { pcg32_srandom_r(&rng, time(NULL), (intptr_t)&rng); }
	ImGui::SameLine();
	ImGui::Text("number: %d", rng);
	ImGui::SameLine();
	ImGui::End();
}
