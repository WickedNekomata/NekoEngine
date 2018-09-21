#ifndef __MODULE_SCENE_INTRO_H__
#define __MODULE_SCENE_INTRO_H__

#include "Module.h"

#include "Pcg/pcg_variants.h"

class ModuleSceneIntro : public Module
{
public:

	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

private:

	void ShowMenuBar();
	void ShowDemoWindow();
	void ShowInspectorWindow();
	void ShowRandWindow();

private:

	bool showInspector = false;
	bool showDemo = false;
	bool showRandWindow = false;

	pcg32_random_t rng;
};

#endif
