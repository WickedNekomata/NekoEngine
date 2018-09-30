#ifndef __MODULE_WINDOW_H__
#define __MODULE_WINDOW_H__

#include "Module.h"
#include "SDL/include/SDL.h"

class ModuleWindow : public Module
{
public:

	ModuleWindow(bool start_enabled = true);
	~ModuleWindow();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	void SetTitle(const char* title);
	const char* GetTitle() const;

	void SetWindowBrightness(float brightness) const;
	float GetWindowBrightness() const;

	void SetScreenSize(uint size);
	uint GetScreenSize() const;
	void SetWindowWidth(uint width);
	uint GetWindowWidth() const;
	void SetWindowHeight(uint height);
	uint GetWindowHeight() const;
	void UpdateWindowSize() const;

	uint GetRefreshRate() const;
	void GetScreenSize(uint& width, uint& height) const;
	
	void SetFullscreenWindow(bool fullscreen);
	bool GetFullscreenWindow() const;
	void SetFullDesktopWindow(bool fullDesktop);
	bool GetFullDesktopWindow() const;
	void SetResizableWindow(bool resizable);
	bool GetResizableWindow() const;
	void SetBorderlessWindow(bool borderless);
	bool GetBorderlessWindow() const;

	void SetWindowActive(bool active);
	bool GetWindowActive() const;

public:
	
	SDL_Window* window; // The window we'll be rendering to	
	SDL_Surface* screen_surface;// The surface contained by the window

private:

	uint width = 0;
	uint height = 0;
	uint size = 1;

	bool fullscreen = false;
	bool resizable = false;
	bool borderless = false;
	bool fullDesktop = false;

	const char* title = nullptr;

	bool active = true;
};

#endif