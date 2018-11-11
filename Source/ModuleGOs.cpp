#include "ModuleGOs.h"

#include "Globals.h"
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
	strcpy_s((char*)nameScene, DEFAULT_BUF_SIZE, "Main Scene");
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
	{
		std::string outputFileName = nameScene;
		SerializeFromNode(App->scene->root, outputFileName);
	}

	serializeScene = false;

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

bool ModuleGOs::OnGameMode()
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

	return true;
}

bool ModuleGOs::OnEditorMode()
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

	return true;
}

GameObject* ModuleGOs::CreateGameObject(const char* name, GameObject* parent, bool disableTransform)
{
	GameObject* newGameObject = new GameObject(name, parent, disableTransform);

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
	App->scene->selectedObject = CurrentSelection::SelectedType::null;
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

void ModuleGOs::GetGameObjects(std::vector<GameObject*>& gameObjects) const
{
	for (uint i = 0; i < this->gameObjects.size(); ++i)
		gameObjects.push_back(this->gameObjects[i]);
}

void ModuleGOs::GetStaticGameObjects(std::vector<GameObject*>& gameObjects) const
{
	for (uint i = 0; i < this->gameObjects.size(); ++i)
	{
		if (this->gameObjects[i]->IsStatic())
			gameObjects.push_back(this->gameObjects[i]);
	}
}

void ModuleGOs::GetDynamicGameObjects(std::vector<GameObject*>& gameObjects) const
{
	for (uint i = 0; i < this->gameObjects.size(); ++i)
	{
		if (!this->gameObjects[i]->IsStatic())
			gameObjects.push_back(this->gameObjects[i]);
	}
}

void ModuleGOs::ReorderGameObjects(GameObject* source, GameObject* target)
{
	int index = 0;
	for (index = 0; index < gameObjects.size(); ++index)
	{
		if (gameObjects[index] == target)
			break;
	}

	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), source), gameObjects.end());
	gameObjects.insert(gameObjects.begin() + index, source);
}

void ModuleGOs::MarkSceneToSerialize()
{
	serializeScene = true;
}

/*
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
*/

bool ModuleGOs::SerializeFromNode(const GameObject* node, std::string& outputFileName)
{
	JSON_Value* rootValue = json_value_init_array();
	JSON_Array* goArray = json_value_get_array(rootValue);

	node->RecursiveSerialitzation(goArray);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);

	uint size = App->fs->SaveInLibrary(buf, sizeBuf, FileType::SceneFile, outputFileName);
	if (size > 0)
	{
		CONSOLE_LOG("Scene Serialization: Successfully saved Scene '%s'", outputFileName.data());
	}
	else
	{
		CONSOLE_LOG("Scene Serialization: Could not save Scene '%s'", outputFileName.data());
		return false;
	}

	RELEASE(buf);
	json_value_free(rootValue);

	return true;
}

bool ModuleGOs::LoadScene(const char* fileName)
{
	char* buffer;
	uint size = App->fs->LoadFromLibrary(fileName, &buffer, FileType::SceneFile);
	if (size > 0)
	{
		CONSOLE_LOG("Scene Serialization: Successfully loaded Scene '%s'", fileName);
	}
	else
	{
		CONSOLE_LOG("Scene Serialization: Could not load Scene '%s'", fileName);
		return false;
	}

	JSON_Value* root_value;
	JSON_Array* gameObjectsArray;
	JSON_Object* gObject;

	/* parsing json and validating output */
	root_value = json_parse_string(buffer);
	if (json_value_get_type(root_value) != JSONArray)
		return false;

	gameObjectsArray = json_value_get_array(root_value);
	std::vector<GameObject*>auxList;
	auxList.reserve(json_array_get_count(gameObjectsArray));
	for (int i = 0; i < json_array_get_count(gameObjectsArray); i++) 
	{
		gObject = json_array_get_object(gameObjectsArray, i);
		GameObject* go = CreateGameObject((char*)json_object_get_string(gObject, "name"), App->scene->root, true);
		go->OnLoad(gObject);
		auxList.push_back(go);
	}

	for (int i = 0; i < json_array_get_count(gameObjectsArray); i++)
	{
		gObject = json_array_get_object(gameObjectsArray, i);
		GameObject* parent = GetGameObjectByUUID(json_object_get_number(gObject, "Parent UUID"));
		if (parent != nullptr) {
			auxList[i]->GetParent()->EraseChild(auxList[i]);
			parent->AddChild(auxList[i]);
			auxList[i]->SetParent(parent);
		}
	}

	RELEASE_ARRAY(buffer);
	json_value_free(root_value);

	return true;
}

bool ModuleGOs::GetMeshResourcesFromScene(const char* fileName, std::list<uint>& UUIDs) const
{
	char* buffer;
	uint size = App->fs->LoadFromLibrary(fileName, &buffer, FileType::SceneFile);
	if (size > 0)
	{
		CONSOLE_LOG("Scene Serialization: Successfully loaded Scene '%s'", fileName);
	}
	else
	{
		CONSOLE_LOG("Scene Serialization: Could not load Scene '%s'", fileName);
		return false;
	}

	JSON_Value* root_value;
	JSON_Array* gameObjectsArray;
	JSON_Object* gObject;

	/* parsing json and validating output */
	root_value = json_parse_string(buffer);
	if (json_value_get_type(root_value) != JSONArray)
		return false;

	gameObjectsArray = json_value_get_array(root_value);

	for (int i = 0; i < json_array_get_count(gameObjectsArray); i++) 
	{
		gObject = json_array_get_object(gameObjectsArray, i);

		JSON_Array* jsonComponents = json_object_get_array(gObject, "jsonComponents");
		JSON_Object* cObject;

		for (int i = 0; i < json_array_get_count(jsonComponents); i++) 
		{
			cObject = json_array_get_object(jsonComponents, i);
			if ((ComponentType)(int)json_object_get_number(cObject, "Type") == ComponentType::Mesh_Component)
				UUIDs.push_back(json_object_get_number(cObject, "ResourceMesh"));
		}
	}

	RELEASE_ARRAY(buffer);
	json_value_free(root_value);

	return true;
}