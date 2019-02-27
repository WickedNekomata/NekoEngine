#include "ModuleGOs.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "GameObject.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"

#include "ModuleResourceManager.h"
#include "ResourceShaderProgram.h"

#include "ModuleRenderer3D.h"

#include <assert.h>

ModuleGOs::ModuleGOs(bool start_enabled) : Module(start_enabled)
{
	strcpy_s(nameScene, DEFAULT_BUF_SIZE, "Main Scene");
}

ModuleGOs::~ModuleGOs() {}

bool ModuleGOs::CleanUp()
{
	ClearScene();

	return true;
}

void ModuleGOs::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::RecalculateBBoxes:

		for (auto it = gameobjects.begin(); it != gameobjects.end(); ++it)
		{
			if (event.goEvent.gameObject == *it)
				(*it)->OnSystemEvent(event);
		}

		break;

	case System_Event_Type::LayerNameReset:

		// Reset layer to default (all game objects)
		for (std::vector<GameObject*>::const_iterator it = gameobjects.begin(); it != gameobjects.end(); ++it)
		{
			if ((*it)->GetLayer() == event.layerEvent.layer)
				(*it)->SetLayer(0);
		}
		break;

	case System_Event_Type::GameObjectDestroyed:
		event.goEvent.gameObject->GetParent()->EraseChild(event.goEvent.gameObject);
		gameobjects.erase(std::remove(gameobjects.begin(), gameobjects.end(), event.goEvent.gameObject), gameobjects.end());
		staticGos.erase(std::remove(staticGos.begin(), staticGos.end(), event.goEvent.gameObject), staticGos.end());
		dynamicGos.erase(std::remove(dynamicGos.begin(), dynamicGos.end(), event.goEvent.gameObject), dynamicGos.end());
		break;

	case System_Event_Type::ComponentDestroyed:
	{
		GameObject* go = event.compEvent.component->GetParent();
		go->EraseComponent(event.compEvent.component);

		switch (event.compEvent.component->GetType())
		{
		case ComponentTypes::TransformComponent:
			go->transform = 0;
			break;
		case ComponentTypes::MeshComponent:
			go->cmp_mesh = 0;
			break;
		case ComponentTypes::MaterialComponent:
			go->cmp_material = 0;
			break;
		case ComponentTypes::CameraComponent:
			go->cmp_camera = 0;
			break;
		case ComponentTypes::NavAgentComponent:
			go->cmp_navAgent = 0;
			break;
		case ComponentTypes::EmitterComponent:
			go->cmp_emitter = 0;
			break;
		case ComponentTypes::BoneComponent:
			go->cmp_bone = 0;
			break;
		case ComponentTypes::LightComponent:
			go->cmp_light = 0;
			break;
		case ComponentTypes::ProjectorComponent:
			go->cmp_projector = 0;
			break;
		case ComponentTypes::RigidStaticComponent:
		case ComponentTypes::RigidDynamicComponent:
			go->cmp_rigidActor = 0;
			break;
		case ComponentTypes::BoxColliderComponent:
		case ComponentTypes::SphereColliderComponent:
		case ComponentTypes::CapsuleColliderComponent:
		case ComponentTypes::PlaneColliderComponent:
			go->cmp_collider = 0;
			break;
		}
		break;
	}
	case System_Event_Type::ResourceDestroyed:
		InvalidateResource(event.resEvent.resource);
		break;
	case System_Event_Type::ScriptingDomainReloaded:
	case System_Event_Type::Stop:
	case System_Event_Type::LoadFinished:
	{
		for (auto it = gameobjects.begin(); it != gameobjects.end(); ++it)
		{			
			(*it)->OnSystemEvent(event);
		}
		break;
	}
	}
}

GameObject* ModuleGOs::CreateGameObject(const char* goName, GameObject* parent, bool disableTransform)
{
	GameObject* newGameObject = new GameObject(goName, parent, disableTransform);
	gameobjects.push_back(newGameObject);
	dynamicGos.push_back(newGameObject);
	return newGameObject;
}

GameObject* ModuleGOs::Instanciate(GameObject* copy, GameObject* newRoot)
{
	GameObject* newGameObject = new GameObject(*copy);
	gameobjects.push_back(newGameObject);

	if(!newRoot)
	{
		newGameObject->SetParent(copy->GetParent());
		copy->GetParent()->AddChild(newGameObject);
	}
	else
	{
		newGameObject->SetParent(newRoot);
		newRoot->AddChild(newGameObject);
	}

	std::vector<GameObject*> childs; newGameObject->GetChildrenAndThisVectorFromLeaf(childs);
	for (int i = 0; i < childs.size(); ++i)
	{
		App->GOs->RecalculateVector(childs[i], false);
	}

	System_Event newEvent;
	newEvent.type = System_Event_Type::RecreateQuadtree;
	App->PushSystemEvent(newEvent);

	return newGameObject;
}

void ModuleGOs::DeleteGameObject(GameObject* toDelete)
{
	toDelete->Destroy();
}

void ModuleGOs::Kill(GameObject* go)
{
	std::vector<GameObject*> toDestroy;
	go->GetChildrenAndThisVectorFromLeaf(toDestroy);
	for (int i = 0; i < toDestroy.size(); ++i)
	{
		for (int j = 0; j < toDestroy[i]->components.size(); ++j)
			RELEASE(toDestroy[i]->components[j]);
		RELEASE(toDestroy[i]);
	}
}

void ModuleGOs::GetGameobjects(std::vector<GameObject*>& gos) const
{
	gos = gameobjects;
}

void ModuleGOs::GetStaticGameobjects(std::vector<GameObject*>& gos) const
{
	gos = staticGos;
}

void ModuleGOs::GetDynamicGameobjects(std::vector<GameObject*>& gos) const
{
	gos = dynamicGos;
}

GameObject* ModuleGOs::GetGameObjectByUID(uint UID) const
{
	for (int i = 0; i < gameobjects.size(); ++i)
	{
		if (gameobjects[i]->GetUUID() == UID)
		{
			return gameobjects[i];
		}
	}
	return nullptr;
}

void ModuleGOs::ClearScene()
{
	for (int i = 0; i < gameobjects.size(); ++i)
		RELEASE(gameobjects[i]);

	gameobjects.clear();
	staticGos.clear();
	dynamicGos.clear();

	App->scene->FreeRoot();
}

void ModuleGOs::RecalculateVector(GameObject* go, bool sendEvent)
{
	dynamicGos.erase(std::remove(dynamicGos.begin(), dynamicGos.end(), go), dynamicGos.end());
	staticGos.erase(std::remove(staticGos.begin(), staticGos.end(), go), staticGos.end());

	if (go->IsStatic())
		staticGos.push_back(go);
	else
		dynamicGos.push_back(go);

	if (sendEvent)
	{
		System_Event newEvent;
		newEvent.type = System_Event_Type::RecreateQuadtree;
		App->PushSystemEvent(newEvent);
	}	
}

bool ModuleGOs::SerializeFromNode(GameObject* node, char*& outStateBuffer, size_t& sizeBuffer, bool navmesh)
{
	std::vector<GameObject*> go;
	node->GetChildrenVector(go);
	sizeBuffer = sizeof(uint);
	for (int i = 0; i < go.size(); ++i)
		sizeBuffer += go[i]->GetSerializationBytes();

	// Get size navmesh tiles data
	if (navmesh)
	sizeBuffer += App->navigation->GetNavMeshSerialitzationBytes();

	outStateBuffer = new char[sizeBuffer];
	char* cursor = outStateBuffer;

	uint totalGO = go.size();
	memcpy(cursor, &totalGO, sizeof(uint));
	cursor += sizeof(uint);

	for (int i = 0; i < go.size(); ++i)
		go[i]->OnSave(cursor);

	// Discuss if this should be a resource
	if (navmesh)
		App->navigation->SaveNavmesh(cursor);

	return true;
}

GameObject* ModuleGOs::DeSerializeToNode(char*& buffer, size_t sizeBuffer, bool navmesh)
{
	char* cursor = buffer;
	size_t bytes = sizeof(uint);
	uint totalGO;
	memcpy(&totalGO, cursor, bytes);
	cursor += bytes;

	std::vector<GameObject*> gos;
	gos.reserve(totalGO);

	for (int i = 0; i < totalGO; ++i)
	{
		GameObject* go = new GameObject("", nullptr, true);
		go->OnLoad(cursor, false);

		for (int i = gos.size() - 1; i >= 0; --i)
		{
			if (gos[i]->GetUUID() == go->GetParentUUID())
			{
				go->SetParent(gos[i]);
				gos[i]->AddChild(go);
			}
		}

		/*if (go->GetParent() == 0)
		{
			assert(App->scene->root == 0);
			App->scene->root = go;
		}
		else
		{
			gameobjects.push_back(go);
			go->IsStatic() ? staticGos.push_back(go) : dynamicGos.push_back(go);
		}*/

		gos.push_back(go);
	}

	// Discuss if this should be a resource
	if (navmesh)
		App->navigation->LoadNavmesh(cursor);

	return gos[0]; //the root node
}

bool ModuleGOs::LoadScene(char*& buffer, size_t sizeBuffer, bool navmesh)
{
	char* cursor = buffer;
	size_t bytes = sizeof(uint);
	uint totalGO;
	memcpy(&totalGO, cursor, bytes);
	cursor += bytes;

	std::vector<GameObject*> gos;
	gameobjects.reserve(totalGO - 1);
	gos.reserve(totalGO);

	for (int i = 0; i < totalGO; ++i)
	{
		GameObject* go = new GameObject("", nullptr, true);
		go->OnLoad(cursor);

		for (int i = gos.size() - 1; i >= 0; --i)
		{
			if (gos[i]->GetUUID() == go->GetParentUUID())
			{
				go->SetParent(gos[i]);
				gos[i]->AddChild(go);
			}
		}
		if (go->GetParent() == 0)
		{
			assert(App->scene->root == 0);
			App->scene->root = go;
		}
		else
		{
			gameobjects.push_back(go);
			go->IsStatic() ? staticGos.push_back(go) : dynamicGos.push_back(go);
		}
		gos.push_back(go);
	}

	// Discuss if this should be a resource
	if (navmesh)
		App->navigation->LoadNavmesh(cursor);

	System_Event event;
	event.type = System_Event_Type::LoadFinished;
	App->PushSystemEvent(event);

	return true;
}

bool ModuleGOs::InvalidateResource(Resource* resource)
{
	assert(resource != nullptr);

	for (uint i = 0; i < gameobjects.size(); ++i)
	{
		switch (resource->GetType())
		{
		case ResourceTypes::MeshResource:
			if (gameobjects[i]->cmp_mesh != nullptr && gameobjects[i]->cmp_mesh->res == resource->GetUuid())
				gameobjects[i]->cmp_mesh->SetResource(0);
			break;
		case ResourceTypes::TextureResource:
			if (gameobjects[i]->cmp_material != nullptr && gameobjects[i]->cmp_material->res == resource->GetUuid())
				gameobjects[i]->cmp_material->SetResource(0);
			break;
		}
	}

	return true;
}