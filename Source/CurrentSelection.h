#include <assert.h>
#include "SceneImporter.h"
#include "MaterialImporter.h"

class GameObject;
class Resource;

// Highly recomend using this instead of operator =. ////Take care of one line conditionals!!///
#define DESTROYANDSET(x) \
{ \
App->scene->selectedObject.DestroyImportSettings(); \
App->scene->selectedObject = x; \
} \

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
		assert(null == NULL && "Invalid comparasion");
		return cur == nullptr;
	}

	bool operator!=(const SelectedType rhs)
	{
		return type != rhs;
	}

	bool operator!=(int null)
	{
		assert(null == NULL && "Invalid comparasion");
		return cur != nullptr;
	}

	//-----------// GAMEOBJECTS //----------//

	CurrentSelection& operator=(GameObject* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::gameObject;
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

	bool DestroyImportSettings()
	{
		if (type != SelectedType::meshImportSettings && type != SelectedType::textureImportSettings)
			return false;

		switch (type) {
		case SelectedType::meshImportSettings:
		{
			MeshImportSettings* toRelease = (MeshImportSettings*)cur;
			RELEASE_ARRAY(toRelease->metaFile);
			break;
		}
		case SelectedType::textureImportSettings:
		{
			TextureImportSettings* toRelease = (TextureImportSettings*)cur;
			RELEASE_ARRAY(toRelease->metaFile);
			break;
		}
		}	
		delete cur;
		cur = nullptr;
		type = SelectedType::null;
		return true;
	}

	// Add operators in case of new kinds of selection :)
};
