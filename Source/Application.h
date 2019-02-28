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

#include "MathGeoLib/include/Algorithm/Random/LCG.h"
#define FPS_TRACK_SIZE 60
#define MS_TRACK_SIZE 60

enum engine_states
{
	// Game
	ENGINE_PLAY = 1,
	ENGINE_PAUSE,
	ENGINE_STEP,

	// Editor
	ENGINE_EDITOR,
};

struct Module;
struct ModuleResourceManager;
struct ModuleInternalResHandler;
struct MaterialImporter;
struct SceneImporter;
struct AnimationImporter;
struct BoneImporter;
struct ShaderImporter;
struct ModuleCameraEditor;
struct ModuleGui;
struct Raycaster;
struct ModuleWindow;
struct ModuleInput;
struct ModuleScene;
struct ModuleRenderer3D;
struct ModuleFileSystem;
struct ModuleAnimation;
struct ModuleGOs;
struct ModuleTimeManager;
struct ModuleParticle;
struct DebugDrawer;
struct ModuleNavigation;
struct ScriptingModule;
struct ModuleEvents;
struct ModulePhysics;
struct ModuleLayers;

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

	ModuleResourceManager*	  res;
	ModuleInternalResHandler* resHandler;
	MaterialImporter*		  materialImporter;
	SceneImporter*			  sceneImporter;
	ShaderImporter*			  shaderImporter;
	mutable BoneImporter*	  boneImporter;
	mutable AnimationImporter*animImporter;
	ModuleParticle*			  particle;

#ifndef GAMEMODE
	ModuleCameraEditor*		  camera;
	ModuleGui*				  gui;
							  
	Raycaster*				  raycaster;
#endif // GAME

	ModuleWindow*			  window;
	ModuleInput*			  input;
	ModuleScene*			  scene;
	ModuleRenderer3D*		  renderer3D;
	ModuleFileSystem*		  fs;
	ModuleGOs*				  GOs;
	ModuleTimeManager*		  timeManager;
	ScriptingModule*		  scripting;
	ModuleEvents*			  events;
	ModulePhysics*			  physics;
	ModuleAnimation*		  animation;
	DebugDrawer*			  debugDrawer;
	ModuleNavigation*		  navigation;
	ModuleLayers*			  layers;
							  
	pcg32_random_t			  rng;

	bool firstFrame = true;
	math::LCG randomMathLCG; //Cant be private with const Get

private:

	// Framerate
	PerfTimer			perfTimer;
	uint				maxFramerate;
	double				lastFrameMs = 0;
	double				dt = 0;
	double				fps = 0;
	bool				capFrames = false;
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
