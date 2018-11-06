#include "ResourceManager.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"

#include "Application.h"

#include <assert.h> 

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

// Returns the uuid associated to the resource of the file. In case of error returns 0.
uint ResourceManager::Find(const char* fileInAssets) const
{
	for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetExportedFile(), fileInAssets) == 0)
			return it->first;
	}

	return 0;
}

// Import file into a resource. In case of error returns 0.
uint ResourceManager::ImportFile(const char* newFileInAssets)
{
	uint ret = 0;

	bool imported = false;
	std::string outputFileName;

	std::string extension;
	App->filesystem->GetExtension(newFileInAssets, extension);
	ResourceType type = GetResourceTypeByExtension(extension.data());

	switch (type)
	{
	case ResourceType::Mesh_Resource:
		imported = App->sceneImporter->Import(nullptr, newFileInAssets, outputFileName);
		break;
	case ResourceType::Material_Resource:
		imported = App->materialImporter->Import(nullptr, newFileInAssets, outputFileName);
		break;
	case ResourceType::No_Type_Resource:
	default:
		break;
	}

	if (imported)
	{
		Resource* resource = CreateNewResource(type);
		resource->file = newFileInAssets;
		resource->exportedFileName = outputFileName;
		ret = resource->GetUUID();

		// Generate the meta
		switch (type)
		{
		case ResourceType::Mesh_Resource:
			App->sceneImporter->GenerateMeta(resource);
			break;
		case ResourceType::Material_Resource:
			App->materialImporter->GenerateMeta(resource);
			break;
		}
	}

	return ret;
}

ResourceType ResourceManager::GetResourceTypeByExtension(const char* extension)
{
	if (strcmp(extension, "fbx") || strcmp(extension, "obj"))
		return ResourceType::Mesh_Resource;
	else if (strcmp(extension, "dds") || strcmp(extension, "png") || strcmp(extension, "jpg"))
		return ResourceType::Material_Resource;

	return ResourceType::No_Type_Resource;
}

// Get resource associated to the uuid.
const Resource* ResourceManager::GetResource(uint uuid) const
{
	std::map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it != resources.end())
		return it->second;

	return nullptr;
}

// First argument defines the kind of resource to create. Second argument is used to force and set the uuid.
// In case of uuid set to 0, a random uuid will be generated.
Resource* ResourceManager::CreateNewResource(ResourceType type, uint force_uuid)
{
	assert(type != ResourceType::No_Type_Resource && "Invalid resource type");

	Resource* resource = nullptr;

	uint uuid = force_uuid;
	if (uuid <= 0)
		uuid = pcg32_random_r(&(App->rng));

	switch (type)
	{
	case ResourceType::Mesh_Resource:
		resource = new ResourceMesh(type, uuid);
		break;
	case ResourceType::Material_Resource:
		resource = new ResourceMaterial(type, uuid);
		break;
	}

	if (resource != nullptr)
		resources[uuid] = resource;

	return resource;
}

// Returns true if resource associated to the uuid can be found and deleted. Returns false in case of error.
bool ResourceManager::DestroyResource(uint uuid)
{
	std::map<uint, Resource*>::iterator it = resources.find(uuid);

	if (it == resources.end())
		return false;
	
	delete it->second;
	resources.erase(uuid);
	return true;
}

// Deletes all resources.
void ResourceManager::DestroyResources()
{
	for (std::map<uint, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
		delete it->second;

	resources.clear();
}

bool ResourceManager::SomethingOnMemory() const
{
	for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->CountReferences() > 0)
			return true;
	}

	return false;
}
