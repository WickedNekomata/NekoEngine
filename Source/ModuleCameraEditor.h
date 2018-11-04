#ifndef __MODULE_CAMERA_EDITOR_H__
#define __MODULE_CAMERA_EDITOR_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Module.h"
#include "Globals.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

struct ComponentCamera;

class ModuleCameraEditor : public Module
{
public:

	ModuleCameraEditor(bool start_enabled = true);
	~ModuleCameraEditor();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status Update();
	bool CleanUp();

	void LookAt(math::float3 focus);

public:

	ComponentCamera* camera = nullptr;
};

#endif

#endif // GAME