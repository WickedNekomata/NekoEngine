#ifndef __COMPONENT_CAMERA_H__
#define __COMPONENT_CAMERA_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib\include\Geometry\Frustum.h"
#include "MathGeoLib\include\Geometry\Ray.h"
#include "MathGeoLib\include\Math\float2.h"

class ComponentCamera : public Component
{
public:

	ComponentCamera(GameObject* parent, bool dummy = false);
	ComponentCamera(const ComponentCamera& componentCamera, GameObject* parent, bool include = true);
	~ComponentCamera();

	void UpdateTransform();

	void OnUniqueEditor();

	void SetFOV(float fov);
	float GetFOV() const;
	void SetNearPlaneDistance(float nearPlane);
	void SetFarPlaneDistance(float farPlane);
	void SetAspectRatio(float aspectRatio);

	math::float4x4 GetOpenGLViewMatrix() const;
	math::float4x4 GetOpenGLProjectionMatrix() const;

	void SetFrustumCulling(bool frustumCulling);
	bool HasFrustumCulling() const;

	void SetMainCamera(bool mainCamera);
	bool IsMainCamera() const;

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

	math::Ray ScreenToRay(math::float2 screenPoint);

public:

	math::Frustum frustum;

private:

	bool frustumCulling = true;
	bool mainCamera = true;
};

#endif