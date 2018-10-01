#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "Primitive.h"
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
	int nVertex = 10;
	float* vertex = new float[13 * 3];
	int radius = 1;

	float angle = 360/nVertex;

	for (int j = 0; j < 13 * 3; j++)
	{
		if (j % 3 == 0)
		{
			vertex[j++] = 0;
			vertex[j++] = 0;
			vertex[j++] = 0;
			continue;
		}
		vertex[j++] = sin(angle) * radius;
		vertex[j++] = cos(angle) * radius;
		vertex[j++] = 0;
	}

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 13 * 3, vertex, GL_STATIC_DRAW);
	delete[] vertex;
	return ret;
}

update_status ModuleScene::Update(float dt)
{
	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	bool ret = true;

	return ret;
}

void ModuleScene::Draw() const 
{
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	// … draw other buffers
	glDrawArrays(GL_TRIANGLES, 0, 13);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	pPlane plane;
	plane.Render();
}