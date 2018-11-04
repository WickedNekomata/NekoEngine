#include "ModuleGOs.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentCamera.h"

#include "Application.h"
#include "ModuleFileSystem.h"

#include "parson/parson.h"
#include <list>
#include <algorithm>

ModuleGOs::ModuleGOs(bool start_enabled) : Module(start_enabled)
{
	name = "GameObjects";
	game = true;

	nameScene = new char[DEFAULT_BUF_SIZE];
	strcpy_s(nameScene, DEFAULT_BUF_SIZE, "Main Scene");
}

ModuleGOs::~ModuleGOs()
{
	delete[] nameScene;
}

update_status ModuleGOs::PostUpdate()
{
	for (int i = gameObjectsToDelete.size() - 1; i >= 0; --i)
	{
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObjectsToDelete[i]), gameObjects.end());
		RELEASE(gameObjectsToDelete[i]);
	}

	gameObjectsToDelete.clear();

	for (uint i = 0; i < componentsToDelete.size(); ++i)
		componentsToDelete[i]->GetParent()->InternallyDeleteComponent(componentsToDelete[i]);

	componentsToDelete.clear();

	if (serializeScene)
		SerializeScene(); serializeScene = false;

	return UPDATE_CONTINUE;
}

bool ModuleGOs::CleanUp()
{
	ClearScene();

	for (int i = gameObjectsToDelete.size() - 1; i >= 0; --i)
	{
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObjectsToDelete[i]), gameObjects.end());
		RELEASE(gameObjectsToDelete[i]);
	}
	
	componentsToDelete.clear();
	gameObjectsToDelete.clear();
	gameObjects.clear();


	for (int i = tmpGameObjects.size() - 1; i >= 0; --i)
	{
		tmpGameObjects.erase(std::remove(tmpGameObjects.begin(), tmpGameObjects.end(), tmpGameObjects[i]), tmpGameObjects.end());
		RELEASE(tmpGameObjects[i]);
	}
	tmpGameObjects.clear();

	return true;
}

void ModuleGOs::OnGameMode()
{
	/*
	// Save scene in memory

	// 1. Copy game objects to a temporary gameObjects vector
	tmpGameObjects.clear();

	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* tmpGameObject = new GameObject(*gameObjects[i]);
		tmpGameObjects.push_back(tmpGameObject);
	}
	*/
}

void ModuleGOs::OnEditorMode()
{
	/*
	// Load scene from memory

	// 1. Clear game objects
	ClearScene();

	for (int i = gameObjectsToDelete.size() - 1; i >= 0; --i)
	{
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObjectsToDelete[i]), gameObjects.end());
		RELEASE(gameObjectsToDelete[i]);
	}

	componentsToDelete.clear();
	gameObjectsToDelete.clear();
	gameObjects.clear();

	// 2. Copy temporary game objects to the real gameObjects vector
	for (uint i = 0; i < tmpGameObjects.size(); ++i)
		gameObjects.push_back(tmpGameObjects[i]);

	// 3. Match correct parent and children of the game objects
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* parent = GetGameObjectByUUID(gameObjects[i]->GetParentUUID());
		parent->AddChild(gameObjects[i]);
		gameObjects[i]->SetParent(parent);
	}
	*/
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
	App->scene->root->DeleteChildren();
	App->scene->SetCurrentGameObject(nullptr);
}

void ModuleGOs::SetToDelete(GameObject* toDelete)
{
	gameObjectsToDelete.push_back(toDelete);
}

void ModuleGOs::SetComponentToDelete(Component* toDelete)
{
	componentsToDelete.push_back(toDelete);
}

GameObject* ModuleGOs::GetGameObject(uint index) const
{
	return gameObjects[index];
}

GameObject* ModuleGOs::GetGameObjectByUUID(uint UUID) const
{
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i]->GetUUID() == UUID && std::find(gameObjectsToDelete.begin(), gameObjectsToDelete.end(), gameObjects[i]) == gameObjectsToDelete.end())
			return gameObjects[i];
	}

	if (UUID == App->scene->root->GetUUID())
		return App->scene->root;
	
	return nullptr;
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

bool ModuleGOs::LoadScene(char* fileName)
{
	char* buf;

	std::string path("Library/Scenes/");
	path += fileName;
	path += ".json";
	
	uint size = App->filesystem->Load(path.c_str(), &buf);

	if (size <= 0)
		return false;

	JSON_Value* root_value;
	JSON_Array* gameObjectsArray;
	JSON_Object* gObject;

	/* parsing json and validating output */
	root_value = json_parse_string(buf);
	if (json_value_get_type(root_value) != JSONArray) {
		return false;
	}

	gameObjectsArray = json_value_get_array(root_value);
	std::vector<GameObject*>auxList;
	auxList.reserve(json_array_get_count(gameObjectsArray));
	for (int i = 0; i < json_array_get_count(gameObjectsArray); i++) {
		gObject = json_array_get_object(gameObjectsArray, i);
		GameObject* go = CreateGameObject((char*)json_object_get_string(gObject, "name"), App->scene->root);
		go->OnLoad(gObject);
		auxList.push_back(go);
	}

	for (int i = 0; i < json_array_get_count(gameObjectsArray); i++)
	{
		gObject = json_array_get_object(gameObjectsArray, i);
		GameObject* parent = GetGameObjectByUUID(json_object_get_number(gObject, "Parent UUID"));
		auxList[i]->GetParent()->EraseChild(auxList[i]);
		parent->AddChild(auxList[i]);
		auxList[i]->SetParent(parent);
	}

	json_value_free(root_value);

	return true;
}