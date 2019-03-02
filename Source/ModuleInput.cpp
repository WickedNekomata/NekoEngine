#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleGui.h"
#include "SceneImporter.h"
#include "ModuleWindow.h"
#include "MaterialImporter.h"
#include "ModuleRenderer3D.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_sdl.h"
#include "imgui\imgui_impl_opengl3.h"



#define MAX_KEYS 300

ModuleInput::ModuleInput(bool start_enabled) : Module(start_enabled)
{
	name = "Input";

	keyboard = new KEY_STATE[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KEY_STATE) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KEY_STATE) * MAX_MOUSE_BUTTONS);
}

ModuleInput::~ModuleInput()
{
	delete[] keyboard;
}

bool ModuleInput::Init(JSON_Object* jObject)
{
	DEPRECATED_LOG("Init SDL input event system");

	bool ret = true;

	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		DEPRECATED_LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

update_status ModuleInput::PreUpdate()
{
	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	
	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
			{
				keyboard[i] = KEY_DOWN;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_DOWN);
#endif
			}
			else
			{
				keyboard[i] = KEY_REPEAT;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_REPEAT);
#endif
			}
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
			{
				keyboard[i] = KEY_UP;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_UP);
#endif
			}
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	uint screenSize = App->window->GetScreenSize();
	mouse_x /= screenSize;
	mouse_y /= screenSize;
	mouse_z = 0;

	for (int i = 0; i < 5; ++i)
	{
		if (buttons & SDL_BUTTON(i))
		{
			if (mouse_buttons[i] == KEY_IDLE)
			{
				mouse_buttons[i] = KEY_DOWN;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_DOWN);
#endif
			}
			else
			{
				mouse_buttons[i] = KEY_REPEAT;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_REPEAT);
#endif
			}
		}
		else
		{
			if (mouse_buttons[i] == KEY_REPEAT || mouse_buttons[i] == KEY_DOWN)
			{
				mouse_buttons[i] = KEY_UP;
#ifndef GAMEMODE
				App->gui->AddInput(i, KEY_UP);
#endif
			}
			else
				mouse_buttons[i] = KEY_IDLE;
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
#ifndef GAMEMODE	
		ImGui_ImplSDL2_ProcessEvent(&event);
#endif
		switch (event.type)
		{
		case SDL_MOUSEWHEEL:
			mouse_z = event.wheel.y;
			break;

		case SDL_MOUSEMOTION:
			mouse_x = event.motion.x / screenSize;
			mouse_y = event.motion.y / screenSize;

			mouse_x_motion = event.motion.xrel / screenSize;
			mouse_y_motion = event.motion.yrel / screenSize;
			break;

		case SDL_QUIT:
			App->CloseApp();
			break;

		case SDL_WINDOWEVENT:
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				App->window->SetWindowWidth(event.window.data1);
				App->window->SetWindowHeight(event.window.data2);
				App->window->UpdateWindowSize();
			}
			break;
		}
		case (SDL_DROPFILE):
		{	
			System_Event newEvent;
			newEvent.type = System_Event_Type::FileDropped;			
			strcpy_s(newEvent.fileEvent.file, DEFAULT_BUF_SIZE, event.drop.file);
			App->PushSystemEvent(newEvent);

			SDL_free(event.drop.file);

			break;
		}
		}
	}

	return UPDATE_CONTINUE;
}

bool ModuleInput::CleanUp()
{
	bool ret = true;

	DEPRECATED_LOG("Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	return ret;
}