#ifndef __MODULE_GOS_H__
#define __MODULE_GOS_H__

#include "Module.h"

#include <vector>

class GameObject;

class ModuleGOs : public Module
{
public:
	ModuleGOs(bool start_enabled = true);
	~ModuleGOs();

	bool CleanUp();
	void OnSystemEvent(System_Event event);

	GameObject* CreateGameObject(const char* name, GameObject* parent, bool disableTransform = false);
	GameObject* Instanciate(GameObject* target);
	void DeleteGameObject(GameObject* toDelete);
	void Kill(GameObject* go);

	void GetGameobjects(std::vector<GameObject*>& gos) const;
	void GetStaticGameobjects(std::vector<GameObject*>& gos) const;
	void GetDynamicGameobjects(std::vector<GameObject*>& gos) const;

	void ClearScene();

	void RecalculateVector(GameObject* go); //if static or dynamic

	bool SerializeFromNode(GameObject* node, char*& outStateBuffer, size_t& sizeBuffer);
	bool LoadScene(char*& buffer, size_t sizeBuffer);

	bool InvalidateResource(Resource* resource);

private:

	std::vector<GameObject*> gameobjects;
	std::vector<GameObject*> staticGos;
	std::vector<GameObject*> dynamicGos;

public:
	// This is for saving and loading scenes from ModuleEvents ;( (friendship?)
	char* sceneStateBuffer = 0;
	size_t sceneStateSize;

	char nameScene[DEFAULT_BUF_SIZE];
};
#endif