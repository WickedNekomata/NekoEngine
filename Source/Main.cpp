#include <stdlib.h>
#include "Application.h"
#include "Globals.h"

#include "SDL\include\SDL.h"
#include "Brofiler\Brofiler.h"

#pragma comment(lib, "SDL\\libx86\\SDL2.lib")
#pragma comment(lib, "SDL\\libx86\\SDL2main.lib")
#pragma comment (lib, "Brofiler\\ProfilerCore32.lib")

enum main_states
{
	MAIN_CREATION,
	MAIN_START,
	MAIN_UPDATE,
	MAIN_FINISH,
	MAIN_EXIT
};

Application* App = NULL;

int main(int argc, char ** argv)
{
	DEPRECATED_LOG("Starting game...");

	int main_return = EXIT_FAILURE;
	main_states state = MAIN_CREATION;

	while (state != MAIN_EXIT)
	{
		switch (state)
		{
		case MAIN_CREATION:

			DEPRECATED_LOG("-------------- Application Creation --------------");
			App = new Application();
			state = MAIN_START;
			break;

		case MAIN_START:

			DEPRECATED_LOG("-------------- Application Init --------------");
			if (App->Init() == false)
			{
				DEPRECATED_LOG("Application Init exits with ERROR");
				state = MAIN_EXIT;
			}
			else
			{
				state = MAIN_UPDATE;
				DEPRECATED_LOG("-------------- Application Update --------------");
			}

			break;

		case MAIN_UPDATE:
		{
			BROFILER_FRAME("Main Loop");
			int update_return = App->Update();

			if (update_return == UPDATE_ERROR)
			{
				DEPRECATED_LOG("Application Update exits with ERROR");
				state = MAIN_EXIT;
			}

			if (update_return == UPDATE_STOP)
				state = MAIN_FINISH;
		}
			break;

		case MAIN_FINISH:

			DEPRECATED_LOG("-------------- Application CleanUp --------------");
			if (App->CleanUp() == false)
			{
				DEPRECATED_LOG("Application CleanUp exits with ERROR");
			}
			else
				main_return = EXIT_SUCCESS;

			state = MAIN_EXIT;

			break;

		}
	}

	delete App;

	return main_return;
}