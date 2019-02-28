#include "Application.h"
#include "ModuleCameraEditor.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ResourceMesh.h"
#include "PanelInspector.h"
#include "ModuleGui.h"
#include "Resource.h"

#include <assert.h>

class Resource;

struct CurrentSelection
{
	enum class SelectedType { null, gameObject, scene, resource, meshImportSettings };

private:
	void* cur = nullptr;
	SelectedType type = SelectedType::null;

public:
	void* Get() const
	{
		return cur;
	}

	SelectedType GetType() const
	{
		return type;
	}

	CurrentSelection& operator=(SelectedType newSelection)
	{
		assert((newSelection == SelectedType::null || newSelection == SelectedType::scene) && "Invalid operation");
		type = newSelection;
		cur = nullptr;
		return *this;
	}

	CurrentSelection& operator=(int null)
	{
		assert(null == NULL && "Invalid operation");
		type = SelectedType::null;
		cur = nullptr;
		return *this;
	}

	bool operator==(const SelectedType rhs)
	{
		return type == rhs;
	}

	bool operator==(int null)
	{
		assert(null == NULL && "Invalid comparison");
		return cur == nullptr;
	}

	bool operator!=(const SelectedType rhs)
	{
		return type != rhs;
	}

	bool operator!=(int null)
	{
		assert(null == NULL && "Invalid comparison");
		return cur != nullptr;
	}

	//-----------// GAMEOBJECTS //----------//

	CurrentSelection& operator=(GameObject* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::gameObject;

#ifndef GAMEMODE
		// New game object selected. Update the camera reference
		if(newSelection->GetLayer() != UILAYER)
			App->camera->SetReference(newSelection->transform->position);
#endif

		return *this;
	}

	bool operator==(const GameObject* rhs)
	{
		return cur == rhs;
	}

	GameObject* GetCurrGameObject()
	{
		if (type == SelectedType::gameObject)
			return (GameObject*)cur;
		else
			return nullptr;
	}
	//-----------// RESOURCES //----------//

	CurrentSelection& operator=(const Resource* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::resource;

		if (newSelection->GetType() == ResourceTypes::TextureResource)
			App->gui->panelInspector->SetTextureImportSettings(((ResourceTexture*)newSelection)->GetSpecificData().textureImportSettings);
		
		return *this;
	}

	bool operator==(const Resource* rhs)
	{
		return cur == rhs;
	}

	//Mesh Import Settings ----------------------------------
	CurrentSelection& operator=(ResourceMeshImportSettings& newSelection)
	{
		cur = (void*)&newSelection;
		type = SelectedType::meshImportSettings;

		App->gui->panelInspector->SetMeshImportSettings(newSelection);

		return *this;
	}
	//-------------------------------------------------------

	// Add operators in case of new kinds of selection :)
};
