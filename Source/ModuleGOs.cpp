#include "ModuleGOs.h"

#include "GameObject.h"
#include "Component.h"

#include <algorithm>

ModuleGOs::ModuleGOs(bool start_enabled)
{
}

ModuleGOs::~ModuleGOs()
{
}

bool ModuleGOs::Init(JSON_Object* jObject)
{
	return true;
}

bool ModuleGOs::Start()
{
	return true;
}

update_status ModuleGOs::PreUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleGOs::Update(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleGOs::PostUpdate(float dt)
{

	for (int i = 0; i < needToBeDeleted.size(); ++i)
	{
		delete needToBeDeleted[i];
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), needToBeDeleted[i]), gameObjects.end());
	}

	needToBeDeleted.clear();

	for (int i = 0; i < componentsToDelete.size(); ++i)
	{
		componentsToDelete[i]->GetParent()->InternallyDeleteComponent(componentsToDelete[i]);
	}

	componentsToDelete.clear();

	return UPDATE_CONTINUE;
}

bool ModuleGOs::CleanUp()
{
	for (int i = 0; i < gameObjects.size(); ++i)
		RELEASE(gameObjects[i]);
	
	gameObjects.clear();

	return true;
}

GameObject* ModuleGOs::CreateGameObject(char* name, GameObject* parent)
{
	GameObject* newGameObject = new GameObject(name, parent);

	if (parent != nullptr)
		parent->AddChild(newGameObject);

	gameObjects.push_back(newGameObject);

	return newGameObject;
}

void ModuleGOs::DeleteGameObject(const char* name)
{
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i]->GetName() == name)
			gameObjects[i]->DeleteMe();
	}
}

void ModuleGOs::DeleteGameObject(GameObject* toDelete)
{
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i] == toDelete)
			gameObjects[i]->DeleteMe();
	}
}

void ModuleGOs::SetToDelete(GameObject* toDelete)
{
	needToBeDeleted.push_back(toDelete);
}

void ModuleGOs::SetComponentToDelete(Component* toDelete)
{
	componentsToDelete.push_back(toDelete);
}

GameObject* ModuleGOs::GetGameObject(uint index) const
{
	return gameObjects[index];
}

uint ModuleGOs::GetGameObjectsLength() const
{
	return gameObjects.size();
}
