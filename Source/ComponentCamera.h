#ifndef __COMPONENT_CAMERA_H__
#define __COMPONENT_CAMERA_H__

#include "Component.h"

#include "Globals.h"

#include  "Primitive.h"
#include "MathGeoLib/include/Geometry/Frustum.h"

class ComponentCamera : public Component
{
public:

	ComponentCamera(GameObject* parent);
	ComponentCamera(const ComponentCamera& componentCamera);
	~ComponentCamera();

	void UpdateTransform();

	void OnUniqueEditor();

	void SetFOV(float fov);
	void SetNearPlaneDistance(float nearPlane);
	void SetFarPlaneDistance(float farPlane);
	void SetAspectRatio(float aspectRatio);

	math::float4x4& GetOpenGLViewMatrix() const;
	math::float4x4& GetOpenGLProjectionMatrix() const;

	void SetFrustumCulling(bool frustumCulling);
	bool HasFrustumCulling() const;

	void SetMainCamera(bool mainCamera);
	bool IsMainCamera() const;

	virtual void OnInternalSave(JSON_Object* file);
	virtual void OnLoad(JSON_Object* file);

public:

	math::Frustum frustum;

private:

	bool frustumCulling = true;
	bool mainCamera = true;
};

#endif