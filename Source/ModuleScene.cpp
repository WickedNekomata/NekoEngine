#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Primitive.h"
#include "SceneImporter.h"
#include "ModuleGOs.h"
#include "GameObject.h"
#include "ComponentTransform.h"

ModuleScene::ModuleScene(bool start_enabled) : Module(start_enabled)
{
	name = "Scene";
}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Init(JSON_Object* jObject)
{
	CreateQuadtree();

	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	App->camera->Move(math::float3(0.0f, 1.0f, 5.0f));

	grid = new PrimitiveGrid();
	grid->ShowAxis(true);

	root = App->GOs->CreateGameObject("Root", nullptr);
	child = App->GOs->CreateGameObject("Api", root);
	GameObject* fillGuillem = App->GOs->CreateGameObject("fill de Api", child);
	App->GOs->CreateGameObject("net de Api1", fillGuillem);
	App->GOs->CreateGameObject("net de Api2", fillGuillem);
	child = App->GOs->CreateGameObject("Patata", root);
	fillGuillem = App->GOs->CreateGameObject("fill de Patata", child);

	App->GOs->CreateGameObject("net de Patata", fillGuillem);
	// Load Baker House last mesh
	std::string outputFile;
	App->sceneImporter->Import("cube.FBX", "Assets/Meshes/", outputFile);

	//Mesh* mesh = new Mesh();
	//App->sceneImporter->Load(outputFile.data(), mesh);

	// Load Baker House texture
	//App->materialImporter->Import("Baker_house.png", "Assets/", outputFile);
	//Texture* texture = new Texture();
	//App->materialImporter->Load(outputFile.data(), texture);
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
	grid->Render();
}

void ModuleScene::CreateQuadtree()
{
	const math::float3 center(0.0f, 10.0f, 0.0f);
	const math::float3 size(1000.0f, 20.0f, 1000.0f);
	math::AABB boundary;
	boundary.SetFromCenterAndSize(center, size);

	quadtree.SetBoundary(boundary);
}