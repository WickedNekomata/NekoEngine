#ifndef __MODULE_CAMERA_3D_H__
#define __MODULE_CAMERA_3D_H__

#include "Module.h"
#include "Globals.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

class ModuleCamera3D : public Module
{
public:

	ModuleCamera3D(Application* app, bool start_enabled = true);
	~ModuleCamera3D();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void Look(const math::float3 &Position, const math::float3 &Reference, bool RotateAroundReference = false);
	void LookAt(const math::float3 &Spot);
	void Move(const math::float3 &Movement);
	float* GetViewMatrix();

private:

	void CalculateViewMatrix();

public:
	
	math::float3 X, Y, Z, Position, Reference;

private:

	math::float4x4 ViewMatrix, ViewMatrixInverse;
};

#endif