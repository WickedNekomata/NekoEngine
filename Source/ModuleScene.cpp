#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Primitive.h"
#include "ModuleMeshes.h"
#include "ModuleGOs.h"

ModuleScene::ModuleScene(bool start_enabled) : Module(start_enabled)
{
	name = "Scene";
}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Init(JSON_Object* jObject)
{
	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	//App->camera->Move(math::float3(1.0f, 1.0f, 0.0f));
	//App->camera->LookAt(math::float3(0, 0, 0));
	App->camera->Move(math::float3(0.0f, 1.0f, 5.0f));
	//App->camera->LookAt(math::float3(0.0f, 0.0f, 0.0f), math::float3(0.0f, 1.0f, 0.0f));

	grid = new PrimitiveGrid();
	grid->ShowAxis(true);

	root = App->GOs->CreateGameObject("Root", nullptr);

	return ret;
}

update_status ModuleScene::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_H) == KEY_DOWN)
	{
		child = App->GOs->CreateGameObject("a", root);
		currentGameObject = child;
	}

	if (App->input->GetKey(SDL_SCANCODE_J) == KEY_DOWN)
	{
		if (child != nullptr)
			child = App->GOs->CreateGameObject("b", child);
	}

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	bool ret = true;

	RELEASE(grid);

	return ret;
}

void ModuleScene::Draw() const 
{
	grid->Render();
}