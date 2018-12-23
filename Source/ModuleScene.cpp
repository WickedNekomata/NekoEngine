#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "Primitive.h"
#include "SceneImporter.h"
#include "ModuleGOs.h"
#include "ModuleGui.h"
#include "GameObject.h"
#include "DebugDrawer.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentMesh.h"

#include "imgui/imgui.h"

#include <list>
#include <vector>

#define QUADTREE_SIZE_X 200.0f
#define QUADTREE_SIZE_Y 100.0f
#define QUADTREE_SIZE_Z 200.0f

ModuleScene::ModuleScene(bool start_enabled) : Module(start_enabled)
{
	name = "Scene";
}

ModuleScene::~ModuleScene() {}

bool ModuleScene::Init(JSON_Object* jObject)
{
	LoadStatus(jObject);

	CreateQuadtree();

	return true;
}

bool ModuleScene::Start()
{
	grid = new PrimitiveGrid();
	grid->ShowAxis(true);
	root = new GameObject("Root", nullptr);

#ifdef GAMEMODE
	App->GOs->LoadScene("Settings/GameReady.nekoScene");
	App->renderer3D->SetCurrentCamera();
	App->renderer3D->OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());
#endif
	//App->GOs->LoadScene("Settings/GameReady.nekoScene");

	return true;
}

update_status ModuleScene::Update()
{
#ifndef GAMEMODE
	if (!App->IsEditor())
		return UPDATE_CONTINUE;

	if (!App->gui->WantTextInput())
	{
		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_IDLE)
		{
			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
				SetImGuizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
			if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
				SetImGuizmoOperation(ImGuizmo::OPERATION::ROTATE);
			if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
				SetImGuizmoOperation(ImGuizmo::OPERATION::SCALE);
		}

		if (App->input->GetKey(SDL_SCANCODE_T) == KEY_DOWN)
		{
			if (currentImGuizmoMode == ImGuizmo::MODE::WORLD)
				SetImGuizmoMode(ImGuizmo::MODE::LOCAL);
			else
				SetImGuizmoMode(ImGuizmo::MODE::WORLD);
		}
	}

	if (selectedObject == CurrentSelection::SelectedType::gameObject)
	{
		GameObject* currentGameObject = (GameObject*)selectedObject.Get();
		OnGizmos(currentGameObject);
	}
#endif

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	bool ret = true;

	RELEASE(grid);

	SELECT(NULL);

	quadtree.Clear();

	return ret;
}

void ModuleScene::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_boolean(jObject, "showGrid", showGrid);
}

void ModuleScene::LoadStatus(const JSON_Object* jObject)
{
	showGrid = json_object_get_boolean(jObject, "showGrid");
}

void ModuleScene::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::RecreateQuadtree:
		RecreateQuadtree();
		break;
	}
}

void ModuleScene::Draw() const
{
	if (showGrid)
		grid->Render();
}

#ifndef GAMEMODE
void ModuleScene::OnGizmos(GameObject* gameObject) const
{
	ImGuiViewport* vport = ImGui::GetMainViewport();
	ImGuizmo::SetRect(vport->Pos.x, vport->Pos.y, vport->Size.x, vport->Size.y);

	math::float4x4 viewMatrix = App->renderer3D->GetCurrentCamera()->GetOpenGLViewMatrix();
	math::float4x4 projectionMatrix = App->renderer3D->GetCurrentCamera()->GetOpenGLProjectionMatrix();
	math::float4x4 transformMatrix = gameObject->transform->GetGlobalMatrix();
	transformMatrix = transformMatrix.Transposed();

	ImGuizmo::MODE mode = currentImGuizmoMode;
	if (currentImGuizmoOperation == ImGuizmo::OPERATION::SCALE && mode != ImGuizmo::MODE::LOCAL)
		mode = ImGuizmo::MODE::LOCAL;

	ImGuizmo::Manipulate(
		viewMatrix.ptr(), projectionMatrix.ptr(),
		currentImGuizmoOperation, mode, transformMatrix.ptr()
	);

	if (ImGuizmo::IsUsing())
	{
		transformMatrix = transformMatrix.Transposed();
		gameObject->transform->SetMatrixFromGlobal(transformMatrix);
	}
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
#endif

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
	RecalculateQuadtree();
}

void ModuleScene::CreateQuadtree()
{
	const math::float3 center(0.0f, 0.0f, 0.0f);
	const math::float3 size(QUADTREE_SIZE_X, QUADTREE_SIZE_Y, QUADTREE_SIZE_Z);
	math::AABB boundary;
	boundary.SetFromCenterAndSize(center, size);

	quadtree.SetBoundary(boundary);
}

void ModuleScene::RecalculateQuadtree()
{
	std::vector<GameObject*> staticGameObjects;
	App->GOs->GetStaticGameObjects(staticGameObjects);

	for (uint i = 0; i < staticGameObjects.size(); ++i)
		App->scene->quadtree.Insert(staticGameObjects[i]);
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

#ifndef GAMEMODE

bool ModuleScene::IsGizmoValid() const
{
	return ImGuizmo::IsOver() || ImGuizmo::IsUsing();
}

#endif

void ModuleScene::FreeRoot()
{
	RELEASE(root);
}
