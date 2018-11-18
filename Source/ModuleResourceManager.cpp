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

#include "Brofiler\Brofiler.h"

#include <sstream>
#include <assert.h>

ModuleResourceManager::ModuleResourceManager() {}

ModuleResourceManager::~ModuleResourceManager() {}

bool ModuleResourceManager::Start()
{
	std::string path = DIR_ASSETS;
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

	case System_Event_Type::ReimportFile:
	{
		// Path of the file in Assets associated to the meta
		std::string fileInAssets = event.fileEvent.metaFile;
		uint found = fileInAssets.find_last_of(".");
		if (found != std::string::npos)
			fileInAssets = fileInAssets.substr(0, found);

		CONSOLE_LOG("RESOURCE MANAGER: The file '%s' needs to be reimported", fileInAssets.data());

		App->res->DestroyResourcesAndRemoveLibraryEntries(event.fileEvent.metaFile);

		App->res->ImportFile(fileInAssets.data());
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

		DestroyResourcesAndRemoveLibraryEntries(event.fileEvent.metaFile);

		// Also remove the meta from the metas map
		App->fs->DeleteMeta(event.fileEvent.metaFile);

		// Import
		ImportFile(fileInAssets.data());
	}
	break;

	case System_Event_Type::FileRemoved:
	{
		CONSOLE_LOG("RESOURCE MANAGER: The file with the meta '%s' has been removed", event.fileEvent.metaFile);

		DestroyResourcesAndRemoveLibraryEntries(event.fileEvent.metaFile);

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
		
		DestroyResourcesAndRemoveLibraryEntries(event.fileEvent.metaFile);

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
			if (!IS_SCENE(extension.data()) && !IS_META(extension.data()))
			{
				bool resources = false;

				if (strcmp(extension.data(), EXTENSION_MESH))
				{
					std::list<uint> UUIDs;
					resources = FindMeshesByExportedFile(path.data(), UUIDs);
				}
				else if (strcmp(extension.data(), EXTENSION_TEXTURE))
				{
					uint UUID;
					resources = FindTextureByExportedFile(path.data(), UUID);
				}

				if (!resources)
					App->fs->DeleteFileOrDir(path.data());
			}
		}

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}
}

// Determines how to import a file and calls ImportFile into a resource. If success, it returns the UUID of the resource. Otherwise, it returns 0
uint ModuleResourceManager::ImportFile(const char* fileInAssets)
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	uint ret = 0;

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(fileInAssets) + 1, fileInAssets); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// CASE 1 (file). The file has no meta associated (the file is new)
	if (!App->fs->Exists(metaFile))
	{
		// Import the file (using the default import settings)
		if (fileInAssets == "Assets/")
		{
			// TODO: DELETE THIS. JUST DEBUG PURPOSES
			CONSOLE_LOG("ERROR. THE FILE THAT YOU WANT TO IMPORT IS CALLED ASSETS/. WHAT ARE YOU TRYING TO DO...?");
			return ret;
		}

		CONSOLE_LOG("RESOURCE MANAGER: The file '%s' needs to be imported", fileInAssets);
		ret = ImportFile(fileInAssets, nullptr, nullptr);
	}
	else
	{
		bool entries = false;

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
				if (meshes > 0 && meshes == UUIDs.size())
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
		else
		{
			bool resources = false;

			switch (type)
			{
			case ResourceType::Mesh_Resource:
			{
				std::list<uint> UUIDs;
				resources = FindMeshesByFile(fileInAssets, UUIDs);
			}
			break;
			case ResourceType::Texture_Resource:
			{
				uint UUID;
				resources = FindTextureByFile(fileInAssets, UUID);
			}
			break;
			}

			// CASE 3 (file + meta + Library file(s)). The resource(s) do(es)n't exist
			if (!resources)
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
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

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
			// Else, update the UUIDs and the last modified time in the existing meta
			else
			{
				outputMetaFile = metaFile;

				App->sceneImporter->SetMeshUUIDsToMeta(metaFile, meshesUUIDs);

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
			// Else, update the UUIDs and the last modified time in the existing meta
			else
			{
				outputMetaFile = metaFile;

				App->materialImporter->SetTextureUUIDToMeta(metaFile, UUID);

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
	union
	{
		char ext[4];
		uint32_t asciiValue;
	} asciiUnion;

	for (int i = 0; i < 4; ++i)
		asciiUnion.ext[i] = extension[i];

	switch (asciiUnion.asciiValue)
	{
	case ASCIIfbx: case ASCIIFBX: case ASCIIobj: case ASCIIOBJ: case ASCIIdae: case ASCIIDAE:
		return ResourceType::Mesh_Resource;
		break;
	case ASCIIdds: case ASCIIDDS: case ASCIIpng: case ASCIIPNG: case ASCIIjpg: case ASCIIJPG:
	case ASCIItga: case ASCIITGA:
		return ResourceType::Texture_Resource;
		break;
	}

	return ResourceType::No_Type_Resource;
}

// Returns the UUID associated to the resource of the file
bool ModuleResourceManager::FindTextureByFile(const char* fileInAssets, uint& UUID) const
{
	UUID = 0;

	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetFile(), fileInAssets) == 0)
		{
			UUID = it->first;
			return true;
		}
	}

	return false;
}

// Returns the UUID associated to the resource of the exported file
bool ModuleResourceManager::FindTextureByExportedFile(const char* exportedFile, uint& UUID) const
{
	UUID = 0;

	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetExportedFile(), exportedFile) == 0)
		{
			UUID = it->first;
			return true;
		}
	}

	return false;
}

// Returns the UUID(s) associated to the resource(s) of the file
bool ModuleResourceManager::FindMeshesByFile(const char* fileInAssets, std::list<uint>& UUIDs) const
{
	bool ret = false;

	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetFile(), fileInAssets) == 0)
		{
			uint UUID = it->first;
			UUIDs.push_back(UUID);
			ret = true;
		}
	}

	return ret;
}

// Returns the UUID(s) associated to the resource(s) of the file
bool ModuleResourceManager::FindMeshesByExportedFile(const char* exportedFile, std::list<uint>& UUIDs) const
{
	bool ret = false;

	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetExportedFile(), exportedFile) == 0)
		{
			uint UUID = it->first;
			UUIDs.push_back(UUID);
			ret = true;
		}
	}

	return ret;
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
int ModuleResourceManager::SetAsUnused(uint UUID) const
{
	auto it = resources.find(UUID);

	if (it == resources.end())
		return -1;

	return it->second->UnloadMemory();
}

// Returns true if the resource associated to the UUID can be found and deleted. In case of error, it returns false
bool ModuleResourceManager::DestroyResource(uint UUID)
{
	auto it = resources.find(UUID);

	if (it == resources.end())
		return false;
	
	delete it->second;
	resources.erase(UUID);
	return true;
}

bool ModuleResourceManager::DestroyResources(std::list<uint> UUIDs)
{
	for (std::list<uint>::const_iterator it = UUIDs.begin(); it != UUIDs.end(); ++it)
	{
		auto res = resources.find(*it);

		if (res != resources.end())
		{
			delete res->second;
			resources.erase(*it);
		}
		else
			return false;
	}

	return true;
}

bool ModuleResourceManager::RemoveTextureLibraryEntry(uint UUID)
{
	std::string entry;

	entry = DIR_LIBRARY_MATERIALS;
	entry.append("/");
	entry.append(std::to_string(UUID));
	entry.append(EXTENSION_TEXTURE);

	// Remove entry in Library
	return App->fs->DeleteFileOrDir(entry.data());
}

bool ModuleResourceManager::RemoveMeshesLibraryEntries(std::list<uint> UUIDs)
{
	bool ret = false;

	std::string entry;

	for (std::list<uint>::const_iterator it = UUIDs.begin(); it != UUIDs.end(); ++it)
	{
		entry = DIR_LIBRARY_MESHES;
		entry.append("/");
		entry.append(std::to_string(*it));
		entry.append(EXTENSION_MESH);

		// Remove entry in Library
		ret = App->fs->DeleteFileOrDir(entry.data());
	}

	return ret;
}

bool ModuleResourceManager::DestroyResourcesAndRemoveLibraryEntries(const char* metaFile)
{
	bool ret = false;

	// Path of the file in Assets associated to the meta
	std::string fileInAssets = metaFile;
	uint found = fileInAssets.find_last_of(".");
	if (found != std::string::npos)
		fileInAssets = fileInAssets.substr(0, found);

	std::string extension;
	App->fs->GetExtension(fileInAssets.data(), extension);
	ResourceType type = GetResourceTypeByExtension(extension.data());

	switch (type)
	{
	case ResourceType::Mesh_Resource:
	{
		std::list<uint> UUIDs;
		if (App->sceneImporter->GetMeshesUUIDsFromMeta(metaFile, UUIDs))
		{
			ret = DestroyResources(UUIDs);
			if (ret)
				ret = RemoveMeshesLibraryEntries(UUIDs);
		}
	}
	break;
	case ResourceType::Texture_Resource:
	{
		uint UUID;
		if (App->materialImporter->GetTextureUUIDFromMeta(metaFile, UUID))
		{
			ret = DestroyResource(UUID);
			if (ret)
				ret = RemoveTextureLibraryEntry(UUID);
		}
	}
	break;
	}

	return ret;
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
