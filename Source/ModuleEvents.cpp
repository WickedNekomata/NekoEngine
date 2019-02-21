#include "ModuleEvents.h"
#include "EventSystem.h"

#include "ComponentTypes.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ComponentCollider.h"
#include "ComponentBoxCollider.h"
#include "ComponentCapsuleCollider.h"
#include "ComponentSphereCollider.h"
#include "ComponentPlaneCollider.h"
#include "ComponentRigidActor.h"
#include "ComponentRigidStatic.h"
#include "ComponentRigidDynamic.h"
#include "ComponentNavAgent.h"
#include "ComponentEmitter.h"
#include "ComponentBone.h"
#include "ComponentScript.h"

#include "Application.h"
#include "ModuleGOs.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"
#include "ScriptingModule.h"

void ModuleEvents::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::ComponentDestroyed:
		{
			delete event.compEvent.component;
			break;
		}
		case System_Event_Type::GameObjectDestroyed:
		{
			delete event.goEvent.gameObject;
			break;
		}
		case System_Event_Type::ResourceDestroyed:
		{
			delete event.resEvent.resource;
			break;
		}
		case System_Event_Type::Play:
			assert(App->GOs->sceneStateBuffer == 0);
			App->GOs->SerializeFromNode(App->scene->root, App->GOs->sceneStateBuffer, App->GOs->sceneStateSize);
			break;
		case System_Event_Type::SaveScene:
		{
			char* file; size_t size;
			App->GOs->SerializeFromNode(App->scene->root, file, size);
			App->fs->SaveInGame(file, size, FileType::SceneFile, std::string(event.sceneEvent.nameScene));
			delete[] file;
			break;
		}
		case System_Event_Type::Stop:
			assert(App->GOs->sceneStateBuffer != 0);
			App->scene->selectedObject = 0;
			App->GOs->ClearScene();
			App->GOs->LoadScene(App->GOs->sceneStateBuffer, App->GOs->sceneStateSize);
			delete[] App->GOs->sceneStateBuffer;
			App->GOs->sceneStateBuffer = 0;
			System_Event newEvent;
			newEvent.type = System_Event_Type::RecreateQuadtree;
			App->PushSystemEvent(newEvent);
			break;
		case System_Event_Type::LoadScene:
		{
			char file[DEFAULT_BUF_SIZE];
			sprintf_s(file, "%s/%s%s", DIR_ASSETS_SCENES, event.sceneEvent.nameScene, EXTENSION_SCENE);
			char* buf;
			size_t size = App->fs->Load(file, &buf);
			if (size != 0)
			{
				App->GOs->ClearScene();
				App->GOs->LoadScene(buf, size);
				delete[] buf;

				System_Event newEvent;
				newEvent.type = System_Event_Type::RecreateQuadtree;
				App->PushSystemEvent(newEvent);
			}
			else
				DEPRECATED_LOG("CANT FIND SCENE IN ASSETS");
			break;
		}

		case System_Event_Type::ScriptingDomainReloaded:
		{
			App->scripting->CreateDomain();
			App->scripting->RecompileScripts();
			App->scripting->ReInstance();
		}
	}
}