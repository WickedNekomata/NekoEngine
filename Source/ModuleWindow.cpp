#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"

#include "glew\include\GL\glew.h"

ModuleWindow::ModuleWindow(bool start_enabled) : Module(start_enabled)
{
	name = "Window";

	window = NULL;
	screen_surface = NULL;
}

ModuleWindow::~ModuleWindow()
{}

bool ModuleWindow::Init(JSON_Object* jObject)
{
	bool ret = true;

	CONSOLE_LOG("Init SDL window & surface");

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		CONSOLE_LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		size = json_object_get_number(jObject, "size");
		width = json_object_get_number(jObject, "width") * size;
		height = json_object_get_number(jObject, "height") * size;

		fullscreen = json_object_get_boolean(jObject, "fullscreen");
		resizable = json_object_get_boolean(jObject, "resizable");
		borderless = json_object_get_boolean(jObject, "borderless");
		fullDesktop = json_object_get_boolean(jObject, "fullDesktop");

		// Create window
		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		// Use OpenGL 3.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		if (fullscreen)
			flags |= SDL_WINDOW_FULLSCREEN;

		if (resizable)
			flags |= SDL_WINDOW_RESIZABLE;

		if (borderless)
			flags |= SDL_WINDOW_BORDERLESS;

		if (fullDesktop)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		window = SDL_CreateWindow(App->GetAppName(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		if (window == NULL)
		{
			CONSOLE_LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			// Get window surface
			screen_surface = SDL_GetWindowSurface(window);
		}
	}

	return ret;
}

bool ModuleWindow::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG("Destroying SDL window and quitting all SDL systems");

	// Destroy window
	if (window != NULL)
		SDL_DestroyWindow(window);

	// Quit SDL subsystems
	SDL_Quit();

	return ret;
}

void ModuleWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(window, title);
}

void ModuleWindow::SetWindowBrightness(float brightness) const
{
	SDL_SetWindowBrightness(window, brightness);
}

float ModuleWindow::GetWindowBrightness() const
{
	return SDL_GetWindowBrightness(window);
}

void ModuleWindow::SetScreenSize(uint size) 
{
	this->size = size;
}

uint ModuleWindow::GetScreenSize() const 
{
	return size;
}

void ModuleWindow::SetWindowWidth(uint width)
{
	this->width = width;
	UpdateWindowSize();
}

uint ModuleWindow::GetWindowWidth() const
{
	return width;
}

void ModuleWindow::SetWindowHeight(uint height) 
{
	this->height = height;
	UpdateWindowSize();
}

uint ModuleWindow::GetWindowHeight() const
{
	return height;
}

void ModuleWindow::UpdateWindowSize() const
{
	SDL_SetWindowSize(window, width, height);
	App->renderer3D->OnResize(width, height);
}

uint ModuleWindow::GetRefreshRate() const
{
	uint refreshRate = 0;

	SDL_DisplayMode desktopDisplay;
	if (SDL_GetDesktopDisplayMode(0, &desktopDisplay) == 0)
		refreshRate = desktopDisplay.refresh_rate;
	else
		CONSOLE_LOG("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());

	return refreshRate;
}

void ModuleWindow::GetScreenSize(uint& width, uint& height) const
{
	SDL_DisplayMode desktopDisplay;
	if (SDL_GetDesktopDisplayMode(0, &desktopDisplay) == 0)
	{
		width = desktopDisplay.w;
		height = desktopDisplay.h;
	}
	else
		CONSOLE_LOG("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
}

void ModuleWindow::SetFullscreenWindow(bool fullscreen) 
{
	this->fullscreen = fullscreen;

	if (fullscreen)
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	else
		SDL_SetWindowFullscreen(window, 0);
}

bool ModuleWindow::GetFullscreenWindow() const 
{
	return fullscreen;
}

void ModuleWindow::SetFullDesktopWindow(bool fullDesktop)
{
	this->fullDesktop = fullDesktop;

	if (fullDesktop)
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	else
		SDL_SetWindowFullscreen(window, 0);
}

bool ModuleWindow::GetFullDesktopWindow() const
{
	return fullDesktop;
}

void ModuleWindow::SetResizableWindow(bool resizable) 
{
	this->resizable = resizable;

	if (resizable)
		SDL_SetWindowResizable(window, SDL_TRUE);
	else
		SDL_SetWindowResizable(window, SDL_FALSE);
}

bool ModuleWindow::GetResizableWindow() const 
{
	return resizable;
}

void ModuleWindow::SetBorderlessWindow(bool borderless) 
{
	this->borderless = borderless;

	if (borderless)
		SDL_SetWindowBordered(window, SDL_FALSE);
	else
		SDL_SetWindowBordered(window, SDL_TRUE);
}

bool ModuleWindow::GetBorderlessWindow() const 
{
	return borderless;
}

void ModuleWindow::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_number(jObject, "width", width);
	json_object_set_number(jObject, "height", height);
	json_object_set_number(jObject, "size", size);

	json_object_set_boolean(jObject, "fullscreen", fullscreen);
	json_object_set_boolean(jObject, "resizable", resizable);
	json_object_set_boolean(jObject, "borderless", borderless);
	json_object_set_boolean(jObject, "fullDesktop", fullDesktop);
}

void ModuleWindow::LoadStatus(const JSON_Object* jObject)
{
	width = json_object_get_number(jObject, "width");
	height = json_object_get_number(jObject, "height");
	size = json_object_get_number(jObject, "size");

	fullscreen = json_object_get_boolean(jObject, "fullscreen");
	resizable = json_object_get_boolean(jObject, "resizable");
	borderless = json_object_get_boolean(jObject, "borderless");
	fullDesktop = json_object_get_boolean(jObject, "fullDesktop");

	SetWindowWidth(width);
	SetWindowHeight(height);

	SetFullDesktopWindow(fullDesktop);
	SetFullscreenWindow(fullscreen);
	SetBorderlessWindow(borderless);
	SetResizableWindow(resizable);	
}
