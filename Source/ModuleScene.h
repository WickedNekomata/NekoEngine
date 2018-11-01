#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include "Module.h"

#include "Quadtree.h"

#include "glew\include\GL\glew.h"

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/MathConstants.h"

class PrimitiveGrid;
class GameObject;

class ModuleScene : public Module
{
public:

	ModuleScene(bool start_enabled = true);
	~ModuleScene();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void Draw() const;

	bool GetShowGrid() const;
	void SetShowGrid(bool showGrid);

	void CreateQuadtree();
	void CreateRandomStaticGameObject();

private:

	PrimitiveGrid* grid = nullptr;
	bool showGrid = true;

public:

	GameObject* child = nullptr;
	GameObject* root = nullptr;
	GameObject* currentGameObject = nullptr;

	Quadtree quadtree;
};

#endif
