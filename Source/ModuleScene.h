#ifndef __MODULE_SCENE_H__
#define __MODULE_SCENE_H__

#include "Module.h"

#include "glew\include\GL\glew.h"

class PrimitiveCube;

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

private:

	GLuint VBO, cubeID;
	PrimitiveCube* cube;
};

#endif
