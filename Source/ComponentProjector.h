#ifndef __COMPONENT_PROJECTOR_H__
#define __COMPONENT_PROJECTOR_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib\include\Geometry\Frustum.h"

class ComponentProjector : public Component
{
public:

	ComponentProjector(GameObject* parent);
	ComponentProjector(const ComponentProjector& componentProjector);
	~ComponentProjector();

	void UpdateTransform();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	void SetFOV(float fov);
	float GetFOV() const;
	void SetNearPlaneDistance(float nearPlane);
	void SetFarPlaneDistance(float farPlane);
	void SetAspectRatio(float aspectRatio);

	math::Frustum GetFrustum() const;
	math::float4x4 GetOpenGLViewMatrix() const;
	math::float4x4 GetOpenGLProjectionMatrix() const;

private:

	math::Frustum frustum;
	uint filterMask = 0;
};

#endif