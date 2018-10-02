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
	ray = new PrimitiveRay(math::float3(0.0f, 0.0f, 0.0f), math::float3(100.0f, 0.0f, 0.0f));
	plane = new PrimitivePlane(math::float3(0.0f, -5.0f, 0.0f));

	// Loading vube into vram. Vertex can be repeated
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

	return ret;
}

void ModuleScene::Draw() const 
{
	// Rendering cube from the vram. Vertex are not reusable
	glEnableClientState(GL_VERTEX_ARRAY);
	glTranslatef(5, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, cubeID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, 108);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	// Rendering cube in direct mode
	glTranslatef(-5, 0, 0);
	GlBeginCube();

	// Rendering Cube from the vram. Vertex are reusable
	glTranslatef(-5, 0, 0);
	cube->Render();
	glTranslatef(+5, 0, 0);


	ray->Render();

	plane->Render();

	//DrawSphereDirectMode();


	//DrawSphereDirectMode();
	DrawCylinderDirectMode();

}

void ModuleScene::GlBeginCube() const
{
	glBegin(GL_TRIANGLES);

	//front
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);

	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);

	//Dreta
	glVertex3f(0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);

	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	//Esquerra
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);

	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	//back
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	//up
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);

	glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);

	//down
	glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);

	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(-0.5f, -0.5f, 0.5f);

	glEnd();
}

void ModuleScene::DrawCircleDirectMode(math::float3 position, float radius, uint subdivisions, float rotationAngle, math::float3 rotation) const
{
	// subdivisions must be an even, positive number
	subdivisions *= 2;

	float deltaAngle = 360.0f / (float)subdivisions; // in degrees
	float angle = 0.0f;

	glRotatef(rotationAngle, rotation.x, rotation.y, rotation.z);
	glBegin(GL_TRIANGLES);
	for (uint i = 0; i < subdivisions; ++i)
	{
		float nextAngle = angle + deltaAngle;
		
		// Triangle
		glVertex3f(position.x, position.y, position.z);
		glVertex3f(radius * cosf(DEGTORAD * nextAngle), radius * sinf(DEGTORAD * nextAngle), position.z);
		glVertex3f(radius * cosf(DEGTORAD * angle), radius * sinf(DEGTORAD * angle), position.z);
			
		angle = nextAngle;
	}
	glEnd();
	glRotatef(-rotationAngle, rotation.x, rotation.y, rotation.z);
}

void ModuleScene::DrawSphereDirectMode() const 
{
	math::float3 position(0.0f, 0.0f, 0.0f);
	float radius = 1.0f;

	uint verticalSubdivisions = 8;
	uint horizontalSubdivisions = 3;

	// verticalSubdivisions must be an even, positive number
	verticalSubdivisions *= 2;

	// Vertical circles
	float deltaAngle = 360.0f / verticalSubdivisions;
	for (float angle = 0.0f; angle < 360.0f; angle += deltaAngle)
	{
		DrawCircleDirectMode(position, radius, verticalSubdivisions / 2, angle, math::float3(0.0f, 1.0f, 0.0f));
	}

	// Horizontal circles
	DrawCircleDirectMode(position, radius, verticalSubdivisions / 2, 90.0f, math::float3(1.0f, 0.0f, 0.0f));

	float deltaHeight = radius / (float)horizontalSubdivisions;
	deltaAngle /= 2;
	float newRadius = radius - DEGTORAD * deltaAngle * 2;
	for (float height = deltaHeight; height < radius; height += deltaHeight)
	{
		DrawCircleDirectMode(math::float3(position.x, position.y, position.z + height), newRadius, verticalSubdivisions / 2, 90.0f, math::float3(1.0f, 0.0f, 0.0f));
		DrawCircleDirectMode(math::float3(position.x, position.y, position.z - height), newRadius, verticalSubdivisions / 2, 90.0f, math::float3(1.0f, 0.0f, 0.0f));
		newRadius -= DEGTORAD * deltaAngle;
	}

	//DrawCircleDirectMode(position, radius, verticalSubdivisions, );

	// Bottom
}

void ModuleScene::DrawCylinderDirectMode() const 
{
	math::float3 position(0.0f, 0.0f, 0.0f);
	float radius = 1.0f;
	float height = 5.0f;
	uint heightSegments = 1;
	uint capSegments = 1;
	uint sides = 16;

	// Top cap
	DrawCircleDirectMode(math::float3(position.x, position.y + height / 2.0f, position.z), radius, sides, 0.0f, math::float3(0.0f, 0.0f, 0.0f));

	// Bottom cap
	DrawCircleDirectMode(math::float3(position.x, position.y - height / 2.0f, position.z), radius, sides, 0.0f, math::float3(0.0f, 0.0f, 0.0f));
}