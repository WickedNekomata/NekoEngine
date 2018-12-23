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

class PrimitiveGrid;
class GameObject;

class ModuleScene : public Module
{
public:

	ModuleScene(bool start_enabled = true);
	~ModuleScene();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status Update();
	bool CleanUp();

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	void OnSystemEvent(System_Event event);

	void Draw() const;

	// ImGuizmo
	void OnGizmos(GameObject* gameObject) const;

#ifndef GAMEMODE
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

	CurrentSelection selectedObject;

	GameObject* child = nullptr;
	GameObject* root = nullptr;

	Quadtree quadtree;
};

#endif
