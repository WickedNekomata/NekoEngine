#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"
#include "ModuleCameraEditor.h"
#include "ModuleResourceManager.h"
#include "ModuleGui.h"
#include "ModuleGOs.h"
#include "DebugDrawer.h"

#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"

#include "Quadtree.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"

#include "Brofiler\Brofiler.h"

#pragma comment(lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment(lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment(lib, "glew\\libx86\\glew32.lib")

#include <stdio.h>
#include <algorithm>

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module(start_enabled)
{
	name = "Renderer3D";
}

ModuleRenderer3D::~ModuleRenderer3D() {}

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
		LoadStatus(jObject);

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
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

		// Multitexturing
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, (GLint*)&maxTextureUnits);

		for (uint i = 0; i < maxTextureUnits; ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glEnable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE0);
	}

	// DEFAULT SHADER
	const GLchar* vertex_shader_glsl_330_es =
		"#version 330 core\n"
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 1) in vec4 normals;\n"
		"layout (location = 2) in vec4 color;\n"
		"layout (location = 3) in vec2 texCoord;\n"
		"uniform mat4 model_matrix;\n"
		"uniform mat4 view_matrix;\n"
		"uniform mat4 proj_matrix;\n"
		"out vec4 ourColor;\n"
		"out vec2 ourTexCoord;\n"
		"void main()\n"
		"{\n"
		"    ourTexCoord = texCoord;\n"
		"    ourColor = color;\n"
		"    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);\n"
		"}\n";

	GLuint vShaderObject = glCreateShader(GL_VERTEX_SHADER); // Creates an empty Shader Object
	glShaderSource(vShaderObject, 1, &vertex_shader_glsl_330_es, NULL); // Takes an array of strings and stores it into the shader

	glCompileShader(vShaderObject);
	{
		GLint success = 0;
		glGetShaderiv(vShaderObject, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint logSize = 0;
			glGetShaderiv(vShaderObject, GL_INFO_LOG_LENGTH, &logSize);

			GLchar* infoLog = new GLchar[logSize];
			glGetShaderInfoLog(vShaderObject, logSize, NULL, infoLog);

			CONSOLE_LOG("Shader Object could not be compiled. ERROR: %s", infoLog);

			glDeleteShader(vShaderObject); // TODO
		}
		else
			CONSOLE_LOG("Successfully compiled Shader Object");
	}

	const GLchar* fragment_shader_glsl_330_es =
		"#version 330 core\n"
		"in vec4 ourColor;\n"
		"in vec2 ourTexCoord;\n"
		"out vec4 FragColor;\n"
		"uniform sampler2D ourTexture_0;\n"
		"void main()\n"
		"{\n"
		"     FragColor = texture(ourTexture_0, ourTexCoord);\n"
		"}\n";


	GLuint fShaderObject = glCreateShader(GL_FRAGMENT_SHADER); // Creates an empty Shader Object
	glShaderSource(fShaderObject, 1, &fragment_shader_glsl_330_es, NULL); // Takes an array of strings and stores it into the shader

	glCompileShader(fShaderObject);
	{
		GLint success = 0;
		glGetShaderiv(fShaderObject, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint logSize = 0;
			glGetShaderiv(fShaderObject, GL_INFO_LOG_LENGTH, &logSize);

			GLchar* infoLog = new GLchar[logSize];
			glGetShaderInfoLog(fShaderObject, logSize, NULL, infoLog);

			CONSOLE_LOG("Shader Object could not be compiled. ERROR: %s", infoLog);

			glDeleteShader(fShaderObject); // TODO
		}
		else
			CONSOLE_LOG("Successfully compiled Shader Object");
	}

	defaultShader = glCreateProgram();

	glAttachShader(defaultShader, vShaderObject);
	glAttachShader(defaultShader, fShaderObject);

	glLinkProgram(defaultShader);
	
	glDetachShader(defaultShader, vShaderObject);
	glDetachShader(defaultShader, fShaderObject);

	{
		GLint success;
		glGetProgramiv(defaultShader, GL_LINK_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint logSize = 0;
			glGetShaderiv(defaultShader, GL_INFO_LOG_LENGTH, &logSize);

			GLchar* infoLog = new GLchar[logSize];
			glGetProgramInfoLog(defaultShader, logSize, NULL, infoLog);

			CONSOLE_LOG("Shader Program could not be linked. ERROR: %s", infoLog);

			glDeleteProgram(defaultShader);
		}
		else
			CONSOLE_LOG("Successfully linked Shader Program");
	}


#ifndef GAMEMODE
	// Editor camera
	currentCamera = App->camera->camera;
	// Projection Matrix
	OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());
#endif // GAME

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(currentCamera->GetOpenGLViewMatrix().ptr());

	// Light 0 on cam pos
	lights[0].SetPos(currentCamera->frustum.pos.x, currentCamera->frustum.pos.y, currentCamera->frustum.pos.z);

	for (uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	return UPDATE_CONTINUE;
}

// PostUpdate: present buffer to screen
update_status ModuleRenderer3D::PostUpdate()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	// 1. Level geometry
	App->scene->Draw();

	if (currentCamera != nullptr)
	{
		if (currentCamera->HasFrustumCulling())
			FrustumCulling();

		for (uint i = 0; i < cameraComponents.size(); ++i)
			cameraComponents[i]->UpdateTransform();

		for (uint i = 0; i < meshComponents.size(); ++i)
		{
			if (meshComponents[i]->IsActive() && meshComponents[i]->GetParent()->GetSeenLastFrame())
				DrawMesh(meshComponents[i]);
		}
	}

#ifndef GAMEMODE
	// 2. Debug geometry
	if (debugDraw)
	{
		App->debugDrawer->StartDebugDraw();

		if (drawBoundingBoxes)
		{
			for (uint i = 0; i < meshComponents.size(); ++i)
				App->debugDrawer->DebugDraw(meshComponents[i]->GetParent()->boundingBox, Yellow);
		}

		if (drawCamerasFrustum)
		{
			for (uint i = 0; i < cameraComponents.size(); ++i)
				App->debugDrawer->DebugDraw(cameraComponents[i]->frustum, Grey);
		}

		if (drawQuadtree)
			RecursiveDrawQuadtree(App->scene->quadtree.root);

		App->debugDrawer->EndDebugDraw();
	}

	// 3. Editor
	App->gui->Draw();
#endif // GAME

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

	glDeleteProgram(defaultShader);

	return ret;
}

void ModuleRenderer3D::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_boolean(jObject, "vSync", vsync);

	json_object_set_boolean(jObject, "debugDraw", debugDraw);
	json_object_set_boolean(jObject, "drawBoundingBoxes", drawBoundingBoxes);
	json_object_set_boolean(jObject, "drawCamerasFrustum", drawCamerasFrustum);
	json_object_set_boolean(jObject, "drawQuadtree", drawQuadtree);
}
void ModuleRenderer3D::LoadStatus(const JSON_Object* jObject)
{
	SetVSync(json_object_get_boolean(jObject, "vSync"));

	debugDraw = json_object_get_boolean(jObject, "debugDraw");
	drawBoundingBoxes = json_object_get_boolean(jObject, "drawBoundingBoxes");
	drawCamerasFrustum = json_object_get_boolean(jObject, "drawCamerasFrustum");
	drawQuadtree = json_object_get_boolean(jObject, "drawQuadtree");
}

bool ModuleRenderer3D::OnGameMode()
{
	return SetCurrentCamera();
}

bool ModuleRenderer3D::OnEditorMode()
{
#ifndef GAMEMODE
	currentCamera = App->camera->camera;
#endif // GAME
	return true;
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);
	currentCamera->SetAspectRatio((float)width / (float)height);
	CalculateProjectionMatrix();
}

void ModuleRenderer3D::CalculateProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glLoadMatrixf(currentCamera->GetOpenGLProjectionMatrix().ptr());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

uint ModuleRenderer3D::GetMaxTextureUnits() const
{
	return maxTextureUnits;
}

bool ModuleRenderer3D::SetVSync(bool vsync) 
{
	bool ret = true;

	this->vsync = vsync;

	if (this->vsync) 
	{

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

void ModuleRenderer3D::SetDrawCamerasFrustum(bool drawCamerasFrustum)
{
	this->drawCamerasFrustum = drawCamerasFrustum;
}

bool ModuleRenderer3D::GetDrawCamerasFrustum() const
{
	return drawCamerasFrustum;
}

void ModuleRenderer3D::SetDrawQuadtree(bool drawQuadtree)
{
	this->drawQuadtree = drawQuadtree;
}

bool ModuleRenderer3D::GetDrawQuadtree() const
{
	return drawQuadtree;
}

ComponentMesh* ModuleRenderer3D::CreateMeshComponent(GameObject* parent)
{
	ComponentMesh* newComponent = new ComponentMesh(parent);

	std::vector<ComponentMesh*>::const_iterator it = std::find(meshComponents.begin(), meshComponents.end(), newComponent);

	if (it == meshComponents.end())
		meshComponents.push_back(newComponent);

	return newComponent;
}

bool ModuleRenderer3D::AddMeshComponent(ComponentMesh* toAdd)
{
	bool ret = true;

	std::vector<ComponentMesh*>::const_iterator it = std::find(meshComponents.begin(), meshComponents.end(), toAdd);
	ret = it == meshComponents.end();

	if (ret)
		meshComponents.push_back(toAdd);

	return ret;
}

bool ModuleRenderer3D::EraseMeshComponent(ComponentMesh* toErase)
{
	bool ret = false;

	std::vector<ComponentMesh*>::const_iterator it = std::find(meshComponents.begin(), meshComponents.end(), toErase);
	ret = it != meshComponents.end();

	if (ret)
		meshComponents.erase(it);

	return ret;
}

ComponentCamera* ModuleRenderer3D::CreateCameraComponent(GameObject* parent)
{
	ComponentCamera* newComponent = new ComponentCamera(parent);

	std::vector<ComponentCamera*>::const_iterator it = std::find(cameraComponents.begin(), cameraComponents.end(), newComponent);

	if (it == cameraComponents.end())
		cameraComponents.push_back(newComponent);

	return newComponent;
}

bool ModuleRenderer3D::AddCameraComponent(ComponentCamera* toAdd)
{
	bool ret = true;

	std::vector<ComponentCamera*>::const_iterator it = std::find(cameraComponents.begin(), cameraComponents.end(), toAdd);
	ret = it == cameraComponents.end();

	if (ret)
		cameraComponents.push_back(toAdd);

	return ret;
}

bool ModuleRenderer3D::EraseCameraComponent(ComponentCamera* toErase)
{
	bool ret = false;

	std::vector<ComponentCamera*>::const_iterator it = std::find(cameraComponents.begin(), cameraComponents.end(), toErase);
	ret = it != cameraComponents.end();

	if (ret)
		cameraComponents.erase(it);

	return ret;
}

bool ModuleRenderer3D::RecalculateMainCamera()
{
	bool ret = false;

	ComponentCamera* mainCamera = nullptr;

	bool multipleMainCameras = false;

	for (uint i = 0; i < cameraComponents.size(); ++i)
	{
		if (cameraComponents[i]->IsActive() && cameraComponents[i]->IsMainCamera())
		{
			if (mainCamera == nullptr)
				mainCamera = cameraComponents[i];
			else
			{
				multipleMainCameras = true;
				mainCamera = nullptr;
				break;
			}
		}			
	}

	if (multipleMainCameras)
	{
		CONSOLE_LOG("Warning! More than 1 Main Camera is defined");
	}
	else if (mainCamera == nullptr)
	{
		CONSOLE_LOG("Warning! No Main Camera is defined");
	}

	ret = SetMainCamera(mainCamera);

	return ret;
}

bool ModuleRenderer3D::SetMainCamera(ComponentCamera* mainCamera)
{
	bool ret = mainCamera != nullptr;

	if (ret)
		this->mainCamera = mainCamera;
	else
		CONSOLE_LOG("Main Camera could not be set");

	return ret;
}

bool ModuleRenderer3D::SetCurrentCamera()
{
	bool ret = RecalculateMainCamera();

	if (ret)
	{
		currentCamera = mainCamera;
		SetMeshComponentsSeenLastFrame(!currentCamera->HasFrustumCulling());
	}
	else
		CONSOLE_LOG("Current Camera could not be set");

	return ret;
}

ComponentCamera* ModuleRenderer3D::GetMainCamera() const
{
	return mainCamera;
}

ComponentCamera* ModuleRenderer3D::GetCurrentCamera() const
{
	return currentCamera;
}

void ModuleRenderer3D::SetMeshComponentsSeenLastFrame(bool seenLastFrame)
{
	for (uint i = 0; i < meshComponents.size(); ++i)
		meshComponents[i]->GetParent()->SetSeenLastFrame(seenLastFrame);
}

void ModuleRenderer3D::FrustumCulling() const
{
	std::vector<GameObject*> gameObjects;
	App->GOs->GetGameObjects(gameObjects);

	for (uint i = 0; i < gameObjects.size(); ++i)
		gameObjects[i]->SetSeenLastFrame(false);

	// Static objects
	std::vector<GameObject*> seen;
	App->scene->quadtree.CollectIntersections(seen, currentCamera->frustum);

	// Dynamic objects
	std::vector<GameObject*> dynamicGameObjects;
	App->GOs->GetDynamicGameObjects(dynamicGameObjects);

	for (uint i = 0; i < dynamicGameObjects.size(); ++i)
	{
		if (dynamicGameObjects[i]->boundingBox.IsFinite())
		{
			if (currentCamera->frustum.Intersects(dynamicGameObjects[i]->boundingBox))
				seen.push_back(dynamicGameObjects[i]);
		}
	}

	for (uint i = 0; i < seen.size(); ++i)
		seen[i]->SetSeenLastFrame(true);
}

void ModuleRenderer3D::DrawMesh(ComponentMesh* toDraw) const
{
	if (toDraw->res == 0)
		return;

	ComponentMaterial* materialRenderer = toDraw->GetParent()->materialRenderer;

	const ResourceMesh* res = (const ResourceMesh*)App->res->GetResource(toDraw->res);

	GLuint shaderProgram;

	if (glIsProgram(materialRenderer->shaderProgram))
		shaderProgram = materialRenderer->shaderProgram;
	else
		shaderProgram = defaultShader;

	glUseProgram(shaderProgram);

	for (int i = 0; i < materialRenderer->res.size(); ++i)
	{
		const ResourceTexture* texRes = (const ResourceTexture*)App->res->GetResource(materialRenderer->res[i].res);

		if (texRes == nullptr)
			continue;

		glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
		glBindTexture(GL_TEXTURE_2D, texRes->id);
		char* ourTexturei = "ourTexture_%i";
		ourTexturei[12] = i - '0';
		glUniform1i(glGetUniformLocation(shaderProgram, ourTexturei), i);
	}

	uint location = glGetUniformLocation(shaderProgram, "model_matrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, toDraw->GetParent()->transform->GetGlobalMatrix().ptr());
	location = glGetUniformLocation(shaderProgram, "view_matrix");
	glUniformMatrix4fv(location, 1, GL_FALSE, App->camera->camera->GetOpenGLViewMatrix().ptr());
	location = glGetUniformLocation(shaderProgram, "proj_matrix");
	glUniformMatrix4fv(location, 1, GL_FALSE, App->camera->camera->GetOpenGLProjectionMatrix().ptr());

	glBindVertexArray(res->VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res->IBO);
	glDrawElements(GL_TRIANGLES, res->indicesSize, GL_UNSIGNED_INT, NULL);


	for (int i = 0; i < materialRenderer->res.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}

void ModuleRenderer3D::RecursiveDrawQuadtree(QuadtreeNode* node) const
{
	App->debugDrawer->DebugDraw(node->boundingBox, Green);

	for (std::list<GameObject*>::const_iterator it = node->objects.begin(); it != node->objects.end(); ++it)
		App->debugDrawer->DebugDraw((*it)->boundingBox, DarkGreen);

	if (!node->IsLeaf())
	{
		for (uint i = 0; i < 4; ++i)
			RecursiveDrawQuadtree(node->children[i]);
	}
}