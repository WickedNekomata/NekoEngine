#ifndef __COMPONENT_NAV_AGENT_H__
#define __COMPONENT_NAV_AGENT_H__

#include "Component.h"

class ComponentNavAgent : public Component
{
public:

	ComponentNavAgent(GameObject* parent);
	ComponentNavAgent(const ComponentNavAgent& componentTransform);
	~ComponentNavAgent();

	void Update();

	void OnUniqueEditor();

	void AddAgent();

	void SetDestination();

	virtual void OnInternalSave(JSON_Object* file);
	virtual void OnLoad(JSON_Object* file);
};

#endif