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
	int  GetIndex() const { return index; }
	bool UpdateParams() const;
	void SetDestination(const float* pos) const;
	bool IsWalking();

	uint GetInternalSerializationBytes();
	virtual void OnInternalSave(char*& cursor);
	virtual void OnInternalLoad(char*& cursor);

private:
	float radius = 1.0f;
	float height = 1.0f;
	float maxAcceleration = 8.0f;
	float maxSpeed = 3.5f;
	unsigned int params = 0;
	float separationWeight = 2.0f;
	int avoidanceQuality = 3;

	int index = -1;
};

#endif