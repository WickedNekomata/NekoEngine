#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"

ModuleWindow::ModuleWindow(bool start_enabled) : Module(start_enabled)
{
	window = NULL;
	screen_surface = NULL;
}

ModuleWindow::~ModuleWindow()
{}

bool ModuleWindow::Init()
{
	bool ret = true;

	_LOG("Init SDL window & surface");

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		_LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		// Create window
		width = SCREEN_WIDTH * SCREEN_SIZE;
		height = SCREEN_HEIGHT * SCREEN_SIZE;
		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		// Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		if (WIN_FULLSCREEN)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		if (WIN_RESIZABLE)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		if (WIN_BORDERLESS)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		if (WIN_FULLSCREEN_DESKTOP)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		if (window == NULL)
		{
			_LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
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

	_LOG("Destroying SDL window and quitting all SDL systems");

	// Destroy window
	if (window != NULL)
	{
		SDL_DestroyWindow(window);
	}

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
}

uint ModuleWindow::GetRefreshRate() const
{
	uint refreshRate = 0;

	SDL_DisplayMode desktopDisplay;
	if (SDL_GetDesktopDisplayMode(0, &desktopDisplay) == 0)
		refreshRate = desktopDisplay.refresh_rate;
	else
		_LOG("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());

	return refreshRate;
}

void ModuleWindow::GetScreenSize(uint& width, uint& height) const
{
	uint refreshRate = 0;

	SDL_DisplayMode desktopDisplay;
	if (SDL_GetDesktopDisplayMode(0, &desktopDisplay) == 0)
	{
		width = desktopDisplay.w;
		height = desktopDisplay.h;
	}
	else
		_LOG("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
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

void ModuleWindow::SetWindowActive(bool active) 
{
	this->active = active;

	if (active)
		SDL_ShowWindow(window);
	else
		SDL_HideWindow(window);
}

bool ModuleWindow::GetWindowActive() const 
{
	return active;
}