#include "Application.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "ModuleCameraEditor.h"
#include "ModuleGui.h"
#include "ModuleFileSystem.h"
#include "ModuleGOs.h"
#include "ModuleTimeManager.h"
#include "ModuleResourceManager.h"
#include "ModuleParticles.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"
#include "BoneImporter.h"
#include "ShaderImporter.h"
#include "DebugDrawer.h"
#include "Raycaster.h"
#include "ModuleNavigation.h"
#include "ScriptingModule.h"
#include "ModuleEvents.h"
#include "ModulePhysics.h"
#include "Layers.h"

#include "parson\parson.h"
#include "PCG\entropy.h"

Application::Application() : fpsTrack(FPS_TRACK_SIZE), msTrack(MS_TRACK_SIZE)
{
	window = new ModuleWindow();
	input = new ModuleInput();
	scene = new ModuleScene();
	renderer3D = new ModuleRenderer3D();
	fs = new ModuleFileSystem();
	GOs = new ModuleGOs();
	timeManager = new ModuleTimeManager();
	res = new ModuleResourceManager();
	debugDrawer = new DebugDrawer();
	materialImporter = new MaterialImporter();
	sceneImporter = new SceneImporter();
	boneImporter = new BoneImporter();
	shaderImporter = new ShaderImporter();
	navigation = new ModuleNavigation();
	particle = new ModuleParticle();
	scripting = new ScriptingModule();
	events = new ModuleEvents();
	physics = new ModulePhysics();
	layers = new Layers();

#ifndef GAMEMODE
	camera = new ModuleCameraEditor();
	gui = new ModuleGui();
	raycaster = new Raycaster();
#endif // GAME

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order
	AddModule(res);
	AddModule(timeManager);

#ifndef GAMEMODE
	AddModule(camera);
	AddModule(gui);
#endif // GAME

	AddModule(particle);
	AddModule(physics);
	AddModule(GOs);
	AddModule(fs);
	AddModule(window);
	AddModule(input);
	AddModule(scene);
	AddModule(scripting);

	// Renderer last!
	AddModule(renderer3D);

	//No, I'm last ;)
	AddModule(events);
}

Application::~Application()
{
	RELEASE_ARRAY(appName);
	RELEASE_ARRAY(organizationName);

	for (std::list<Module*>::const_reverse_iterator item = list_modules.rbegin(); item != list_modules.rend(); ++item)
		delete *item;

#ifndef GAMEMODE
	RELEASE(raycaster);
#endif

	RELEASE(debugDrawer);
	RELEASE(materialImporter);
	RELEASE(sceneImporter);
	RELEASE(boneImporter);
	RELEASE(shaderImporter);
	RELEASE(layers);
}

bool Application::Init()
{
	bool ret = true;

	uint64_t seeds[2];
	entropy_getbytes((void*)seeds, sizeof(seeds));
	pcg32_srandom_r(&rng, seeds[0], seeds[1]);

	// Read config file
	char* buf;
	uint size = fs->Load("Settings/config.json", &buf);
	if (size > 0)
	{
		JSON_Value* rootValue = json_parse_string(buf);
		delete[] buf;
		JSON_Object* data = json_value_get_object(rootValue);

		// Loading Application data
		JSON_Object* modulejObject = json_object_get_object(data, "Application");
		SetCapFrames(json_object_get_boolean(modulejObject, "Cap Frames"));
		SetMaxFramerate(json_object_get_number(modulejObject, "Max FPS"));
		SetAppName(json_object_get_string(modulejObject, "Title"));
		SetOrganizationName(json_object_get_string(modulejObject, "Organization"));

		// Call Init() in all modules
		for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end() && ret; ++item)
		{
			modulejObject = json_object_get_object(data, (*item)->GetName());
			ret = (*item)->Init(modulejObject);
		}

		json_value_free(rootValue);
	}
	else
	{
		for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end() && ret; ++item)
		{
			ret = (*item)->Init(nullptr);
		}
	}

	// After all Init calls we call Start() in all modules
	DEPRECATED_LOG("Application Start --------------");
	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end() && ret; ++item)
		ret = (*item)->Start();

	firstFrame = false;

	perfTimer.Start();

	return ret;
}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;

	PrepareUpdate();

	// Send last frame events
	PopEvents();

	std::list<Module*>::const_iterator item = list_modules.begin();
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsActive())
			ret = (*item)->PreUpdate();
		++item;
	}

	item = list_modules.begin();
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsActive())
			ret = (*item)->Update();
		++item;
	}

	item = list_modules.begin();
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsActive())
			ret = (*item)->PostUpdate();
		++item;
	}

	FinishUpdate();

	if (save)
		Save();

	if (load)
		Load();

	if (closeApp)
		ret = UPDATE_STOP;

	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;

	for (std::list<Module*>::const_reverse_iterator item = list_modules.rbegin(); item != list_modules.rend() && ret; ++item)
		ret = (*item)->CleanUp();

	return ret;
}

void Application::CloseApp()
{
	closeApp = true;
}

void Application::LogGui(const char* log) const
{
#ifndef GAMEMODE
	if (gui != nullptr)
		gui->LogConsole(log);
#endif
}

void Application::PrepareUpdate()
{
	perfTimer.Start();

	switch (engineState)
	{
	case engine_states::ENGINE_WANTS_PLAY:
	{
		bool ret = true;

		// Save the scene and enter game mode
		for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end(); ++item)
		{
			if (!(*item)->OnGameMode())
			{
				App->GOs->DeleteTemporaryGameObjects();

				engineState = engine_states::ENGINE_EDITOR;
				ret = false;
				break;
			}
		}

		if (ret)
		{
			engineState = engine_states::ENGINE_PLAY;			
		}
			
		break;
	}
	case engine_states::ENGINE_WANTS_PAUSE:

		engineState = engine_states::ENGINE_PAUSE;
		break;

	case engine_states::ENGINE_WANTS_EDITOR:
	{
		bool ret = true;

		// Load the scene and enter editor mode
		for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end(); ++item)
		{
			if (!(*item)->OnEditorMode())
			{
				engineState = engine_states::ENGINE_PLAY;
				ret = false;
				break;
			}
		}

		if (ret)
		{
			engineState = engine_states::ENGINE_EDITOR;

			System_Event event;
			event.type = System_Event_Type::Stop;
			PushSystemEvent(event);
		}
			
		break;
	}
	case engine_states::ENGINE_WANTS_STEP:

		engineState = engine_states::ENGINE_STEP;
		break;

	case engine_states::ENGINE_STEP:

		engineState = engine_states::ENGINE_WANTS_PAUSE;
		break;

	default:
		break;
	}

	timeManager->PrepareUpdate();
}

void Application::FinishUpdate()
{
	lastFrameMs = perfTimer.ReadMs();

	if (!renderer3D->GetVSync() && capFrames)
	{
		double desiredFrameMs = 1000.0 / maxFramerate;

		if (lastFrameMs < desiredFrameMs)
			SDL_Delay(desiredFrameMs - lastFrameMs);

		lastFrameMs = perfTimer.ReadMs();
	}

	AddMsToTrack(lastFrameMs);

	fps = 1000.0 / lastFrameMs;
	AddFramerateToTrack(fps);

	dt = 1.0 / fps;
}

void Application::Load()
{
	// Read config file
	char* buf;
	uint size = fs->Load("Settings/config.json", &buf);
	if (size > 0)
	{
		JSON_Value* rootValue = json_parse_string(buf);
		delete[] buf;
		JSON_Object* data = json_value_get_object(rootValue);

		JSON_Object* modulejObject = json_object_get_object(data, "Application");

		SetAppName(json_object_get_string(modulejObject, "Title"));
		window->SetTitle(GetAppName());
		SetOrganizationName(json_object_get_string(modulejObject, "Organization"));
		SetCapFrames(json_object_get_boolean(modulejObject, "Cap Frames"));
		SetMaxFramerate(json_object_get_number(modulejObject, "Max FPS"));

		for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end(); ++item)
		{
			modulejObject = json_object_get_object(data, (*item)->GetName());
			(*item)->LoadStatus(modulejObject);
		}

		json_value_free(rootValue);
	}

	load = false;
}

void Application::Save() const
{
	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	// Saving App data
	JSON_Value* newValue = json_value_init_object();
	JSON_Object* objModule = json_value_get_object(newValue);
	json_object_set_value(rootObject, "Application", newValue);

	json_object_set_string(objModule, "Title", GetAppName());
	json_object_set_string(objModule, "Organization", GetOrganizationName());
	json_object_set_boolean(objModule, "Cap Frames", GetCapFrames());
	json_object_set_number(objModule, "Max FPS", GetMaxFramerate());

	// Saving Modules Data
	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end(); ++item)
	{
		newValue = json_value_init_object();
		objModule = json_value_get_object(newValue);
		json_object_set_value(rootObject, (*item)->GetName(), newValue);
		(*item)->SaveStatus(objModule);
	}

	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);
	fs->Save("Settings/config.json", buf, sizeBuf);
	delete[] buf;
	json_value_free(rootValue);

	save = false;
}

void Application::PopEvents()
{
	while (systemEvents.size() > 0)
	{
		System_Event poppedEvent = systemEvents.front();
		for (auto it = list_modules.begin(); it != list_modules.end(); ++it)
			(*it)->OnSystemEvent(poppedEvent);

		systemEvents.pop();
	}
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}

void Application::SetAppName(const char* name)
{
	appName = new char[INPUT_BUF_SIZE];
	strcpy_s((char*)appName, INPUT_BUF_SIZE, name);

	if (window != nullptr)
		window->SetTitle(name);
}

const char* Application::GetAppName() const
{
	return appName;
}

void Application::SetOrganizationName(const char* name)
{
	organizationName = new char[INPUT_BUF_SIZE];
	strcpy_s((char*)organizationName, INPUT_BUF_SIZE, name);
}

const char* Application::GetOrganizationName() const
{
	return organizationName;
}

void Application::SetCapFrames(bool capFrames)
{
	this->capFrames = capFrames;
}

bool Application::GetCapFrames() const
{
	return capFrames;
}

void Application::SetMaxFramerate(uint maxFramerate)
{
	this->maxFramerate = maxFramerate;
}

uint Application::GetMaxFramerate() const
{
	return this->maxFramerate;
}

void Application::AddFramerateToTrack(float fps)
{
	for (uint i = fpsTrack.size() - 1; i > 0; --i)
		fpsTrack[i] = fpsTrack[i - 1];

	fpsTrack[0] = fps;
}

std::vector<float> Application::GetFramerateTrack() const
{
	return fpsTrack;
}

void Application::AddMsToTrack(float ms)
{
	for (uint i = msTrack.size() - 1; i > 0; --i)
		msTrack[i] = msTrack[i - 1];

	msTrack[0] = ms;
}

std::vector<float> Application::GetMsTrack() const
{
	return msTrack;
}

float Application::GetDt() const
{
	return dt;
}

void Application::Play()
{
	switch (engineState)
	{
	case engine_states::ENGINE_PLAY:
	case engine_states::ENGINE_PAUSE:
	{
		// Enter editor mode
		engineState = engine_states::ENGINE_WANTS_EDITOR;

		System_Event event;
		event.type = System_Event_Type::Stop;
		PushSystemEvent(event);

		break;
	}
	
	case engine_states::ENGINE_EDITOR:
	{
		// Enter play mode
		engineState = engine_states::ENGINE_WANTS_PLAY;

		System_Event event;
		event.type = System_Event_Type::Play;
		PushSystemEvent(event);
		break;
	}
		
	default:
		break;
	}
}

void Application::Pause()
{
	switch (engineState)
	{
	case engine_states::ENGINE_PLAY:

		// Pause
		engineState = engine_states::ENGINE_WANTS_PAUSE;
		break;

	case engine_states::ENGINE_PAUSE:

		// Play again
		engineState = engine_states::ENGINE_PLAY;
		break;

	default:
		break;
	}
}

void Application::Step()
{
	switch (engineState)
	{
	case engine_states::ENGINE_PLAY:

		// Stop and tick (step 1 frame)
		engineState = engine_states::ENGINE_WANTS_STEP;
		break;

	case engine_states::ENGINE_PAUSE:

		// Tick (step 1 frame)
		engineState = engine_states::ENGINE_STEP;
		break;

	default:
		break;
	}
}

engine_states Application::GetEngineState() const
{
	return engineState;
}

bool Application::IsPlay() const
{
	return engineState == engine_states::ENGINE_PLAY;
}

bool Application::IsPause() const
{
	return engineState == engine_states::ENGINE_PAUSE;
}

bool Application::IsStep() const
{
	return engineState == engine_states::ENGINE_STEP;
}

bool Application::IsEditor() const
{
	return engineState == engine_states::ENGINE_EDITOR;
}

uint Application::GenerateRandomNumber() const
{
	return pcg32_random_r(&(App->rng));
}

void Application::SaveState() const
{
	save = true;
}

void Application::LoadState() const
{
	load = true;
}

void Application::PushSystemEvent(System_Event event)
{
	systemEvents.push(event);
}