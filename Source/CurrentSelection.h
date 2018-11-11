#include <assert.h>

class GameObject;
class MeshImportSettings;
class TextureImportSettings;
class Resource;

// Highly recomend using this instead of operator =. ////Take care of one line conditionals!!///
#define DESTROYANDSET(x) \
{ \
App->scene->selectedObject.DestroyImportSettings(); \
App->scene->selectedObject = x; \
} \

struct CurrentSelection
{
	enum class SelectedType { null, gameObject, scene, resourceMesh, meshImportSettings, resourceTexture, textureImportSettings };

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

	CurrentSelection& operator=(const ResourceMesh* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::resourceMesh;
		return *this;
	}

	bool operator==(const ResourceMesh* rhs)
	{
		return cur == rhs;
	}

	CurrentSelection& operator=(const ResourceTexture* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::resourceTexture;
		return *this;
	}

	bool operator==(const ResourceTexture* rhs)
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
		if (type != SelectedType::meshImportSettings || type != SelectedType::textureImportSettings)
			return false;
		delete cur;
		cur = nullptr;
		type = SelectedType::null;
	}

	// Add operators in case of new kinds of selection :)
};
