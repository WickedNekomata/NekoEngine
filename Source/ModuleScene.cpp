#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Primitive.h"
#include "SceneImporter.h"
#include "ModuleGOs.h"
#include "GameObject.h"
#include "DebugDrawer.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentMesh.h"

#include "imgui/imgui.h"

#include <list>

ModuleScene::ModuleScene(bool start_enabled) : Module(start_enabled)
{
	name = "Scene";
	game = true;
}

ModuleScene::~ModuleScene() {}

bool ModuleScene::Init(JSON_Object* jObject)
{
	CreateQuadtree();

	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	grid = new PrimitiveGrid();
	grid->ShowAxis(true);
	root = new GameObject("Root", nullptr);
	//child = App->GOs->CreateGameObject("Api", root);
	//GameObject* fillGuillem = App->GOs->CreateGameObject("fill de Api", child);
	//App->GOs->CreateGameObject("net de Api1", fillGuillem);
	//App->GOs->CreateGameObject("net de Api2", fillGuillem);
	//child = App->GOs->CreateGameObject("Patata", root);
	//fillGuillem = App->GOs->CreateGameObject("fill de Patata", child);

	//App->GOs->CreateGameObject("net de Patata", fillGuillem);
	// Load Baker House last mesh
#ifndef GAMEMODE
	std::string outputFile;
	App->sceneImporter->Import("BakerHouse.fbx", "Assets/Meshes/", outputFile);
#else
	App->GOs->LoadScene("GameReady");
	App->renderer3D->RecalculateMainCamera();
	App->renderer3D->SetMainCameraAsCurrentCamera();
	App->renderer3D->OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());
#endif// !GAME

	//Mesh* mesh = new Mesh();
	//App->sceneImporter->Load(outputFile.data(), mesh);

	// Load Baker House texture
	//App->materialImporter->Import("Baker_house.png", "Assets/", outputFile);
	//Texture* texture = new Texture();
	//App->materialImporter->Load(outputFile.data(), texture);


	return ret;
}

update_status ModuleScene::Update()
{
	//ImGuizmo::Enable(true);
	if (currentGameObject != nullptr)
		OnCurrentGameObjectGizmos();

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

void ModuleScene::OnCurrentGameObjectGizmos() const
{
	ImGuizmo::Enable(true);
	ImGuiIO& io = ImGui::GetIO();


	//, projMatrix, ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, matrix);

	//ImGuizmo::SetRect(0,0, ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->Size.y);

	ImGuizmo::Enable(true);
	if (ImGuizmo::IsUsing())
	{
		switch (currentImGuizmoOperation)
		{
		case ImGuizmo::OPERATION::BOUNDS:
			break;
		case ImGuizmo::OPERATION::TRANSLATE:
			break;
		case ImGuizmo::OPERATION::ROTATE:
			break;
		case ImGuizmo::OPERATION::SCALE:
			break;
		}
	}
	ImGuizmo::SetRect(ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y, ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->Size.y);

	ImGuizmo::Manipulate(
		App->camera->camera->GetOpenGLViewMatrix(), App->renderer3D->GetCurrentCamera()->GetOpenGLProjectionMatrix(),
		currentImGuizmoOperation, currentImGuizmoMode, currentGameObject->transform->GetGlobalMatrix().Transposed().ptr()
	);

	ImGuizmo::Manipulate(math::float4x4::identity.ptr(), App->renderer3D->GetCurrentCamera()->GetOpenGLProjectionMatrix(), ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, currentGameObject->transform->GetGlobalMatrix().Transposed().ptr());

	//ImGuizmo::DrawCube(App->renderer3D->GetCurrentCamera()->GetOpenGLViewMatrix(), App->renderer3D->GetCurrentCamera()->GetOpenGLProjectionMatrix(), currentGameObject->transform->GetGlobalMatrix().Transposed().ptr());
}

void ModuleScene::SetImGuizmoOperation(ImGuizmo::OPERATION operation)
{
	currentImGuizmoOperation = operation;
}

ImGuizmo::OPERATION ModuleScene::GetImGuizmoOperation() const
{
	return currentImGuizmoOperation;
}

void ModuleScene::SetImGuizmoMode(ImGuizmo::MODE mode)
{
	currentImGuizmoMode = mode;
}

ImGuizmo::MODE ModuleScene::GetImGuizmoMode() const
{
	return currentImGuizmoMode;
}

bool ModuleScene::GetShowGrid() const
{
	return showGrid;
}

void ModuleScene::SetShowGrid(bool showGrid)
{
	this->showGrid = showGrid;
}

void ModuleScene::RecreateQuadtree()
{
	// Clear and create the quadtree
	CreateQuadtree();

	// Fill the quadtree with static game objects
	App->GOs->RecalculateQuadtree();
}

void ModuleScene::CreateQuadtree()
{
	const math::float3 center(0.0f, 5.0f, 0.0f);
	const math::float3 size(100.0f, 10.0f, 100.0f);
	math::AABB boundary;
	boundary.SetFromCenterAndSize(center, size);

	quadtree.SetBoundary(boundary);
}

void ModuleScene::CreateRandomStaticGameObject()
{
	GameObject* random = App->GOs->CreateGameObject("Random", root);
	random->transform->position = math::float3(rand() % (50 + 50 + 1) - 50, rand() % 10, rand() % (50 + 50 + 1) - 50);

	const math::float3 center(random->transform->position.x, random->transform->position.y, random->transform->position.z);
	const math::float3 size(2.0f, 2.0f, 2.0f);
	random->boundingBox.SetFromCenterAndSize(center, size);

	quadtree.Insert(random);
}