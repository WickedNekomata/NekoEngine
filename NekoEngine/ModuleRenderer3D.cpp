#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"

#include "glew\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"

#pragma comment(lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment(lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment(lib, "glew/lib/Release/Win32/glew32.lib")

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
		// Use vsync
		SetVSync(vsync);

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
		glEnable(GL_COLOR_MATERIAL);
	}

	// Projection matrix for
	OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetViewMatrix());

	// Light 0 on cam pos
	lights[0].SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);

	for (uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	return UPDATE_CONTINUE;
}

// PostUpdate: present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	// 1. Level geometry
	App->scene->Draw();

	// 2. Debug geometry

	// 3. Editor
	App->gui->Draw();

	// 4. Swap buffers
	SDL_GL_SwapWindow(App->window->window);

	return UPDATE_CONTINUE;
}

bool ModuleRenderer3D::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	return ret;
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	ProjectionMatrix = Perspective(60.0f, (float)width / (float)height, 0.125f, 512.0f);
	glLoadMatrixf((GLfloat*)ProjectionMatrix.ptr());

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

bool ModuleRenderer3D::GetCapabilityState(GLenum capability)
{
	bool ret = false;

	if (glIsEnabled(capability))
		ret = true;

	return ret;
}

void ModuleRenderer3D::SetCapabilityState(GLenum capability, bool enable)
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

math::float4x4 ModuleRenderer3D::Perspective(float fovy, float aspect, float n, float f)
{
	math::float4x4 Perspective;

	float coty = 1.0f / tan(fovy * (float)M_PI / 360.0f);

	Perspective[0][0] = coty / aspect;
	Perspective[0][1] = 0.0f;
	Perspective[0][2] = 0.0f;
	Perspective[0][3] = 0.0f;
	Perspective[1][0] = 0.0f;
	Perspective[1][1] = coty;
	Perspective[1][2] = 0.0f;
	Perspective[1][3] = 0.0f;
	Perspective[2][0] = 0.0f;
	Perspective[2][1] = 0.0f;
	Perspective[2][2] = (n + f) / (n - f);
	Perspective[2][3] = -1.0f;
	Perspective[3][0] = 0.0f;
	Perspective[3][1] = 0.0f;
	Perspective[3][2] = 2.0f * n * f / (n - f);
	Perspective[3][3] = 0.0f;

	return Perspective;
}