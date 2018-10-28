#include "ModuleGOs.h"

#include "GameObject.h"
#include "Component.h"

#include "Application.h"
#include "ModuleFileSystem.h"

#include "parson/parson.h"

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
	for (uint i = 0; i < needToBeDeleted.size(); ++i)
	{
		delete needToBeDeleted[i];
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), needToBeDeleted[i]), gameObjects.end());
	}

	needToBeDeleted.clear();

	for (uint i = 0; i < componentsToDelete.size(); ++i)
		componentsToDelete[i]->GetParent()->InternallyDeleteComponent(componentsToDelete[i]);

	componentsToDelete.clear();

	if (serializeScene)
		SerializeScene(); serializeScene = false;

	return UPDATE_CONTINUE;
}

bool ModuleGOs::CleanUp()
{
	for (uint i = 0; i < gameObjects.size(); ++i)
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
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i]->GetName() == name)
			gameObjects[i]->DeleteMe();
	}
}

void ModuleGOs::DeleteGameObject(GameObject* toDelete)
{
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i] == toDelete)
			gameObjects[i]->DeleteMe();
	}
}

void ModuleGOs::ClearScene()
{
	for (uint i = 0; i < gameObjects.size(); ++i)
			gameObjects[i]->DeleteMe();
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

void ModuleGOs::MarkSceneToSerialize()
{
	serializeScene = true;
}

void ModuleGOs::SerializeScene()
{
	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	for (int i = 0; i < gameObjects.size(); ++i)
	{
		JSON_Value* newValue = json_value_init_object();
		JSON_Object* objToSerialize = json_value_get_object(newValue);

		std::string s = std::to_string(i);
		json_object_set_value(rootObject, s.c_str(), newValue);
		gameObjects[i]->OnSave(objToSerialize);
	}

	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);
	App->filesystem->Save("scene.json", buf, sizeBuf);
	delete[] buf;
	json_value_free(rootValue);

	JSON_Object* file;	
}
