#ifndef __COMPONENT_PROJECTOR_H__
#define __COMPONENT_PROJECTOR_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib\include\Geometry\Frustum.h"

/*
Texture:
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
*/

class ComponentProjector : public Component
{
public:

	ComponentProjector(GameObject* parent);
	ComponentProjector(const ComponentProjector& componentProjector, GameObject* parent);
	~ComponentProjector();

	void UpdateTransform();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	void OnInternalSave(char*& cursor);
	void OnInternalLoad(char*& cursor);

	// ----------------------------------------------------------------------------------------------------

	// Sets
	void SetFOV(float fov);
	float GetFOV() const;
	void SetNearPlaneDistance(float nearPlane);
	void SetFarPlaneDistance(float farPlane);
	void SetAspectRatio(float aspectRatio);

	void SetMaterialRes(uint materialUuid);
	uint GetMaterialRes() const;
	void SetFilterMask(uint filterMask);
	uint GetFilterMask() const;

	// Gets
	math::Frustum GetFrustum() const;
	math::float4x4 GetOpenGLViewMatrix() const;
	math::float4x4 GetOpenGLProjectionMatrix() const;

private:

	math::Frustum frustum;
	uint materialRes = 0;
	uint filterMask = 0;
};

#endif