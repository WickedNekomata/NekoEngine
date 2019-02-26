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
	GameObject* Instanciate(GameObject* target, GameObject* newRoot = nullptr);
	void DeleteGameObject(GameObject* toDelete);
	void Kill(GameObject* go);

	void GetGameobjects(std::vector<GameObject*>& gos) const;
	void GetStaticGameobjects(std::vector<GameObject*>& gos) const;
	void GetDynamicGameobjects(std::vector<GameObject*>& gos) const;
	GameObject* GetGameObjectByUID(uint UID) const;

	void ClearScene();

	void RecalculateVector(GameObject* go, bool sendEvent = true); //if static or dynamic

	bool static SerializeFromNode(GameObject* node, char*& outStateBuffer, size_t& sizeBuffer, bool navmesh = false);
	static GameObject* DeSerializeToNode(char*& buffer, size_t sizeBuffer, bool navmesh = false);

	bool LoadScene(char*& buffer, size_t sizeBuffer, bool navmesh = false);

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