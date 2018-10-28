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
	~ComponentCamera();

	void UpdateTransform();

	void OnUniqueEditor();

	void SetFOV(float fov);
	void SetNearPlaneDistance(float nearPlane);
	void SetFarPlaneDistance(float farPlane);

	float* GetOpenGLViewMatrix();
	float* GetOpenGLProjectionMatrix();

	void SetPlay(bool play);
	bool IsPlay() const;

	void SetFrustumCulling(bool frustumCulling);
	bool GetFrustumCulling() const;

	void SetMainCamera(bool mainCamera);
	bool GetMainCamera() const;

public:

	math::Frustum cameraFrustum;

private:

	bool play = false;
	bool frustumCulling = true;
	bool mainCamera = false;
};

#endif