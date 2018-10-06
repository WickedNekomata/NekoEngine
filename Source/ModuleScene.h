#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include "Module.h"

#include "glew\include\GL\glew.h"

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/MathConstants.h"

class PrimitiveCube;
class PrimitiveRay;
class PrimitivePlane;
class PrimitiveCircle;
class PrimitiveFrustum;
class PrimitiveSphere;
class PrimitiveCylinder;
class PrimitiveCone;
class PrimitiveArrow;
class PrimitiveGrid;

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

private:

	GLuint VBO, cubeID;

	PrimitiveCube* cube = nullptr;
	PrimitiveRay* ray = nullptr;
	PrimitivePlane* plane = nullptr;
	PrimitiveCircle* circle = nullptr;
	PrimitiveFrustum* frustum = nullptr;
	PrimitiveSphere* sphere = nullptr;
	PrimitiveCylinder* cylinder = nullptr;
	PrimitiveCone* cone = nullptr;
	PrimitiveArrow* arrow = nullptr;
	PrimitiveGrid* grid = nullptr;
};

#endif
