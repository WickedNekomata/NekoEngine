#ifndef __COMPONENT_TRANSFORM_H__
#define __COMPONENT_TRANSFORM_H__

#include "Component.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

class ComponentTransform : public Component
{
public:

	ComponentTransform(GameObject* parent);
	virtual ~ComponentTransform();

	void Update() const;

	virtual void OnUniqueEditor() const;

public:
	math::float3 Position = { 0,0,0 };
	math::Quat Rotation = { 1,0,0,0 };
	math::float3 Scale = { 0,0,0 };

};

#endif