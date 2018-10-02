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

	cube = new PrimitiveCube(math::float3(0.0f, 0.0f, 0.0f));
	cube->EnableAxis(true);
	ray = new PrimitiveRay(math::float3(10.0f, 0.0f, 0.0f), math::float3(100.0f, 0.0f, 0.0f));
	plane = new PrimitivePlane(math::float3(0.0f, -5.0f, 0.0f));
	circle = new PrimitiveCircle();
	sphere = new PrimitiveSphere();
	frustum = new PrimitiveFrustum(math::float3(0.0f, 0.0f, -10.0f), 1.f, 1.f, math::float3(0.0f, 0.0f, -20.0f), 5.f, 5.f);
	frustum->EnableAxis(true);

	cylinder = new PrimitiveCylinder();

	// Load Cube into vram. Vertex can be repeated
	float cubeVertex[] =
	{
		-0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
					   
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
	
		 0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,

		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,
		
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,

		 0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
					   
		-0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
	};

	glGenBuffers(1, &cubeID);
	glBindBuffer(GL_ARRAY_BUFFER, cubeID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 108, cubeVertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return ret;
}

update_status ModuleScene::Update(float dt)
{
	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	bool ret = true;

	RELEASE(cube);
	RELEASE(ray);
	RELEASE(plane);
	RELEASE(circle);
	RELEASE(frustum);
	RELEASE(sphere);
	RELEASE(cylinder);

	return ret;
}

void ModuleScene::Draw() const 
{
	// Cube
	// 1. Direct Mode
	glTranslatef(-5.0f, 0.0f, 0.0f);
	DrawCubeDirectMode();
	glTranslatef(5.0f, 0.0f, 0.0f);

	// 2. Vertex Array
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, cubeID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, 108);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	
	// Element Array (Vertex Array with indices)
	glTranslatef(5.0f, 0.0f, 0.0f);
	cube->Render();
	glTranslatef(-5.0f, 0.0f, 0.0f);

	// Big plane as ground
	plane->Render();

	// Primitives
	glTranslatef(15.0f, 0.0f, -5.0f);
	circle->Render();
	glTranslatef(-15.0f, 0.0f, 5.0f);

	glTranslatef(15.0f, 0.0f, 5.0f);
	sphere->Render();
	glTranslatef(-15.0f, 0.0f, -5.0f);

	ray->Render();
	frustum->Render();
	//cylinder->Render();
}

void ModuleScene::DrawCubeDirectMode() const
{
	glBegin(GL_TRIANGLES);

	// Front
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);

	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);

	// Right
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);

	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	// Left
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);

	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Back
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Up
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);

	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	// Down
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);

	glEnd();
}