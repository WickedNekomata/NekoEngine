#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ModuleRenderer3D.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl2.h"

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
	CONSOLE_LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		CONSOLE_LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

update_status ModuleInput::PreUpdate(float dt)
{
	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	
	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
				keyboard[i] = KEY_DOWN;
			else
				keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
				keyboard[i] = KEY_UP;
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
				mouse_buttons[i] = KEY_DOWN;
			else
				mouse_buttons[i] = KEY_REPEAT;
		}
		else
		{
			if (mouse_buttons[i] == KEY_REPEAT || mouse_buttons[i] == KEY_DOWN)
				mouse_buttons[i] = KEY_UP;
			else
				mouse_buttons[i] = KEY_IDLE;
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

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
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				App->renderer3D->OnResize(event.window.data1, event.window.data2);
			break;
		}
		case (SDL_DROPFILE):
		{
			std::string droppedFilePath = event.drop.file;

			if (droppedFilePath.find(".fbx") != std::string::npos || droppedFilePath.find(".FBX") != std::string::npos
				|| droppedFilePath.find(".obj") != std::string::npos || droppedFilePath.find(".OBJ") != std::string::npos)
			{
				App->sceneImporter->Import(nullptr, droppedFilePath.data(), nullptr);
			}
			else if (droppedFilePath.find(".png") != std::string::npos || droppedFilePath.find(".PNG") != std::string::npos
				|| droppedFilePath.find(".dds") != std::string::npos || droppedFilePath.find(".DDS") != std::string::npos)
			{

				App->materialImporter->Import(nullptr, droppedFilePath.data(), nullptr);
			}

			SDL_free((char*)droppedFilePath.data());

			break;
		}
		}
	}

	return UPDATE_CONTINUE;
}

bool ModuleInput::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG("Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	return ret;
}