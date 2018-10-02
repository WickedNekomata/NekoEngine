#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include "Module.h"

#include "glew\include\GL\glew.h"

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/MathConstants.h"

class PrimitiveCube;
class PrimitiveRay;
class PrimitivePlane;

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

	void GlBeginCube() const;
	void DrawCircleDirectMode(math::float3 position, float radius, uint subdivision, float rotationAngle, math::float3 rotation) const;
	void DrawSphereDirectMode() const;
	void DrawCylinderDirectMode() const;

private:

	GLuint VBO, cubeID;
	PrimitiveCube* cube;
	PrimitiveRay* ray;
	PrimitivePlane* plane;
};

#endif
