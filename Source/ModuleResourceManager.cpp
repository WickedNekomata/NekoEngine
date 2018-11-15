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
	path.append(DIR_ASSETS);
	RecursiveImportFilesFromDir(DIR_ASSETS, path);

	// Remove any entries in Library that are not being used by the resources
	path.clear();
	path.append(DIR_LIBRARY);
	RecursiveDeleteUnusedFilesFromDir(DIR_LIBRARY, path);

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
	{
		CONSOLE_LOG("RESOURCE MANAGER: A new file '%s' has been added", event.fileEvent.file);

		// Import
		ImportFile(event.fileEvent.file);
	}
	break;

	case System_Event_Type::MetaRemoved:
	{
		// Path of the file in Assets associated to the meta
		std::string fileInAssets = event.fileEvent.metaFile;
		uint found = fileInAssets.find_last_of(".");
		if (found != std::string::npos)
			fileInAssets = fileInAssets.substr(0, found);

		CONSOLE_LOG("RESOURCE MANAGER: The meta '%s' has been removed", event.fileEvent.metaFile);

		std::string extension;
		App->fs->GetExtension(fileInAssets.data(), extension);
		ResourceType type = GetResourceTypeByExtension(extension.data());

		std::string entry;

		// Set its resources to invalid and remove its entries in Library
		for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			if (strcmp(it->second->GetFile(), fileInAssets.data()) == 0)
			{
				// Invalidate resource
				it->second->InvalidateResource();

				// Remove entry in Library
				switch (type)
				{
				case ResourceType::Mesh_Resource:
					entry = DIR_LIBRARY_MESHES;
					entry.append("/");
					entry.append(std::to_string(it->first));
					entry.append(EXTENSION_MESH);
					break;
				case ResourceType::Texture_Resource:
					entry = DIR_LIBRARY_MATERIALS;
					entry.append("/");
					entry.append(std::to_string(it->first));
					entry.append(EXTENSION_TEXTURE);
					break;
				}

				App->fs->DeleteFileOrDir(entry.data());
			}
		}

		// Also remove the meta from the metas map
		App->fs->DeleteMeta(event.fileEvent.metaFile);

		// Import
		ImportFile(fileInAssets.data());
	}
	break;

	case System_Event_Type::FileRemoved:
	{
		// Path of the file in Assets associated to the meta
		std::string fileInAssets = event.fileEvent.metaFile;
		uint found = fileInAssets.find_last_of(".");
		if (found != std::string::npos)
			fileInAssets = fileInAssets.substr(0, found);

		CONSOLE_LOG("RESOURCE MANAGER: The file '%s' has been removed", fileInAssets.data());

		std::string extension;
		App->fs->GetExtension(fileInAssets.data(), extension);
		ResourceType type = GetResourceTypeByExtension(extension.data());

		std::string entry;

		// Set its resources to invalid and remove its entries in Library
		for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			if (strcmp(it->second->GetFile(), fileInAssets.data()) == 0)
			{
				// Invalidate resource
				it->second->InvalidateResource();

				// Remove entry in Library
				switch (type)
				{
				case ResourceType::Mesh_Resource:
					entry = DIR_LIBRARY_MESHES;
					entry.append("/");
					entry.append(std::to_string(it->first));
					entry.append(EXTENSION_MESH);
					break;
				case ResourceType::Texture_Resource:
					entry = DIR_LIBRARY_MATERIALS;
					entry.append("/");
					entry.append(std::to_string(it->first));
					entry.append(EXTENSION_TEXTURE);
					break;
				}

				App->fs->DeleteFileOrDir(entry.data());
			}
		}

		// Remove its meta from the metas map before removing it completely
		App->fs->DeleteFileOrDir(event.fileEvent.metaFile);
		App->fs->DeleteMeta(event.fileEvent.metaFile);
	}
	break;

	case System_Event_Type::FileOverwritten:

		// Path of the file in Assets associated to the meta
		std::string fileInAssets = event.fileEvent.metaFile;
		uint found = fileInAssets.find_last_of(".");
		if (found != std::string::npos)
			fileInAssets = fileInAssets.substr(0, found);

		CONSOLE_LOG("RESOURCE MANAGER: The file '%s' has been overwritten", fileInAssets.data());

		std::string extension;
		App->fs->GetExtension(fileInAssets.data(), extension);
		ResourceType type = GetResourceTypeByExtension(extension.data());

		std::string entry;

		// Set its resources to invalid and remove its entries in Library
		for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			if (strcmp(it->second->GetFile(), fileInAssets.data()) == 0)
			{
				// Invalidate resource
				it->second->InvalidateResource();

				// Remove entry in Library
				switch (type)
				{
				case ResourceType::Mesh_Resource:
					entry = DIR_LIBRARY_MESHES;
					entry.append("/");
					entry.append(std::to_string(it->first));
					entry.append(EXTENSION_MESH);
					break;
				case ResourceType::Texture_Resource:
					entry = DIR_LIBRARY_MATERIALS;
					entry.append("/");
					entry.append(std::to_string(it->first));
					entry.append(EXTENSION_TEXTURE);
					break;
				}

				App->fs->DeleteFileOrDir(entry.data());
			}
		}

		// Reimport
		ImportFile(fileInAssets.data(), event.fileEvent.metaFile, nullptr);

		break;
	}
}

// Imports all files found in a directory (except scenes and metas)
void ModuleResourceManager::RecursiveImportFilesFromDir(const char* dir, std::string& path)
{
	if (dir == nullptr)
	{
		assert(dir != nullptr);
		return;
	}

	const char** files = App->fs->GetFilesFromDir(path.data());
	const char** it;

	path.append("/");

	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		if (App->fs->IsDirectory(path.data()))
			RecursiveImportFilesFromDir(*it, path);
		else
		{
			std::string extension;
			App->fs->GetExtension(*it, extension);

			// Ignore scenes and metas
			if (!IS_SCENE(extension.data()) && !IS_META(extension.data()))
				ImportFile(path.data());
		}

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}
}

// Deletes all files found in a directory (excepte scenes and metas) that are not being used by any resource
// Normally called to delete unused files in Library
void ModuleResourceManager::RecursiveDeleteUnusedFilesFromDir(const char* dir, std::string& path)
{
	if (dir == nullptr)
	{
		assert(dir != nullptr);
		return;
	}

	const char** files = App->fs->GetFilesFromDir(path.data());
	const char** it;

	path.append("/");

	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		if (App->fs->IsDirectory(path.data()))
			RecursiveDeleteUnusedFilesFromDir(*it, path);
		else
		{
			std::string extension;
			App->fs->GetExtension(*it, extension);

			// Ignore scenes and metas
			// If the file has no associated resource, then it is unused and must be deleted
			if (!IS_SCENE(extension.data()) && !IS_META(extension.data())
				&& FindByExportedFile(path.data()) <= 0)
				App->fs->DeleteFileOrDir(path.data());
		}

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}
}

// Determines how to import a file and calls ImportFile into a resource. If success, it returns the UUID of the resource. Otherwise, it returns 0
uint ModuleResourceManager::ImportFile(const char* fileInAssets)
{
	uint ret = 0;

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(fileInAssets) + 1, fileInAssets); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// CASE 1 (file). The file has no meta associated (the file is new)
	if (!App->fs->Exists(metaFile))
	{
		// Import the file (using the default import settings)
		CONSOLE_LOG("RESOURCE MANAGER: The file '%s' needs to be imported", fileInAssets);
		ret = ImportFile(fileInAssets, nullptr, nullptr);
	}
	else
	{
		bool entries = true;
		char exportedFile[DEFAULT_BUF_SIZE];

		std::string extension;
		App->fs->GetExtension(fileInAssets, extension);
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
					App->fs->GetFileName(fileInAssets, exportedFileName);
					sprintf_s(exportedFile, "%s/%s%s", DIR_ASSETS_SCENES, exportedFileName.data(), EXTENSION_SCENE);
					entries = true;
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
				entries = App->fs->Exists(exportedFile);
			}
		}
		break;
		}

		// CASE 2 (file + meta). The file(s) in Libray associated to the meta do(es)n't exist
		if (!entries)
		{
			// Reimport the file (using the import settings from the meta)
			CONSOLE_LOG("RESOURCE MANAGER: The file '%s' has Library file(s) that need(s) to be reimported", fileInAssets);
			ret = ImportFile(fileInAssets, metaFile, nullptr);
		}
		// CASE 3 (file + meta + Library file(s)). The resource(s) do(es)n't exist
		else
		{
			if (FindByFile(fileInAssets) == 0)
			{
				// Create the resources
				CONSOLE_LOG("RESOURCE MANAGER: The file '%s' has resources that need to be created", fileInAssets);
				ret = ImportFile(fileInAssets, metaFile, exportedFile);
			}
		}
	}

	return ret;
}

// Imports a file into a resource. If success, it returns the UUID of the resource. Otherwise, it returns 0
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
	std::string outputMetaFile;

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
				App->sceneImporter->GenerateMeta(resources, (MeshImportSettings*)importSettings, outputMetaFile);
			// Else, update the last modified time in the existing meta
			else
			{
				outputMetaFile = metaFile;
				int lastModTime = App->fs->GetLastModificationTime(fileInAssets);
				Importer::SetLastModificationTimeToMeta(metaFile, lastModTime);
			}
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
				App->materialImporter->GenerateMeta(resources.front(), (TextureImportSettings*)importSettings, outputMetaFile);
			// Else, update the last modified time in the existing meta
			else
			{
				outputMetaFile = metaFile;
				int lastModTime = App->fs->GetLastModificationTime(fileInAssets);
				Importer::SetLastModificationTimeToMeta(metaFile, lastModTime);
			}
		}
		break;
		}

		// Add the meta to the metas map to keep track of it
		if (!outputMetaFile.empty())
		{
			int lastModTime;
			App->sceneImporter->GetLastModificationTimeFromMeta(outputMetaFile.data(), lastModTime);
			App->fs->AddMeta(outputMetaFile.data(), lastModTime);
		}

		if (resources.size() > 0)
			ret = resources.front()->GetUUID();
	}

	RELEASE(importSettings);

	return ret;
}

// First argument defines the kind of resource to create. Second argument is used to force and set the UUID
// In case of UUID set to 0, a random UUID will be generated
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

// Returns the resource associated to the UUID. Otherwise, it returns nullptr
const Resource* ModuleResourceManager::GetResource(uint uuid) const
{
	auto it = resources.find(uuid);

	if (it != resources.end())
		return it->second;

	return nullptr;
}

// Returns the resource type that matches the extension
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

// Returns the UUID associated to the resource of the file. If error, it returns 0
uint ModuleResourceManager::FindByFile(const char* fileInAssets) const
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetFile(), fileInAssets) == 0)
			return it->first;
	}

	return 0;
}

// Returns the UUID associated to the resource of the exported file. If error, it returns 0
uint ModuleResourceManager::FindByExportedFile(const char* exportedFile) const
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetExportedFile(), exportedFile) == 0)
			return it->first;
	}

	return 0;
}

// Loads the resource to memory and returns the number of references. In case of error, it returns -1
int ModuleResourceManager::SetAsUsed(uint uuid) const
{
	auto it = resources.find(uuid);

	if (it == resources.end())
		return -1;

	return it->second->LoadToMemory();
}

// Unloads the resource from memory and returns the number of references. In case of error, it returns -1.
int ModuleResourceManager::SetAsUnused(uint uuid) const
{
	auto it = resources.find(uuid);

	if (it == resources.end())
		return -1;

	return it->second->UnloadMemory();
}

// Returns true if resource associated to the UUID can be found and deleted. In case of error, it returns false
bool ModuleResourceManager::DestroyResource(uint uuid)
{
	auto it = resources.find(uuid);

	if (it == resources.end())
		return false;
	
	delete it->second;
	resources.erase(uuid);
	return true;
}

// Deletes all resources
void ModuleResourceManager::DestroyResources()
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
		delete it->second;

	resources.clear();
}

// Returns true if someone is still referencing any resource
bool ModuleResourceManager::SomethingOnMemory() const
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->CountReferences() > 0)
			return true;
	}

	return false;
}
