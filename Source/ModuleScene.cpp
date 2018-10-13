#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Primitive.h"
#include "ModuleMeshImporter.h"

ModuleScene::ModuleScene(bool start_enabled) : Module(start_enabled)
{
	name = "Scene";
}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Init(JSON_Object* jObject)
{
	showGrid = json_object_get_boolean(jObject, "showGrid");

	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	//App->camera->Move(math::float3(1.0f, 1.0f, 0.0f));
	//App->camera->LookAt(math::float3(0, 0, 0));
	App->camera->Move(math::float3(0.0f, 1.0f, -20.0f));

	grid = new PrimitiveGrid();
	grid->ShowAxis(true);

	return ret;
}

update_status ModuleScene::Update(float dt)
{
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
	if (showGrid)
		grid->Render();
}

bool ModuleScene::GetShowGrid() const
{
	return showGrid;
}

void ModuleScene::SetShowGrid(bool showGrid)
{
	this->showGrid = showGrid;
}