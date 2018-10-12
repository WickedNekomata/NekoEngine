#ifndef __APPLICATION_H_
#define __APPLICATION_H_

#include <list>
#include <vector>

#include "Globals.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleGui.h"
#include "ModuleFileSystem.h"
#include "ModuleMeshes.h"
#include "ModuleTextures.h"
#include "ModuleGOs.h"

#include "PerfTimer.h"

#define FPS_TRACK_SIZE 60
#define MS_TRACK_SIZE 60

class Application
{
public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

	void CloseApp();
	void LogGui(const char* log) const;

	void SetAppName(const char* title);
	const char* GetAppName() const;
	void SetOrganizationName(const char* title);
	const char* GetOrganizationName() const;

	void SetCapFrames(bool capFrames);
	bool GetCapFrames() const;
	void SetMaxFramerate(uint maxFramerate);
	uint GetMaxFramerate() const;
	void AddFramerateToTrack(float fps);
	std::vector<float> GetFramerateTrack() const;
	void AddMsToTrack(float ms);
	std::vector<float> GetMsTrack() const;

	void SaveState() const;
	void LoadState() const;

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

	void Load();
	void Save() const;

public:

	ModuleWindow*		window;
	ModuleInput*		input;
	ModuleScene*		scene;
	ModuleRenderer3D*	renderer3D;
	ModuleCamera3D*		camera;
	ModuleGui*			gui;
	ModuleFileSystem*	filesystem;
	ModuleMeshes*		meshImporter;
	ModuleTextures*		tex;
	ModuleGOs*			GOs;

private:

	// Framerate
	PerfTimer			perfTimer;
	uint				maxFramerate;
	double				lastFrameMs = 0;
	double				dt = 0;
	double				fps = 0;
	bool				capFrames = 0;
	std::vector<float>	fpsTrack;
	std::vector<float>	msTrack;

	std::list<Module*>	list_modules;

	const char*			appName = nullptr;
	const char*			organizationName = nullptr;

	bool				closeApp = false;

	mutable bool		save = false;
	mutable bool		load = false;
};

extern Application* App;

#endif