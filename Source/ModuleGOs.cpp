#include "ModuleGOs.h"

#include "GameObject.h"

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
	return UPDATE_CONTINUE;
}

bool ModuleGOs::CleanUp()
{
	return true;
}

GameObject* ModuleGOs::CreateGameObject(char* name, GameObject* parent)
{
	GameObject* newGameObject = new GameObject(name, parent);

	gameObjects.push_back(newGameObject);

	return newGameObject;
}

void ModuleGOs::DeleteGameObject(const char* name)
{
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i]->GetName() == name)
		{
			delete gameObjects[i];
			gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), name), gameObjects.end());
		}
	}
}

void ModuleGOs::DeleteGameObject(GameObject* toDelete)
{
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i] == toDelete)
		{
			delete gameObjects[i];
			gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), name), gameObjects.end());
		}
	}
}
