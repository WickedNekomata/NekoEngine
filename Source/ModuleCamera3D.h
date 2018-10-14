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

	const float* GetViewMatrix() const;

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

private:

	void CalculateViewMatrix();

public:

	math::float3 X, Y, Z;
	math::float3 position;

private:

	math::float3 lastX, lastY, lastZ;
	math::float3 lastPosition;

	math::float3 reference;
	float referenceRadius = 0.0f;

	math::float4x4 ViewMatrix, ViewMatrixInverse;

	bool play = false;

	float movementSpeed = 0.0f;
	float rotateSensitivity = 0.0f;
	float zoomSpeed = 0.0f;
	float orbitSpeed = 0.0f;
};

#endif