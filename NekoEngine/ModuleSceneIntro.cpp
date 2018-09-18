#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"

#include "imgui/imgui.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_sdl.h"

#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	float f;
	char* buf;
	App->camera->Move(vec3(1.0f, 1.0f, 0.0f));
	App->camera->LookAt(vec3(0, 0, 0));
	//lul
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer3D->context);
	ImGui_ImplOpenGL2_Init();
	ImGui::StyleColorsDark();
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	
	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update
update_status ModuleSceneIntro::Update(float dt)
{
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();

	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
	{
		patata = !patata;
	}
	if (patata)
	{
		ImGui::ShowDemoWindow(&patata);
		ImGui::Begin("text");
		ImGui::Text("hola");
		if (ImGui::Button("close"))
		{
			return UPDATE_STOP;
		}
		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	
	return UPDATE_CONTINUE;
}

