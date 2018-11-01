#include "ModuleGOs.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentCamera.h"

#include "Application.h"
#include "ModuleFileSystem.h"

#include "parson/parson.h"
#include <list>
#include <algorithm>

ModuleGOs::ModuleGOs(bool start_enabled)
{
	nameScene = new char[DEFAULT_BUF_SIZE];
	strcpy_s(nameScene, DEFAULT_BUF_SIZE, "Main Scene");
}

ModuleGOs::~ModuleGOs()
{
	delete[] nameScene;
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
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), needToBeDeleted[i]), gameObjects.end());
		delete needToBeDeleted[i];
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

void ModuleGOs::RecalculateQuadtree()
{
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i]->IsStatic())
			App->scene->quadtree.Insert(gameObjects[i]);
	}
}

void ModuleGOs::MarkSceneToSerialize()
{
	serializeScene = true;
}

void ModuleGOs::SerializeScene()
{
	JSON_Value* rootValue = json_value_init_array();
	JSON_Array* goArray = json_value_get_array(rootValue);

	for (int i = 0; i < gameObjects.size(); ++i)
	{
		JSON_Value* newValue = json_value_init_object();
		JSON_Object* objToSerialize = json_value_get_object(newValue);

		gameObjects[i]->OnSave(objToSerialize);
		json_array_append_value(goArray, newValue);
	}
	
	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);
	App->filesystem->SaveInLibrary(buf, sizeBuf, FileType::SceneFile, std::string(nameScene));
	delete[] buf;
	json_value_free(rootValue);
}