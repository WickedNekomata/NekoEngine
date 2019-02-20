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
	GameObject* CreateCanvas(const char* name, GameObject* parent);
	inline void AddGameObject(GameObject* gameObject) { gameObjects.push_back(gameObject); }
	void DeleteGameObject(const char* name);
	void DeleteGameObject(GameObject* toDelete);
	void DeleteCanvasPointer();

	void DeleteTemporaryGameObjects();
	void DeleteScene();
	void ClearScene();

	void SetToDelete(GameObject* toDelete);
	void SetComponentToDelete(Component* toDelete);

	GameObject* GetGameObject(uint index) const;
	GameObject* GetGameObjectByUUID(uint UUID) const;
	void GetGameObjects(std::vector<GameObject*>& gameObjects) const;
	void GetStaticGameObjects(std::vector<GameObject*>& gameObjects) const;
	void GetMeshComponentsFromStaticGameObjects(std::vector<class ComponentMesh*>& components) const;
	void GetDynamicGameObjects(std::vector<GameObject*>& gameObjects) const;

	//TODO: Ask if this is right
	inline GameObject* getRoot() const { return gameObjects[0]; }

	void ReorderGameObjects(GameObject* source, GameObject* target);

	void MarkSceneToSerialize();
	bool SerializeFromNode(const GameObject* node, std::string& outputFile);
	bool LoadScene(const char* file);
	bool GetMeshResourcesFromScene(const char* file, std::vector<std::string>& meshes, std::vector<uint>& UUIDs) const;

	bool InvalidateResource(const Resource* resource);


	//UI
	bool ExistCanvas() const;
	bool IsCanvas(GameObject* go_canvas) const;
	GameObject* GetCanvas()const;


public:

	const char* nameScene = nullptr;

private:

	bool serializeScene = false;

	std::vector<GameObject*> gameObjects;
	std::vector<GameObject*> gameObjectsToDelete;
	std::vector<Component*> componentsToDelete;

	// OnGameMode/OnEditorMode
	std::vector<GameObject*> tmpGameObjects;

	//Canvas
	GameObject* canvas = nullptr;
};

#endif