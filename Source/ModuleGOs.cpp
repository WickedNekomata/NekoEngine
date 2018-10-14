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
	/* TODO WARNING: think about how to approach the children deletion. Create a method in go to erase a go and call it 
					 on go's parent deletechildren method? Or only set a bool to delete and delete them here (TAKE CARE if
					 parent is constantly beeing swaped cause of gameobjects' vector iteration order).
					
					 Not easy to approach.

	  				 ps: the same problem need to be solved at components :)
	*/

	for (int i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i]->needToBeDeleted)
		{
			delete gameObjects[i];
			gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObjects[i]), gameObjects.end());
		}
	}

	return UPDATE_CONTINUE;
}

bool ModuleGOs::CleanUp()
{
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
		//if (gameObjects[i]->GetName() == name)
		//{
			//delete gameObjects[i];
			//gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), name), gameObjects.end());
		//}
	}
}

void ModuleGOs::DeleteGameObject(GameObject* toDelete)
{
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i] == toDelete)
			gameObjects[i]->needToBeDeleted = true;
	}
}

GameObject* ModuleGOs::GetGameObject(uint index) const
{
	return gameObjects[index];
}

uint ModuleGOs::GetGameObjectsLength() const
{
	return gameObjects.size();
}
