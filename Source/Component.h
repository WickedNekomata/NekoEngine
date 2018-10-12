#ifndef __COMPONENT_H__
#define __COMPONENT_H__

struct GameObject;

class Component
{
private:

	enum class ComponentType { No_type };

public:

	Component(GameObject* parent, ComponentType type = ComponentType::No_type);
	virtual ~Component();

	void Update() const;

	virtual void OnEditor() const;

private:

	ComponentType type;
	GameObject* parent = nullptr;
};

#endif