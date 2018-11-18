#ifndef __MODULE_GOS_H__
#define __MODULE_GOS_H__

#include "Module.h"

#include <vector>
#include <list>

class GameObject;
class Component;
class ComponentCamera;
class Resource;

class ModuleGOs : public Module
{
public:

	ModuleGOs(bool start_enabled = true);
	~ModuleGOs();

	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	bool OnGameMode();
	bool OnEditorMode();

	GameObject* CreateGameObject(const char* name, GameObject* parent, bool disableTransform = false);
	void DeleteGameObject(const char* name);
	void DeleteGameObject(GameObject* toDelete);

	void DeleteTemporaryGameObjects();
	void DeleteScene();
	void ClearScene();

	void SetToDelete(GameObject* toDelete);
	void SetComponentToDelete(Component* toDelete);

	GameObject* GetGameObject(uint index) const;
	GameObject* GetGameObjectByUUID(uint UUID) const;
	void GetGameObjects(std::vector<GameObject*>& gameObjects) const;
	void GetStaticGameObjects(std::vector<GameObject*>& gameObjects) const;
	void GetDynamicGameObjects(std::vector<GameObject*>& gameObjects) const;

	void ReorderGameObjects(GameObject* source, GameObject* target);

	void MarkSceneToSerialize();
	bool SerializeFromNode(const GameObject* node, std::string& outputFile);
	bool LoadScene(const char* file);
	bool GetMeshResourcesFromScene(const char* file, std::list<uint>& UUIDs) const;

	bool InvalidateResource(const Resource* resource);

public:

	const char* nameScene = nullptr;

private:

	bool serializeScene = false;

	std::vector<GameObject*> gameObjects;
	std::vector<GameObject*> gameObjectsToDelete;
	std::vector<Component*> componentsToDelete;

	// OnGameMode/OnEditorMode
	std::vector<GameObject*> tmpGameObjects;
};

#endif