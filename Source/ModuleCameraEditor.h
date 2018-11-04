#ifndef __MODULE_CAMERA_EDITOR_H__
#define __MODULE_CAMERA_EDITOR_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Module.h"
#include "Globals.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

class ComponentCamera;

class ModuleCameraEditor : public Module
{
public:

	ModuleCameraEditor(bool start_enabled = true);
	~ModuleCameraEditor();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status Update();
	bool CleanUp();

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	void SetReference(const math::float3& reference);
	void SetReferenceRadius(float referenceRadius);

	void LookAt(const math::float3& reference, float radius) const;
	void LookAround(const math::float3& reference, float pitch, float yaw) const;

public:

	ComponentCamera* camera = nullptr;

private:

	math::float3 reference = { 0.0f,0.0f,0.0f };

	float movementSpeed = 0.0f;
	float rotationSpeed = 0.0f;
	float zoomSpeed = 0.0f;
	float referenceRadius = 0.0f;
};

#endif

#endif // GAME