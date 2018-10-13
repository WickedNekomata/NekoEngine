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

	void SetTarget(const math::float3& target);
	void SetTargetRadius(float targetRadius);

	void LookAt(const math::float3& reference, float radius);
	void LookAround(const math::float3& reference, float pitch, float yaw);

	void Move(const math::float3& movement);
	void MoveTo(const math::float3& position);

	void Zoom(float zoom);

	void SetPlay(bool play);
	bool IsPlay() const;

	float* GetViewMatrix();

private:

	void CalculateViewMatrix();

public:
	
	// Camera
	math::float3 X, Y, Z;
	math::float3 position;
	math::float3 lastX, lastY, lastZ;
	math::float3 lastPosition;

	math::float3 reference;
	float referenceRadius = 0.0f;

private:

	math::float4x4 ViewMatrix, ViewMatrixInverse;

	bool play = false;
};

#endif