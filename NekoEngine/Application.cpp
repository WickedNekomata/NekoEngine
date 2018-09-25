#include "Application.h"

Application::Application()
{
	window = new ModuleWindow(this);
	input = new ModuleInput(this);
	audio = new ModuleAudio(this, true);
	scene = new ModuleScene(this);
	renderer3D = new ModuleRenderer3D(this);
	camera = new ModuleCamera3D(this);
	gui = new ModuleGui(this);

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

	// Call Init() in all modules
	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end() && ret; ++item)
	{
		ret = (*item)->Init();
	}

	// After all Init calls we call Start() in all modules
	_LOG("Application Start --------------");
	for (std::list<Module*>::const_iterator item = list_modules.begin(); item != list_modules.end() && ret; ++item)
	{
		ret = (*item)->Start();
	}
	
	ms_timer.Start();

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

void Application::PrepareUpdate()
{
	dt = (float)ms_timer.Read() / 1000.0f;
	ms_timer.Start();
}

void Application::FinishUpdate()
{}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}