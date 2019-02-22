#ifndef __COMPONENT_LIGHT_H__
#define __COMPONENT_LIGHT_H__

#include "Component.h"

class ComponentLight : public Component
{
	enum class LightTypes { DirectionalLight, PointLight, SpotLight };
public:

	ComponentLight(GameObject* parent);
	ComponentLight(const ComponentLight& componentCamera);
	~ComponentLight();

	void OnUniqueEditor();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

private:
	LightTypes lightType = LightTypes::DirectionalLight;
	int intensity = 1;
};

#endif