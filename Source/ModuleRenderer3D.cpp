#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Primitive.h"
#include "Color.h"

#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"

#pragma comment(lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment(lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment(lib, "glew/libx86/glew32.lib")

#include <algorithm>

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module(start_enabled)
{
	name = "Renderer3D";
}

ModuleRenderer3D::~ModuleRenderer3D()
{}

bool ModuleRenderer3D::Init(JSON_Object* jObject)
{
	bool ret = true;

	CONSOLE_LOG("Creating 3D Renderer context");
	
	// Create context
	context = SDL_GL_CreateContext(App->window->window);

	if (context == NULL)
	{
		CONSOLE_LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	
	if (ret)
	{
		// TODO: load this variables from .json (and save them when the app is closed)
		SetVSync(json_object_get_boolean(jObject, "vSync"));
		SetDebugDraw(json_object_get_boolean(jObject, "debugDraw"));
		//SetFOV(MAX_FOV);

		// Initialize glew
		GLenum error = glewInit();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG("Error initializing glew! %s\n", glewGetErrorString(error));
			ret = false;
		}

		// Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// Initialize clear depth
		glClearDepth(1.0f);

		// Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		GLfloat LightModelAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();
		lights[0].Active(true);

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		// GL capabilities
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_MATERIAL);
	}

	// Projection Matrix for
	OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());


	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->camera->GetOpenGLViewMatrix());

	// Light 0 on cam pos
	lights[0].SetPos(App->camera->camera->cameraFrustum.pos.x, App->camera->camera->cameraFrustum.pos.y, App->camera->camera->cameraFrustum.pos.z);

	for (uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	return UPDATE_CONTINUE;
}

// PostUpdate: present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	// 1. Level geometry
	App->scene->Draw();

	if (mainCamera != nullptr)
	{
		if (mainCamera->GetFrustumCulling())
			FrustumCulling();

		for (uint i = 0; i < meshComponents.size(); ++i)
		{
			if (meshComponents[i]->GetParent()->GetSeenLastFrame())
				DrawMesh(meshComponents[i]);
		}
	}

	// 2. Debug geometry
	if (debugDraw)
	{
		bool cullFace = GetCapabilityState(GL_CULL_FACE);
		bool lighting = GetCapabilityState(GL_LIGHTING);
		bool texture2D = GetCapabilityState(GL_TEXTURE_2D);

		SetDebugDrawCapabilitiesState(false, false, false);

		if (App->scene->GetDrawQuadtree())
			App->scene->RecursiveDrawQuadtree(App->scene->quadtree.root);

		if (drawBoundingBoxes)
		{
			for (uint i = 0; i < meshComponents.size(); ++i)
				DrawBoundingBox(meshComponents[i]);
		}

		SetDebugDrawCapabilitiesState(cullFace, lighting, texture2D);
	}

	// 3. Editor
	App->gui->Draw();

	// 4. Swap buffers
	SDL_GL_MakeCurrent(App->window->window, context);
	SDL_GL_SwapWindow(App->window->window);

	return UPDATE_CONTINUE;
}

bool ModuleRenderer3D::CleanUp()
{
	bool ret = true;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	uint x, y;
	App->window->GetScreenSize(x, y);
	glViewport(0, 0, x, y);

	CONSOLE_LOG("Destroying 3D Renderer");
	SDL_GL_DeleteContext(context);

	return ret;
}

void ModuleRenderer3D::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_boolean(jObject, "vSync", vsync);
}
void ModuleRenderer3D::LoadStatus(const JSON_Object* jObject)
{
	SetVSync(json_object_get_boolean(jObject, "vSync"));
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	CalculateProjectionMatrix();
}

void ModuleRenderer3D::CalculateProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glLoadMatrixf(App->camera->camera->GetOpenGLProjectionMatrix());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool ModuleRenderer3D::SetVSync(bool vsync) 
{
	bool ret = true;

	this->vsync = vsync;

	if (this->vsync) {

		if (SDL_GL_SetSwapInterval(1) == -1)
		{
			ret = false;
			CONSOLE_LOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}
	}
	else {
	
		if (SDL_GL_SetSwapInterval(0) == -1) 
		{
			ret = false;
			CONSOLE_LOG("Warning: Unable to set immediate updates! SDL Error: %s\n", SDL_GetError());
		}
	}

	return ret;
}

bool ModuleRenderer3D::GetVSync() const 
{
	return vsync;
}

void ModuleRenderer3D::SetDebugDrawCapabilitiesState(bool cullFace, bool lighting, bool texture2D) const
{
	SetCapabilityState(GL_CULL_FACE, cullFace);
	SetCapabilityState(GL_LIGHTING, lighting);
	SetCapabilityState(GL_TEXTURE_2D, texture2D);
}

void ModuleRenderer3D::SetCapabilityState(GLenum capability, bool enable) const
{
	if (GetCapabilityState(capability))
	{
		if (!enable)
			glDisable(capability);
	}
	else
	{
		if (enable)
			glEnable(capability);
	}
}

bool ModuleRenderer3D::GetCapabilityState(GLenum capability) const
{
	bool ret = false;

	if (glIsEnabled(capability))
		ret = true;

	return ret;
}

void ModuleRenderer3D::SetWireframeMode(bool enable) const 
{
	if (enable)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool ModuleRenderer3D::IsWireframeMode() const 
{
	bool ret = false;

	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);

	if (polygonMode[0] == GL_LINE && polygonMode[1] == GL_LINE)
		ret = true;

	return ret;
}

void ModuleRenderer3D::SetDebugDraw(bool debugDraw)
{
	this->debugDraw = debugDraw;
}

bool ModuleRenderer3D::GetDebugDraw() const 
{
	return debugDraw;
}

void ModuleRenderer3D::SetDrawBoundingBoxes(bool drawBoundingBoxes)
{
	this->drawBoundingBoxes = drawBoundingBoxes;
}

bool ModuleRenderer3D::GetDrawBoundingBoxes() const
{
	return drawBoundingBoxes;
}

void ModuleRenderer3D::SetMainCamera(ComponentCamera* mainCamera)
{
	if (mainCamera == nullptr)
		return;

	if (this->mainCamera != nullptr && this->mainCamera != mainCamera)
		this->mainCamera->SetMainCamera(false);

	this->mainCamera = mainCamera;
}

void ModuleRenderer3D::SetMeshComponentsSeenLastFrame(bool seenLastFrame)
{
	for (uint i = 0; i < meshComponents.size(); ++i)
		meshComponents[i]->GetParent()->SetSeenLastFrame(seenLastFrame);
}

void ModuleRenderer3D::FrustumCulling() const
{
	for (uint i = 0; i < meshComponents.size(); ++i)
		meshComponents[i]->GetParent()->SetSeenLastFrame(false);

	// Static objects (test against Quadtree)
	std::vector<GameObject*> objects;
	App->scene->quadtree.CollectIntersections(objects, mainCamera->cameraFrustum);

	for (uint i = 0; i < objects.size(); ++i)
		objects[i]->SetSeenLastFrame(true);

	// Dynamic objects (test against Frustum)
	/*
	for (uint i = 0; i < meshComponents.size(); ++i)
	{
		if (!meshComponents[i]->GetParent()->GetIsStatic())
		{
			if (mainCamera->cameraFrustum.Contains(meshComponents[i]->GetParent()->boundingBox))
				meshComponents[i]->GetParent()->SetSeenLastFrame(true);
		}
	}
	*/
}

// -------------------- COMPONENTS------------------------------COMPONENTS--------------------------------COMPONENTS-------------------- //

ComponentMesh* ModuleRenderer3D::CreateMeshComponent(GameObject* parent)
{
	ComponentMesh* newComponent;

	newComponent = new ComponentMesh(parent);

	meshComponents.push_back(newComponent);

	return newComponent;
}

void ModuleRenderer3D::EraseComponent(ComponentMesh* toErase)
{
	for (int i = 0; i < meshComponents.size(); ++i)
	{
		if (meshComponents[i] == toErase)
			meshComponents.erase(std::remove(meshComponents.begin(), meshComponents.end(), meshComponents[i]), meshComponents.end());
	}
}

void ModuleRenderer3D::DrawMesh(ComponentMesh* toDraw) const
{
	glPushMatrix();
	math::float4x4 matrix = toDraw->GetParent()->transform->GetGlobalMatrix();
	glMultMatrixf(matrix.Transposed().ptr());
	
	ComponentMaterial* materialRenderer = toDraw->GetParent()->materialRenderer;

	if (materialRenderer != nullptr)
	{
		for (int i = 0; i < materialRenderer->textures.size(); ++i)
		{
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glActiveTexture(GL_TEXTURE0 + i);

			glBindTexture(GL_TEXTURE_2D, materialRenderer->textures[i]->id);

			glBindBuffer(GL_ARRAY_BUFFER, toDraw->mesh->textureCoordsID);
			glTexCoordPointer(2, GL_FLOAT, 0, NULL);
		}
	}

	// -----

	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, toDraw->mesh->verticesID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, toDraw->mesh->indicesID);
	glDrawElements(GL_TRIANGLES, toDraw->mesh->indicesSize, GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);

	// -----

	// Disable Multitexturing
	if (materialRenderer != nullptr)
	{
		for (int i = 0; i < materialRenderer->textures.size(); ++i)
		{
			glClientActiveTexture(GL_TEXTURE0 + i);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	
	glPopMatrix();
}

void ModuleRenderer3D::DrawBoundingBox(ComponentMesh* toDraw) const
{
	if (toDraw->debugBoundingBox != nullptr)
	{
		math::float3 position = toDraw->GetParent()->boundingBox.CenterPoint();
		math::float3 scale = toDraw->GetParent()->boundingBox.Size();
		toDraw->debugBoundingBox->Render(math::float4x4::FromTRS(position, math::Quat::identity, scale));
	}
}