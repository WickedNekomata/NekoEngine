#include "Application.h"
#include "parson/parson.h"

Application::Application() : fpsTrack(FPS_TRACK_SIZE), msTrack(MS_TRACK_SIZE)
{
	window = new ModuleWindow();
	input = new ModuleInput();
	audio = new ModuleAudio(true);
	scene = new ModuleScene();
	renderer3D = new ModuleRenderer3D();
	camera = new ModuleCamera3D();
	gui = new ModuleGui();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(audio);
	AddModule(gui);
	
	// Scenes
	AddModule(scene);

	// Renderer last!
	AddModule(renderer3D);
}

Application::~Application()
{
	for (std::list<Module*>::const_reverse_iterator item = list_modules.rbegin(); item != list_modules.rend(); ++item)
	{
		delete *item;
	}
}

bool Application::Init()
{
	bool ret = true;

	JSON_Value* rootValue = json_parse_file("data.json");
	JSON_Object* data = json_value_get_object(rootValue);

	// Call Init() in all modules
	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end() && ret; ++item)
	{
		JSON_Object* modulejObject = json_object_get_object(data, (*item)->GetName());
		ret = (*item)->Init(modulejObject);
	}

	json_value_free(rootValue);

	// After all Init calls we call Start() in all modules
	CONSOLE_LOG("Application Start --------------");
	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end() && ret; ++item)	
		ret = (*item)->Start();

	perfTimer.Start();
	capFrames = true;
	maxFramerate = 60;

	return ret;
}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;

	PrepareUpdate();
	
	std::list<Module*>::const_iterator item = list_modules.begin();
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PreUpdate(dt);
		++item;
	}

	item = list_modules.begin();
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->Update(dt);
		++item;
	}

	item = list_modules.begin();
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PostUpdate(dt);
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
	{
		ret = (*item)->CleanUp();
	}

	return ret;
}

void Application::CloseApp()
{
	closeApp = true;
}

void Application::LogGui(const char* log) const
{
	if (gui != nullptr)
		gui->LogConsole(log);
}

void Application::PrepareUpdate()
{
	perfTimer.Start();
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

void Application::Load() const
{
	JSON_Value* rootValue = json_parse_file("data.json");
	JSON_Object* data = json_value_get_object(rootValue);

	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end(); ++item)
	{
		JSON_Object* modulejObject = json_object_get_object(data, (*item)->GetName());
		(*item)->LoadStatus(modulejObject);
	}

	json_value_free(rootValue);

	load = false;
}

void Application::Save() const
{
	// TODO: Check if the current object in json exist. if not, create it

	JSON_Value* rootValue = json_parse_file("data.json");
	JSON_Object* data = json_value_get_object(rootValue);

	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end(); ++item)
	{
		JSON_Object* modulejObject = json_object_get_object(data, (*item)->GetName());
		(*item)->SaveStatus(modulejObject);
	}
	json_serialize_to_file_pretty(rootValue,"data.json");
	json_value_free(rootValue);

	save = false;
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}

void Application::SetAppName(const char* name)
{
	appName = name;
	window->SetTitle(name);
}

const char* Application::GetAppName() const 
{
	return appName;
}

void Application::SetOrganizationName(const char* name)
{
	organizationName = name;
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

void Application::SaveState() const
{
	save = true;
}

void Application::LoadState() const
{
	load = true;
}
