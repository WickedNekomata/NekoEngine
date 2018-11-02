#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"

#include "Globals.h"

#include <vector>

struct Texture;

class ComponentMaterial : public Component
{
public:

	ComponentMaterial(GameObject* parent);
	~ComponentMaterial();

	void Update();

	void OnUniqueEditor();

	virtual void OnInternalSave(JSON_Object* file);
	virtual void OnLoad(JSON_Object* file);

	void OnGameMode();
	void OnEditorMode();

public:

	std::vector<Texture*> textures;
};

#endif