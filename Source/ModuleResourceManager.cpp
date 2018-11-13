#include "ModuleResourceManager.h"
#include "MaterialImporter.h"
#include "SceneImporter.h"
#include "Application.h"
#include "ModuleGOs.h"
#include "ModuleFileSystem.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"

#include "Application.h"

#include <sstream>
#include <assert.h>

ModuleResourceManager::ModuleResourceManager() {}

ModuleResourceManager::~ModuleResourceManager() {}

bool ModuleResourceManager::Start()
{
	std::string path;
	RecursiveCreateResourcesFromFilesInAssets(DIR_ASSETS, path);

	return true;
}

bool ModuleResourceManager::CleanUp()
{
	assert(SomethingOnMemory() == false && "Memory still allocated on vram. Code better!");

	DestroyResources();

	return true;
}

void ModuleResourceManager::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::FileDropped:
	{
		std::string extension;
		App->fs->GetExtension(event.fileEvent.file, extension);

		if (GetResourceTypeByExtension(extension.data()) != ResourceType::No_Type_Resource)
		{
			CONSOLE_LOG("FILE SYSTEM: The file '%s' has been dropped and needs to be copied to Assets", event.fileEvent.file);

			std::string outputFile;
			App->fs->Copy(event.fileEvent.file, DIR_ASSETS, outputFile);
		}
	}
	break;

	case System_Event_Type::NewFile:

		// Import
		ImportFile(event.fileEvent.file);

		break;

	case System_Event_Type::MetaRemoved:

		// Import
		ImportFile(event.fileEvent.file);

		break;

	case System_Event_Type::FileRemoved:
	{
		std::string extension;
		App->fs->GetExtension(event.fileEvent.file, extension);

		// Set its resources to invalid and remove its entries in Library
		switch (GetResourceTypeByExtension(extension.data()))
		{
		case ResourceType::Mesh_Resource:
		{
			std::list<uint> UUIDs;
			App->sceneImporter->GetMeshesUUIDsFromMeta(event.fileEvent.metaFile, UUIDs);

			std::string entry;
			for (std::list<uint>::const_iterator it = UUIDs.begin(); it != UUIDs.end(); ++it)
			{
				// Invalidate resources


				entry = DIR_LIBRARY_MESHES;
				entry.append("/");
				entry.append(std::to_string(*it));
				entry.append(EXTENSION_MESH);

				App->fs->DeleteFileOrDir(entry.data());
			}
		}
		break;
		case ResourceType::Texture_Resource:

			uint UUID;
			App->materialImporter->GetTextureUUIDFromMeta(event.fileEvent.metaFile, UUID);

			// Invalidate resource

			std::string entry;
			entry = DIR_LIBRARY_MATERIALS;
			entry.append("/");
			entry.append(std::to_string(UUID));
			entry.append(EXTENSION_TEXTURE);

			App->fs->DeleteFileOrDir(entry.data());

			break;
		}

		// Remove its meta
		App->fs->DeleteFileOrDir(event.fileEvent.metaFile);
	}
	break;

	case System_Event_Type::FileOverwritten:

		// Reimport
		ImportFile(event.fileEvent.file, event.fileEvent.metaFile);

		break;
	}
}

void ModuleResourceManager::RecursiveCreateResourcesFromFilesInAssets(const char* dir, std::string& path)
{
	if (dir == nullptr)
	{
		assert(dir != nullptr);
		return;
	}

	path.append(dir);
	path.append("/");

	const char** files = App->fs->GetFilesFromDir(dir);
	const char** it;

	for (it = files; *it != nullptr; ++it)
	{
		if (App->fs->IsDirectory(*it))
		{
			RecursiveCreateResourcesFromFilesInAssets(*it, path);

			uint found = path.rfind(*it);
			if (found != std::string::npos)
				path = path.substr(0, found);
		}
		else
		{
			std::string extension;
			App->fs->GetExtension(*it, extension);

			// Ignore scenes and metas
			if (IS_SCENE(extension.data()) || IS_META(extension.data()))
				continue;

			// Search for the meta associated to the file
			char metaFile[DEFAULT_BUF_SIZE];
			strcpy_s(metaFile, strlen(path.data()) + 1, path.data()); // path
			strcat_s(metaFile, strlen(metaFile) + strlen(*it) + 1, *it); // fileName
			strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

			std::string file = path;

			// CASE 1 (file). The file has no meta associated (the file is new)
			if (!App->fs->Exists(metaFile))
			{
				// Import the file (using the default import settings)
				CONSOLE_LOG("FILE SYSTEM: There is a new file '%s' in %s that needs to be imported", *it, path.data());
				file.append(*it);
				ImportFile(file.data());
			}
			else
			{
				bool exists = true;
				char exportedFile[DEFAULT_BUF_SIZE];

				ResourceType type = GetResourceTypeByExtension(extension.data());

				switch (type)
				{
				case ResourceType::Mesh_Resource:
				{
					std::list<uint> UUIDs;
					if (App->sceneImporter->GetMeshesUUIDsFromMeta(metaFile, UUIDs))
					{
						uint meshes = 0;
						for (std::list<uint>::const_iterator it = UUIDs.begin(); it != UUIDs.end(); ++it)
						{
							sprintf_s(exportedFile, "%s/%u%s", DIR_LIBRARY_MESHES, *it, EXTENSION_MESH);
							if (App->fs->Exists(exportedFile))
								meshes++;
						}
						if (meshes == UUIDs.size())
						{
							std::string exportedFileName;
							App->fs->GetFileName(*it, exportedFileName);
							sprintf_s(exportedFile, "%s/%s%s", DIR_ASSETS_SCENES, exportedFileName.data(), EXTENSION_SCENE);

							exists = true;
						}
					}
				}
				break;

				case ResourceType::Texture_Resource:
				{
					uint UUID;
					if (App->materialImporter->GetTextureUUIDFromMeta(metaFile, UUID))
					{
						sprintf_s(exportedFile, "%s/%u%s", DIR_LIBRARY_MATERIALS, UUID, EXTENSION_TEXTURE);
						exists = App->fs->Exists(exportedFile);
					}
				}
				break;
				}

				// CASE 2 (file + meta). The file(s) in Libray associated to the meta do(es)n't exist
				if (!exists)
				{
					// Reimport the file (using the import settings from the meta)
					CONSOLE_LOG("FILE SYSTEM: There is a file '%s' in %s which Library file(s) need(s) to be reimported", *it, path.data());
					file.append(*it);
					ImportFile(file.data(), metaFile);
				}
				// CASE 3 (file + meta + Library file(s)). The resource(s) do(es)n't exist
				else
				{
					file.append(*it);

					if (Find(file.data()) == 0)
					{
						// Create the resources
						CONSOLE_LOG("FILE SYSTEM: There is a file '%s' in %s which resources need to be created", *it, path.data());
						ImportFile(file.data(), metaFile, exportedFile);
					}
				}
			}
		}
	}
}

// Returns the UUID associated to the resource of the file. In case of error, it returns 0
uint ModuleResourceManager::Find(const char* fileInAssets) const
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetFile(), fileInAssets) == 0)
			return it->first;
	}

	return 0;
}

// Imports a file into a resource. If case of success, it returns the UUID of the resource. Otherwise, it returns 0
uint ModuleResourceManager::ImportFile(const char* fileInAssets, const char* metaFile, const char* exportedFile)
{
	uint ret = 0;

	if (fileInAssets == nullptr)
	{
		assert(fileInAssets != nullptr);
		return ret;
	}

	bool imported = false;
	ImportSettings* importSettings = nullptr;
	std::string outputFile;

	std::string extension;
	App->fs->GetExtension(fileInAssets, extension);
	ResourceType type = GetResourceTypeByExtension(extension.data());

	// If the file has no file(s) in Libray or its import settings have changed, import or reimport the file
	if (exportedFile == nullptr)
	{
		// Initialize the import settings to the default import settings
		switch (type)
		{
		case ResourceType::Mesh_Resource:
			importSettings = new MeshImportSettings();
			break;
		case ResourceType::Texture_Resource:
			importSettings = new TextureImportSettings();
			break;
		}

		// If the file has a meta associated, use the import settings from the meta
		if (metaFile != nullptr)
		{
			switch (type)
			{
			case ResourceType::Mesh_Resource:
				App->sceneImporter->GetMeshImportSettingsFromMeta(metaFile, (MeshImportSettings*)importSettings);
				break;
			case ResourceType::Texture_Resource:
				App->materialImporter->GetTextureImportSettingsFromMeta(metaFile, (TextureImportSettings*)importSettings);
				break;
			}
		}

		// Import the file using the import settings
		switch (type)
		{
		case ResourceType::Mesh_Resource:
			imported = App->sceneImporter->Import(fileInAssets, outputFile, importSettings); // Models' outputFileName is the name of the Scene
			break;
		case ResourceType::Texture_Resource:
			imported = App->materialImporter->Import(fileInAssets, outputFile, importSettings); // Textures' outputFileName is the name of the file in Library, which is its UUID
			break;
		}
	}
	else
	{
		imported = true;
		outputFile = exportedFile;
	}

	if (imported)
	{
		std::list<Resource*> resources;

		switch (type)
		{
		case ResourceType::Mesh_Resource:
		{
			// Create a new resource for each mesh
			std::list<uint> meshesUUIDs;
			App->GOs->GetMeshResourcesFromScene(outputFile.data(), meshesUUIDs);

			for (auto it = meshesUUIDs.begin(); it != meshesUUIDs.end(); ++it)
			{
				Resource* resource = CreateNewResource(type, *it);
				resource->file = fileInAssets;

				resource->exportedFile = DIR_LIBRARY_MESHES;
				resource->exportedFile.append("/");
				resource->exportedFile.append(std::to_string(*it));
				resource->exportedFile.append(EXTENSION_MESH);

				resources.push_back(resource);
			}

			// If the file has no meta associated, generate a new meta
			if (metaFile == nullptr)
				App->sceneImporter->GenerateMeta(resources, (MeshImportSettings*)importSettings);
		}
		break;
		case ResourceType::Texture_Resource:
		{
			std::string outputFileName;
			App->fs->GetFileName(outputFile.data(), outputFileName);
			
			uint UUID = strtoul(outputFileName.data(), NULL, 0);
			
			// Create a new resource for the texture
			Resource* resource = CreateNewResource(type, UUID);
			resource->file = fileInAssets;
			resource->exportedFile = outputFile;
			resources.push_back(resource);

			// If the file has no meta associated, generate a new meta
			if (metaFile == nullptr)
				App->materialImporter->GenerateMeta(resources.front(), (TextureImportSettings*)importSettings);
		}
		break;
		}

		if (resources.size() > 0)
			ret = resources.front()->GetUUID();
	}

	RELEASE(importSettings);

	return ret;
}

ResourceType ModuleResourceManager::GetResourceTypeByExtension(const char* extension)
{
	uint64_t asciiValue;
	std::stringstream ascii;
	for (const char* it = extension; *it; ++it)
		ascii << uint16_t((*it));
	ascii >> asciiValue;

	switch (asciiValue)
	{
	case ASCIIfbx: case ASCIIFBX: case ASCIIobj: case ASCIIOBJ:
		return ResourceType::Mesh_Resource;
		break;
	case ASCIIdds: case ASCIIDDS: case ASCIIpng: case ASCIIPNG: case ASCIIjpg: case ASCIIJPG:
		return ResourceType::Texture_Resource;
		break;
	}

	return ResourceType::No_Type_Resource;
}

// Get the resource associated to the UUID
const Resource* ModuleResourceManager::GetResource(uint uuid) const
{
	auto it = resources.find(uuid);

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
	case ResourceType::Texture_Resource:
		resource = new ResourceTexture(type, uuid);
		break;
	}

	if (resource != nullptr)
		resources[uuid] = resource;

	return resource;
}

// Load resource to memory and return number of references. In case of error returns -1.
int ModuleResourceManager::SetAsUsed(uint uuid) const
{
	auto it = resources.find(uuid);

	if (it == resources.end())
		return -1;

	return it->second->LoadToMemory();
}

// Unload resource from memory and return number of references. In case of error returns -1.
int ModuleResourceManager::SetAsUnused(uint uuid) const
{
	auto it = resources.find(uuid);

	if (it == resources.end())
		return -1;

	return it->second->UnloadMemory();
}

// Returns true if resource associated to the uuid can be found and deleted. Returns false in case of error.
bool ModuleResourceManager::DestroyResource(uint uuid)
{
	auto it = resources.find(uuid);

	if (it == resources.end())
		return false;
	
	delete it->second;
	resources.erase(uuid);
	return true;
}

// Deletes all resources.
void ModuleResourceManager::DestroyResources()
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
		delete it->second;

	resources.clear();
}

// Returns true if someone is still referencing to any resource.
bool ModuleResourceManager::SomethingOnMemory() const
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->CountReferences() > 0)
			return true;
	}

	return false;
}
