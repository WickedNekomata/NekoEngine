#ifndef __APPLICATION_H_
#define __APPLICATION_H_

#include "GameMode.h"

#include <list>
#include <vector>

#include "Globals.h"
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
#include "MaterialImporter.h"
#include "SceneImporter.h"
#include "DebugDrawer.h"
#include "Raycaster.h"

#include "PerfTimer.h"

#include "PCG/pcg_variants.h"

#define FPS_TRACK_SIZE 60
#define MS_TRACK_SIZE 60

enum engine_states
{
	// Game
	ENGINE_PLAY = 1,
	ENGINE_WANTS_PLAY,
	ENGINE_PAUSE,
	ENGINE_WANTS_PAUSE,
	ENGINE_TICK,
	ENGINE_WANTS_TICK,

	// Editor
	ENGINE_EDITOR,
	ENGINE_WANTS_EDITOR
};

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

	float GetDt() const;

	void Play();
	void Pause();
	void Tick();

	engine_states GetEngineState() const;
	bool IsPlay() const;
	bool IsPause() const;
	bool IsEditor() const;

	void SaveState() const;
	void LoadState() const;

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

	void Load();
	void Save() const;

public:

	MaterialImporter*	materialImporter;
	SceneImporter*		sceneImporter;
#ifndef GAMEMODE
	
	ModuleCameraEditor*	camera;
	ModuleGui*			gui;

	Raycaster*			raycaster;
#endif // GAME

	ModuleWindow*		window;
	ModuleInput*		input;
	ModuleScene*		scene;
	ModuleRenderer3D*	renderer3D;
	ModuleFileSystem*	filesystem;
	ModuleGOs*			GOs;
	ModuleTimeManager*	timeManager;

	DebugDrawer*		debugDrawer;

	pcg32_random_t		rng;

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

	engine_states engineState = engine_states::ENGINE_EDITOR;
};

extern Application* App;

#endif