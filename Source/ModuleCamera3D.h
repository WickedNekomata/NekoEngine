#ifndef __MODULE_CAMERA_3D_H__
#define __MODULE_CAMERA_3D_H__

#include "Module.h"
#include "Globals.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

class ModuleCamera3D : public Module
{
public:

	ModuleCamera3D(bool start_enabled = true);
	~ModuleCamera3D();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	// Camera position, Target to look at, Up vector (WORLD SPACE)
	void LookAt(const math::float3 &reference, const math::float3 &up);
	void LookAround(float pitch, float yaw);

	void Move(const math::float3 &movement);
	void MoveTo(const math::float3 &position);

	void Zoom(float zoom);

	float* GetViewMatrix();

private:

	void CalculateViewMatrix();

public:
	
	// Camera
	math::float3 X, Y, Z; // Z = direction
	math::float3 position;
	math::float3 reference; // target

private:

	math::float4x4 ViewMatrix, ViewMatrixInverse;
};

#endif