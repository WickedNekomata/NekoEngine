#ifndef __APPLICATION_H_
#define __APPLICATION_H_

#include "GameMode.h"

#include <list>
#include <vector>
#include <queue>

#include "Globals.h"
#include "EventSystem.h"

#include "PerfTimer.h"

#include "PCG\pcg_variants.h"

#define FPS_TRACK_SIZE 60
#define MS_TRACK_SIZE 60

enum engine_states
{
	// Game
	ENGINE_PLAY = 1,
	ENGINE_WANTS_EDITOR,
	ENGINE_PAUSE,
	ENGINE_WANTS_PAUSE,
	ENGINE_STEP,
	ENGINE_WANTS_STEP,

	// Editor
	ENGINE_EDITOR,
	ENGINE_WANTS_PLAY
};

struct Module;
struct ModuleResourceManager;
struct MaterialImporter;
struct SceneImporter;
struct ShaderImporter;
struct ModuleCameraEditor;
struct ModuleGui;
struct Raycaster;
struct ModuleWindow;
struct ModuleInput;
struct ModuleScene;
struct ModuleRenderer3D;
struct ModuleFileSystem;
struct ModuleGOs;
struct ModuleTimeManager;
struct DebugDrawer;
struct NMSupplier;
struct ModuleNavigation;

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
	void Step();

	engine_states GetEngineState() const;
	bool IsPlay() const;
	bool IsPause() const;
	bool IsStep() const;
	bool IsEditor() const;

	uint GenerateRandomNumber() const;

	void SaveState() const;
	void LoadState() const;

	void PushSystemEvent(System_Event event);

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

	void Load();
	void Save() const;

	void PopEvents();

public:

	ModuleResourceManager*	res;
	MaterialImporter*		materialImporter;
	SceneImporter*			sceneImporter;
	ShaderImporter*			shaderImporter;

#ifndef GAMEMODE	
	ModuleCameraEditor*		camera;
	ModuleGui*				gui;

	Raycaster*				raycaster;
#endif // GAME

	ModuleWindow*			window;
	ModuleInput*			input;
	ModuleScene*			scene;
	ModuleRenderer3D*		renderer3D;
	ModuleFileSystem*		fs;
	ModuleGOs*				GOs;
	ModuleTimeManager*		timeManager;
	DebugDrawer*			debugDrawer;
	NMSupplier*				nmSupplier;
	ModuleNavigation*		navigation;

	pcg32_random_t			rng;

	bool firstFrame = true;

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
	std::queue<System_Event> systemEvents;

	const char*			appName = nullptr;
	const char*			organizationName = nullptr;

	bool				closeApp = false;

	mutable bool		save = false;
	mutable bool		load = false;

	engine_states engineState = engine_states::ENGINE_EDITOR;
};

extern Application* App;

#endif