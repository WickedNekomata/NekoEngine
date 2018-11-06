#include "ModuleResourceManager.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"

#include "Application.h"

#include <assert.h> 

ModuleResourceManager::ModuleResourceManager() {}

ModuleResourceManager::~ModuleResourceManager() {}

update_status ModuleResourceManager::Update()
{
	timer += App->timeManager->GetRealDt();

	if (timer >= assetsCheckTime)
	{
		std::string newFileInAssets;
		if (App->filesystem->RecursiveFindNewFileInAssets("Assets", newFileInAssets))
			ImportFile(newFileInAssets.data());

		timer = 0.0f;
	}

	return UPDATE_CONTINUE;
}

bool ModuleResourceManager::CleanUp()
{
	assert(SomethingOnMemory() == false && "Memory still allocated on vram. Code better!");

	return true;
}

void ModuleResourceManager::SetAssetsCheckTime(float assetsCheckTime)
{
	this->assetsCheckTime = assetsCheckTime;

	if (this->assetsCheckTime > MAX_ASSETS_CHECK_TIME)
		this->assetsCheckTime = MAX_ASSETS_CHECK_TIME;
}

float ModuleResourceManager::GetAssetsCheckTime() const
{
	return assetsCheckTime;
}

// Returns the uuid associated to the resource of the file. In case of error returns 0.
uint ModuleResourceManager::Find(const char* fileInAssets) const
{
	for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetExportedFile(), fileInAssets) == 0)
			return it->first;
	}

	return 0;
}

// Import file into a resource. In case of error returns 0.
uint ModuleResourceManager::ImportFile(const char* newFileInAssets)
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

ResourceType ModuleResourceManager::GetResourceTypeByExtension(const char* extension)
{
	if (strcmp(extension, ".fbx") == 0 || strcmp(extension, ".FBX") == 0
		|| strcmp(extension, ".obj") == 0 || strcmp(extension, ".OBJ") == 0)
		return ResourceType::Mesh_Resource;
	else if (strcmp(extension, ".dds") == 0 || strcmp(extension, ".DDS")
		|| strcmp(extension, ".png") == 0 || strcmp(extension, ".PNG") == 0
		|| strcmp(extension, ".jpg") == 0 || strcmp(extension, ".JPG") == 0)
		return ResourceType::Material_Resource;

	return ResourceType::No_Type_Resource;
}

// Get resource associated to the uuid.
const Resource* ModuleResourceManager::GetResource(uint uuid) const
{
	std::map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it != resources.end())
		return it->second;

	return nullptr;
}

// First argument defines the kind of resource to create. Second argument is used to force and set the uuid.
// In case of uuid set to 0, a random uuid will be generated.
Resource* ModuleResourceManager::CreateNewResource(ResourceType type, uint force_uuid)
{
	assert(type != ResourceType::No_Type_Resource && "Invalid resource type");

	Resource* resource = nullptr;

	uint uuid = force_uuid;
	if (uuid <= 0)
		uuid = App->GenerateRandomNumber();

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

// Load resource to memory and return number of references. In case of error returns -1.
int ModuleResourceManager::SetAsUsed(uint uuid) const
{
	std::map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it == resources.end())
		return -1;

	return it->second->LoadToMemory();
}

// Unload resource from memory and return number of references. In case of error returns -1.
int ModuleResourceManager::SetAsUnused(uint uuid) const
{
	std::map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it == resources.end())
		return -1;

	return it->second->UnloadMemory();
}

// Returns true if resource associated to the uuid can be found and deleted. Returns false in case of error.
bool ModuleResourceManager::DestroyResource(uint uuid)
{
	std::map<uint, Resource*>::iterator it = resources.find(uuid);

	if (it == resources.end())
		return false;
	
	delete it->second;
	resources.erase(uuid);
	return true;
}

// Deletes all resources.
void ModuleResourceManager::DestroyResources()
{
	for (std::map<uint, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
		delete it->second;

	resources.clear();
}

// Returns true if someone is still referencing to any resource.
bool ModuleResourceManager::SomethingOnMemory() const
{
	for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->CountReferences() > 0)
			return true;
	}

	return false;
}
