#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include "Module.h"

#include "CurrentSelection.h"
#include "Quadtree.h"

#include "glew\include\GL\glew.h"

#include "MathGeoLib\include\Math\TransformOps.h"
#include "MathGeoLib\include\Math\MathConstants.h"

#include "ImGuizmo\ImGuizmo.h"

#define SELECT(x) App->scene->selectedObject = x;
#define MAX_UNDO 20

class PrimitiveGrid;
class GameObject;

#ifndef GAMEMODE
struct LastTransform
{
	math::float4x4 matrix;
	GameObject* object;
};
#endif

class ModuleScene : public Module
{
public:

	ModuleScene(bool start_enabled = true);
	~ModuleScene();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status Update();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	void Draw() const;

	// ImGuizmo
	void OnGizmos(GameObject* gameObject);

#ifndef GAMEMODE
	void SaveLastTransform(math::float4x4 matrix);
	void GetPreviousTransform();
	void SetImGuizmoOperation(ImGuizmo::OPERATION operation);
	ImGuizmo::OPERATION GetImGuizmoOperation() const;

	void SetImGuizmoMode(ImGuizmo::MODE mode);
	ImGuizmo::MODE GetImGuizmoMode() const;
#endif

	bool GetShowGrid() const;
	void SetShowGrid(bool showGrid);

	void RecreateQuadtree();
	void CreateQuadtree();
	void RecalculateQuadtree();
	void CreateRandomStaticGameObject();
	bool IsGizmoValid() const;

	void FreeRoot();

private:

	PrimitiveGrid* grid = nullptr;
	bool showGrid = true;

#ifndef GAMEMODE
	ImGuizmo::OPERATION currentImGuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE currentImGuizmoMode = ImGuizmo::MODE::WORLD;
#endif

public:

#ifndef GAMEMODE
	math::float4x4 lastMat;
	std::list<LastTransform> prevTransforms;
	bool saveTransform = false;

	CurrentSelection selectedObject;
#endif

	GameObject* child = nullptr;
	GameObject* root = nullptr;

	Quadtree quadtree;
};

#endif
