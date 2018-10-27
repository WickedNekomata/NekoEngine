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
	virtual ~ComponentCamera();

	void Update();

	virtual void OnUniqueEditor();

	void SetFOV(float fov);
	void SetNearPlaneDistance(float nearPlane);
	void SetFarPlaneDistance(float farPlane);

	float* GetOpenGLViewMatrix();
	float* GetOpenGLProjectionMatrix();

public:
	math::Frustum cameraFrustum;
};

#endif