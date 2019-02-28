#ifndef __COMPONENT_TRANSFORM_H__
#define __COMPONENT_TRANSFORM_H__

#include "Component.h"

#include "Globals.h"

#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\Quat.h"
#include "MathGeoLib\include\Math\float4x4.h"

#define TRANSFORMINPUTSWIDTH 50.0f

class ComponentTransform : public Component
{
public:

	ComponentTransform(GameObject* parent);
	ComponentTransform(const ComponentTransform& componentTransform, GameObject* parent);
	~ComponentTransform();

	void Update();

	void OnEditor();

	void OnUniqueEditor();

	void SavePrevTransform(const math::float4x4 & prevTransformMat);

	math::float4x4& GetMatrix() const;
	math::float4x4& GetGlobalMatrix() const;
	void SetMatrixFromGlobal(math::float4x4& globalMatrix);

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

public:

	math::float3 position = math::float3::zero;
	math::Quat rotation = math::Quat::identity;
	math::float3 scale = math::float3::one;

	bool dragTransform = false;
};

#endif