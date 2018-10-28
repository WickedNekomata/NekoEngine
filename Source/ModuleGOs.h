#ifndef __MODULE_GOS_H__
#define __MODULE_GOS_H__

#include "Module.h"

#include <vector>

struct GameObject;
struct Component;

class ModuleGOs : public Module
{
public:

	ModuleGOs(bool start_enabled = true);
	~ModuleGOs();

	bool Init(JSON_Object* jObject);
	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	GameObject* CreateGameObject(char* name, GameObject* parent);
	void DeleteGameObject(const char* name);
	void DeleteGameObject(GameObject* toDelete);

	void SetToDelete(GameObject* toDelete);
	void SetComponentToDelete(Component* toDelete);

	GameObject* GetGameObject(uint index) const;
	uint GetGameObjectsLength() const;

private:

	std::vector<GameObject*> gameObjects;
	std::vector<GameObject*> needToBeDeleted;

	std::vector<Component*> componentsToDelete;
};

#endif