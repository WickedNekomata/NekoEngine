#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include "Module.h"

#include "CurrentSelection.h"
#include "Quadtree.h"

#include "glew\include\GL\glew.h"

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/MathConstants.h"

#include "ImGuizmo/ImGuizmo.h"

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

	void OnSystemEvent(System_Event event);

	void Draw() const;

	// ImGuizmo
	void OnGizmos(GameObject* gameObject) const;

	void SetImGuizmoOperation(ImGuizmo::OPERATION operation);
	ImGuizmo::OPERATION GetImGuizmoOperation() const;

	void SetImGuizmoMode(ImGuizmo::MODE mode);
	ImGuizmo::MODE GetImGuizmoMode() const;

	bool GetShowGrid() const;
	void SetShowGrid(bool showGrid);

	void RecreateQuadtree();
	void CreateQuadtree();
	void RecalculateQuadtree();
	void CreateRandomStaticGameObject();
	bool IsImguizmoValid() const;

	void FreeRoot();

private:

	PrimitiveGrid* grid = nullptr;
	bool showGrid = true;

	ImGuizmo::OPERATION currentImGuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE currentImGuizmoMode = ImGuizmo::MODE::WORLD;

public:

	CurrentSelection selectedObject;

	GameObject* child = nullptr;
	GameObject* root = nullptr;

	Quadtree quadtree;
};

#endif
