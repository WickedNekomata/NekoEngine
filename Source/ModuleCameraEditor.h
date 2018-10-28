#ifndef __MODULE_CAMERA_EDITOR_H__
#define __MODULE_CAMERA_EDITOR_H__

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
	update_status Update(float dt);
	bool CleanUp();

public:

	ComponentCamera* camera = nullptr;
};

#endif