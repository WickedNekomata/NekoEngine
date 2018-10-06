#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Primitive.h"
#include "ModuleAssetImporter.h"

ModuleScene::ModuleScene(bool start_enabled) : Module(start_enabled)
{
	name = "Scene";
}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Init(JSON_Object * jObject)
{
	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	App->camera->Move(math::float3(1.0f, 1.0f, 0.0f));
	App->camera->LookAt(math::float3(0, 0, 0));

	plane = new PrimitivePlane();
	plane->SetPosition(math::float3(0.0f, -10.0f, 0.0f));

	cube = new PrimitiveCube();
	cube->ShowAxis(true);

	ray = new PrimitiveRay();	
	circle = new PrimitiveCircle();
	sphere = new PrimitiveSphere();
	frustum = new PrimitiveFrustum();
	cylinder = new PrimitiveCylinder();
	cylinder->ShowAxis(true);

	cone = new PrimitiveCone();
	arrow = new PrimitiveArrow();

	grid = new PrimitiveGrid();

	// Load model
	/*
	char* buffer;
	uint size;
	App->filesystem->OpenRead("Assets/BakerHouse.fbx", &buffer, size);
	App->assetsImporter->LoadFBXfromMemory(buffer, size);
	delete[] buffer;
	*/
	//App->assetsImporter->LoadFBXwithPHYSFS("Assets/BakerHouse.fbx");

	return ret;
}

update_status ModuleScene::Update(float dt)
{
	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	bool ret = true;

	RELEASE(cube);
	RELEASE(ray);
	RELEASE(plane);
	RELEASE(circle);
	RELEASE(frustum);
	RELEASE(sphere);
	RELEASE(cylinder);
	RELEASE(cone);

	RELEASE(grid);

	return ret;
}

void ModuleScene::Draw() const 
{
	// Big plane as ground
	//plane->Render();

	// Primitives
	//cube->Render();
	circle->Render();
	//sphere->Render();
	//ray->Render();
	//frustum->Render();
	//cylinder->Render();
	//cone->Render();
	//arrow->Render();
	grid->Render();
}