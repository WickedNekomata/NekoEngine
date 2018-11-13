#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"

#include "Globals.h"

#include <vector>
#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Math/float4x4.h"

struct MaterialResource
{
	uint res = 0;
	math::float4x4 matrix = math::float4x4::identity;

	bool operator==(const MaterialResource rhs)
	{
		return this->res == rhs.res && matrix.Equals(rhs.matrix);
	}
};

class ComponentMaterial : public Component
{
public:

	ComponentMaterial(GameObject* parent);
	ComponentMaterial(const ComponentMaterial& componentMaterial);
	~ComponentMaterial();

	void Update();

	void SetResource(uint res_uuid, uint position);

	void OnUniqueEditor();

	virtual void OnInternalSave(JSON_Object* file);
	virtual void OnLoad(JSON_Object* file);

private:

	void EditCurrentResMatrixByIndex(int i);

public:

	std::vector<MaterialResource> res;
};

#endif