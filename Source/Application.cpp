#include "Application.h"

#include "parson/parson.h"

Application::Application() : fpsTrack(FPS_TRACK_SIZE), msTrack(MS_TRACK_SIZE)
{
	window = new ModuleWindow();
	input = new ModuleInput();
	scene = new ModuleScene();
	renderer3D = new ModuleRenderer3D();
	camera = new ModuleCameraEditor();
	gui = new ModuleGui();
	filesystem = new ModuleFileSystem();
	GOs = new ModuleGOs();
	materialImporter = new MaterialImporter();
	sceneImporter = new SceneImporter();
	debugDrawer = new DebugDrawer();
	raycaster = new Raycaster();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(GOs);
	AddModule(filesystem);
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(gui);
	
	// Scenes
	AddModule(scene);

	// Renderer last!
	AddModule(renderer3D);
}

Application::~Application()
{
	RELEASE_ARRAY(appName);
	RELEASE_ARRAY(organizationName);

	for (std::list<Module*>::const_reverse_iterator item = list_modules.rbegin(); item != list_modules.rend(); ++item)
		delete *item;
}

bool Application::Init()
{
	bool ret = true;

	// Read config file
	char* buf;
	uint size = App->filesystem->Load("Assets/config.json", &buf);
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
	CONSOLE_LOG("Application Start --------------");
	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end() && ret; ++item)	
		ret = (*item)->Start();

	perfTimer.Start();

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
		if ((*item)->IsActive())
			ret = (*item)->PreUpdate(dt);
		++item;
	}

	item = list_modules.begin();
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsActive())
			ret = (*item)->Update(dt);
		++item;
	}

	item = list_modules.begin();
	while (item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsActive())
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
		ret = (*item)->CleanUp();

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

void Application::Load()
{
	// Read config file
	char* buf;
	uint size = App->filesystem->Load("Assets/config.json", &buf);
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
		SetMaxFramerate(json_object_get_boolean(modulejObject, "Max FPS"));

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

	json_object_set_string(objModule, "Title", App->GetAppName());
	json_object_set_string(objModule, "Organization", App->GetOrganizationName());
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
	filesystem->Save("config.json", buf, sizeBuf);
	delete[] buf;
	json_value_free(rootValue);

	save = false;
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

void Application::SaveState() const
{
	save = true;
}

void Application::LoadState() const
{
	load = true;
}
