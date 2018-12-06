#include "Application.h"
#include "ModuleCameraEditor.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"

#include <assert.h>

class Resource;

struct CurrentSelection
{
	enum class SelectedType { null, gameObject, scene, resource, meshImportSettings, textureImportSettings };

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
		App->camera->SetReference(newSelection->transform->position);
#endif

		return *this;
	}

	bool operator==(const GameObject* rhs)
	{
		return cur == rhs;
	}

	//-----------// RESOURCES //----------//

	CurrentSelection& operator=(const Resource* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::resource;
		return *this;
	}

	bool operator==(const Resource* rhs)
	{
		return cur == rhs;
	}

	//-----------// IMPORT SETTINGS //----------//

	CurrentSelection& operator=(MeshImportSettings* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::meshImportSettings;
		return *this;
	}

	CurrentSelection& operator=(TextureImportSettings* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::textureImportSettings;
		return *this;
	}

	bool operator==(const MeshImportSettings* rhs)
	{
		return cur == rhs;
	}

	bool operator==(const TextureImportSettings* rhs)
	{
		return cur == rhs;
	}

	// Add operators in case of new kinds of selection :)
};
