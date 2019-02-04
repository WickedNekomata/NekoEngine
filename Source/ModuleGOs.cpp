#include "ModuleGOs.h"

#include "Globals.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "ModuleResourceManager.h"

#include "Resource.h"
#include "ResourceShaderProgram.h"

#include "parson\parson.h"
#include "Brofiler\Brofiler.h"

#include <list>
#include <algorithm>

ModuleGOs::ModuleGOs(bool start_enabled) : Module(start_enabled)
{
	name = "GameObjects";

	nameScene = new char[DEFAULT_BUF_SIZE];
	strcpy_s((char*)nameScene, DEFAULT_BUF_SIZE, "Main Scene");
}

ModuleGOs::~ModuleGOs()
{
	RELEASE_ARRAY(nameScene);
}

update_status ModuleGOs::PostUpdate()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);
#endif

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
		if (SerializeFromNode(App->scene->root, outputFileName))
		{
			System_Event newEvent;
			newEvent.type = System_Event_Type::RefreshFiles;
			App->PushSystemEvent(newEvent);
		}
	}

	serializeScene = false;

	return UPDATE_CONTINUE;
}

bool ModuleGOs::CleanUp()
{
	// Remove hierarchy (game objects) and load hierarchy from temporary game objects
	DeleteScene();

	for (uint i = 0; i < tmpGameObjects.size(); ++i)
		gameObjects.push_back(tmpGameObjects[i]);

	tmpGameObjects.clear();

	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* parent = GetGameObjectByUUID(gameObjects[i]->GetParentUUID());
		parent->AddChild(gameObjects[i]);
		gameObjects[i]->SetParent(parent);
	}

	// Remove hierarchy (temporary game objects)
	ClearScene();

	for (uint i = 0; i < gameObjectsToDelete.size(); ++i)
	{
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObjectsToDelete[i]), gameObjects.end());
		RELEASE(gameObjectsToDelete[i]);
	}

	componentsToDelete.clear();
	gameObjectsToDelete.clear();
	gameObjects.clear();

	// Free the root
	App->scene->FreeRoot();

	return true;
}

void ModuleGOs::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::RecalculateBBoxes:

		for (std::vector<GameObject*>::const_iterator it = gameObjects.begin(); it != gameObjects.end(); ++it)
		{
			if (event.goEvent.gameObject == *it)
				(*it)->OnSystemEvent(event);
		}

		break;

	case System_Event_Type::ShaderProgramChanged:

		for (std::vector<GameObject*>::const_iterator it = gameObjects.begin(); it != gameObjects.end(); ++it)
		{
			if ((*it)->materialRenderer != nullptr)
			{
				ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)App->res->GetResource((*it)->materialRenderer->shaderProgramUUID);
				
				if (shaderProgram != nullptr && shaderProgram->shaderProgram == event.shaderEvent.shader)
					(*it)->OnSystemEvent(event);
			}
		}

		break;

	case System_Event_Type::LayerReset:

		for (std::vector<GameObject*>::const_iterator it = gameObjects.begin(); it != gameObjects.end(); ++it)
		{
			if ((*it)->layer == event.layerEvent.layer)
				(*it)->layer = 0;
		}
	}
}

bool ModuleGOs::OnGameMode()
{
	// Save scene in memory

	// 1. Copy game objects to a temporary gameObjects vector
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* tmpGameObject = new GameObject(*gameObjects[i]);
		tmpGameObjects.push_back(tmpGameObject);
	}
	DEPRECATED_LOG("MODULE GOS: tmpGameObjects vector size OnGameMode: %i", tmpGameObjects.size());

	return true;
}

bool ModuleGOs::OnEditorMode()
{
	// Load scene from memory

	// 1. Clear game objects
	DeleteScene();

	// 2. Copy temporary game objects to the real gameObjects vector and activate them
	DEPRECATED_LOG("MODULE GOS: tmpGameObjects vector size OnEditorMode: %i", tmpGameObjects.size());
	for (uint i = 0; i < tmpGameObjects.size(); ++i)
	{
		gameObjects.push_back(tmpGameObjects[i]);
		tmpGameObjects[i]->Activate();
	}

	tmpGameObjects.clear();

	// 3. Match correct parent and children of the game objects
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* parent = GetGameObjectByUUID(gameObjects[i]->GetParentUUID());
		parent->AddChild(gameObjects[i]);
		gameObjects[i]->SetParent(parent);
	}

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
		{
			gameObjects[i]->DeleteMe();
			gameObjects[i]->GetParent()->EraseChild(gameObjects[i]);
		}
	}
}

void ModuleGOs::DeleteGameObject(GameObject* toDelete)
{
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i] == toDelete)
		{
			gameObjects[i]->DeleteMe();
			gameObjects[i]->GetParent()->EraseChild(gameObjects[i]);
		}
	}
}

void ModuleGOs::DeleteTemporaryGameObjects()
{
	for (uint i = 0; i < tmpGameObjects.size(); ++i)
		RELEASE(tmpGameObjects[i]);

	tmpGameObjects.clear();
}

void ModuleGOs::DeleteScene()
{
	DEPRECATED_LOG("MODULE GOS: Game Objects in hierarchy: %i", gameObjects.size());
	ClearScene();
	DEPRECATED_LOG("MODULE GOS: Game Objects in gameObjectsToDelete vector after ClearScene: %i", gameObjectsToDelete.size());

	for (uint i = 0; i < gameObjectsToDelete.size(); ++i)
	{
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObjectsToDelete[i]), gameObjects.end());
		RELEASE(gameObjectsToDelete[i]);
	}

	componentsToDelete.clear();
	gameObjectsToDelete.clear();
	gameObjects.clear();
}

void ModuleGOs::ClearScene()
{
	App->scene->root->DeleteChildren();
	SELECT(NULL);
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

bool ModuleGOs::SerializeFromNode(const GameObject* node, std::string& outputFile)
{
	JSON_Value* rootValue = json_value_init_array();
	JSON_Array* goArray = json_value_get_array(rootValue);

	node->RecursiveSerialitzation(goArray);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);

	uint size = App->fs->SaveInGame(buf, sizeBuf, FileType::SceneFile, outputFile);
	if (size > 0)
	{
		DEPRECATED_LOG("Scene Serialization: Successfully saved Scene '%s'", outputFile.data());
	}
	else
	{
		DEPRECATED_LOG("Scene Serialization: Could not save Scene '%s'", outputFile.data());
		return false;
	}

	RELEASE_ARRAY(buf);
	json_value_free(rootValue);

	return true;
}

bool ModuleGOs::LoadScene(const char* file)
{
	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		DEPRECATED_LOG("Scene Serialization: Successfully loaded Scene '%s'", file);
	}
	else
	{
		DEPRECATED_LOG("Scene Serialization: Could not load Scene '%s'", file);
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
	int sizeOfArray = json_array_get_count(gameObjectsArray);
	for (int i = 0; i < sizeOfArray; i++)
	{
		gObject = json_array_get_object(gameObjectsArray, i);
		GameObject* go = CreateGameObject((char*)json_object_get_string(gObject, "Name"), App->scene->root, true);
		go->OnLoad(gObject);
		auxList.push_back(go);
	}

	for (int i = 0; i < sizeOfArray; i++)
	{
		gObject = json_array_get_object(gameObjectsArray, i);
		GameObject* parent = GetGameObjectByUUID(json_object_get_number(gObject, "Parent UUID"));
		if (parent != nullptr) {
			auxList[i]->GetParent()->EraseChild(auxList[i]);
			parent->AddChild(auxList[i]);
			auxList[i]->SetParent(parent);
		}
	}

	for (int i = 0; i < auxList.size(); ++i)
		auxList[i]->ForceUUID(App->GenerateRandomNumber());

	RELEASE_ARRAY(buffer);
	json_value_free(root_value);

	return true;
}

bool ModuleGOs::GetMeshResourcesFromScene(const char* file, std::vector<std::string>& meshes, std::vector<uint>& UUIDs) const
{
	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		DEPRECATED_LOG("Scene Serialization: Successfully loaded Scene '%s'", file);
	}
	else
	{
		DEPRECATED_LOG("Scene Serialization: Could not load Scene '%s'", file);
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

			if ((ComponentTypes)(int)json_object_get_number(cObject, "Type") == ComponentTypes::MeshComponent)
			{
				meshes.push_back(json_object_get_string(gObject, "Name"));
				UUIDs.push_back(json_object_get_number(cObject, "ResourceMesh"));
			}
		}
	}

	RELEASE_ARRAY(buffer);
	json_value_free(root_value);

	return true;
}

bool ModuleGOs::InvalidateResource(const Resource* resource)
{
	if (resource == nullptr)
	{
		assert(resource != nullptr);
		return false;
	}

	ResourceType type = resource->GetType();

	for (uint i = 0; i < this->gameObjects.size(); ++i)
	{
		switch (type)
		{
		case ResourceType::MeshResource:
			if (gameObjects[i]->meshRenderer != nullptr && gameObjects[i]->meshRenderer->res == resource->GetUUID())
				gameObjects[i]->meshRenderer->SetResource(0);
			break;
		case ResourceType::TextureResource:
			if (gameObjects[i]->materialRenderer != nullptr)
			{
				for (uint j = 0; j < gameObjects[i]->materialRenderer->res.size(); ++j)
				{
					if (gameObjects[i]->materialRenderer->res[j].res == resource->GetUUID())
						gameObjects[i]->materialRenderer->SetResource(0, j);
				}
			}
			break;
		}
	}

	//assert(resource->CountReferences() <= 0);
	return true;
}