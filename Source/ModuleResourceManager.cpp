#include "ModuleResourceManager.h"

#include "Application.h"
#include "ModuleFileSystem.h"

#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ShaderImporter.h"

#include "ResourceTypes.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"
#include "ResourceAnimation.h"
#include "ResourceBone.h"
#include "ResourceScript.h"

#include <assert.h>

ModuleResourceManager::ModuleResourceManager(bool start_enabled) : Module(start_enabled)
{
	name = "ResourceManager";
}

ModuleResourceManager::~ModuleResourceManager() {}

bool ModuleResourceManager::Start()
{
	return true;
}

bool ModuleResourceManager::CleanUp()
{
	DeleteResources();

	return true;
}

void ModuleResourceManager::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::NewFile:
	case System_Event_Type::ImportFile:
	{
		// 1. Import file
		ImportFile(event.fileEvent.file);
	}
	break;

	case System_Event_Type::FileOverwritten:
	case System_Event_Type::ReImportFile:
	{
		// 1. Delete resource(s)
		std::vector<uint> resourcesUuids;
		if (GetResourcesUuidsByFile(event.fileEvent.file, resourcesUuids))
			DeleteResources(resourcesUuids); // TODO: don't delete script resources

		// 2. Import file
		ImportFile(event.fileEvent.file);
	}
	break;

	case System_Event_Type::FileMoved:
	{
		// 1. Move meta

		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(event.fileEvent.file) + 1, event.fileEvent.file); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		std::string dir;
		App->fs->GetPath(event.fileEvent.newFileLocation, dir, false);
		std::string outputFile;
		App->fs->Copy(metaFile, dir.data(), outputFile);
		App->fs->deleteFile(metaFile);

		// 2. Update resource(s)
		std::vector<uint> resourcesUuids;
		if (GetResourcesUuidsByFile(event.fileEvent.file, resourcesUuids))
		{
			for (uint i = 0; i < resourcesUuids.size(); ++i)
				GetResource(resourcesUuids[i])->SetFile(event.fileEvent.newFileLocation);
		}
	}
	break;

	case System_Event_Type::FileRemoved:
	{
		// 1. Delete meta

		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(event.fileEvent.file) + 1, event.fileEvent.file); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		App->fs->deleteFile(metaFile);

		// 2. Delete entries
		std::vector<uint> resourcesUuids;
		bool resources = GetResourcesUuidsByFile(event.fileEvent.file, resourcesUuids);
		if (resources)
		{
			for (uint i = 0; i < resourcesUuids.size(); ++i)
				App->fs->deleteFile(GetResource(resourcesUuids[i])->GetExportedFile());
		}

		// 3. Delete resource(s)
		if (resources)
			DeleteResources(resourcesUuids);
	}
	break;

	case System_Event_Type::ForceReImport:
	{
		// 1. Delete meta

		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(event.fileEvent.file) + 1, event.fileEvent.file); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		App->fs->deleteFile(metaFile);

		// 2. Delete entries
		std::vector<uint> resourcesUuids;
		bool resources = GetResourcesUuidsByFile(event.fileEvent.file, resourcesUuids);
		if (resources)
		{
			for (uint i = 0; i < resourcesUuids.size(); ++i)
				App->fs->deleteFile(GetResource(resourcesUuids[i])->GetExportedFile());
		}

		// 3. Delete resource(s)
		if (resources)
			DeleteResources(resourcesUuids);

		// 4. Import file
		ImportFile(event.fileEvent.file);
	}
	break;

	case System_Event_Type::DeleteUnusedFiles:
	{
		// 1. Delete unused metas
		std::string path = DIR_ASSETS;
		RecursiveDeleteUnusedMetas(DIR_ASSETS, path);

		// 2. Delete unused entries
		path = DIR_LIBRARY;
		RecursiveDeleteUnusedEntries(DIR_LIBRARY, path);
	}
	break;
	}
}

// ----------------------------------------------------------------------------------------------------

// Note: in case of a mesh resource, it returns the last mesh resource created
Resource* ModuleResourceManager::ImportFile(const char* file)
{
	assert(file != nullptr);

	Resource* resource = nullptr;

	std::string extension;
	App->fs->GetExtension(file, extension);
	ResourceTypes type = GetResourceTypeByExtension(extension.data());

	switch (type)
	{
	case ResourceTypes::MeshResource:
	{
		ResourceMeshImportSettings meshImportSettings;
		std::vector<std::string> outputFiles;
		if (ResourceMesh::ImportFile(file, meshImportSettings, outputFiles))
		{
			std::vector<uint> resourcesUuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The mesh file '%s' has resources that need to be created", file);

				// 1. Meshes
				resourcesUuids.reserve(outputFiles.size());
				for (uint i = 0; i < outputFiles.size(); ++i)
				{
					std::string fileName;
					App->fs->GetFileName(outputFiles[i].data(), fileName);
					uint uuid = strtoul(fileName.data(), NULL, 0);
					assert(uuid > 0);

					ResourceData data;
					ResourceMeshData meshData;
					data.file = file;
					data.exportedFile = outputFiles[i].data();
					App->fs->GetFileName(file, data.name);
					meshData.meshImportSettings = meshImportSettings;
					strcpy((char*)meshData.meshImportSettings.modelPath, file);
					App->sceneImporter->Load(outputFiles[i].data(), data, meshData);

					resource = CreateResource(ResourceTypes::MeshResource, data, &meshData, uuid);
					if (resource != nullptr)
						resourcesUuids.push_back(uuid);
				}
				resourcesUuids.shrink_to_fit();
			}

			// 2. Meta
			// TODO: only create meta if any of its fields has been modificated
			std::string outputMetaFile;
			int64_t lastModTime = ResourceMesh::CreateMeta(file, meshImportSettings, resourcesUuids, outputMetaFile);
			assert(lastModTime > 0);
		}
	}
	break;

	case ResourceTypes::TextureResource:
	{
		ResourceTextureImportSettings textureImportSettings;
		std::string outputFile;
		if (ResourceTexture::ImportFile(file, textureImportSettings, outputFile))
		{
			std::vector<uint> resourcesUuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The texture file '%s' has resources that need to be created", file);

				// 1. Texture
				std::string fileName;
				App->fs->GetFileName(outputFile.data(), fileName);
				uint uuid = strtoul(fileName.data(), NULL, 0);
				assert(uuid > 0);
				resourcesUuids.push_back(uuid);
				resourcesUuids.shrink_to_fit();

				ResourceData data;
				ResourceTextureData textureData;
				data.file = file;
				data.exportedFile = outputFile.data();
				App->fs->GetFileName(file, data.name);
				textureData.textureImportSettings = textureImportSettings;

				resource = CreateResource(ResourceTypes::TextureResource, data, &textureData, uuid);
			}

			// 2. Meta
			// TODO: only create meta if any of its fields has been modificated
			std::string outputMetaFile;
			int64_t lastModTime = ResourceTexture::CreateMeta(file, textureImportSettings, resourcesUuids.front(), outputMetaFile);
			assert(lastModTime > 0);
		}
	}
	break;

	case ResourceTypes::ShaderObjectResource:
	{
		std::string outputFile;
		std::string name;
		if (ResourceShaderObject::ImportFile(file, name, outputFile))
		{
			std::vector<uint> resourcesUuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The shader object file '%s' has resources that need to be created", file);

				// 1. Shader object
				uint uuid = outputFile.empty() ? App->GenerateRandomNumber() : strtoul(outputFile.data(), NULL, 0);
				assert(uuid > 0);
				resourcesUuids.push_back(uuid);
				resourcesUuids.shrink_to_fit();

				ResourceData data;
				ResourceShaderObjectData shaderObjectData;
				data.file = file;
				if (name.empty())
					App->fs->GetFileName(file, data.name);
				else
					data.name = name.data();

				if (IS_VERTEX_SHADER(extension.data()))
					shaderObjectData.shaderType = ShaderTypes::VertexShaderType;
				else if (IS_FRAGMENT_SHADER(extension.data()))
					shaderObjectData.shaderType = ShaderTypes::FragmentShaderType;

				uint shaderObject = 0;
				bool success = App->shaderImporter->LoadShaderObject(file, shaderObjectData, shaderObject);

				resource = CreateResource(ResourceTypes::ShaderObjectResource, data, &shaderObjectData, uuid);
				ResourceShaderObject* shaderObjectResource = (ResourceShaderObject*)resource;
				shaderObjectResource->isValid = success;
				if (success)
					shaderObjectResource->shaderObject = shaderObject;
			}
			else
				resource = GetResource(resourcesUuids.front());

			// 2. Meta
			// TODO: only create meta if any of its fields has been modificated
			std::string outputMetaFile;
			std::string name = resource->GetName();
			int64_t lastModTime = ResourceShaderObject::CreateMeta(file, resourcesUuids.front(), name, outputMetaFile);
			assert(lastModTime > 0);
		}
	}
	break;

	case ResourceTypes::ShaderProgramResource:
	{
		std::string outputFile;
		std::string name;
		std::vector<std::string> shaderObjectsNames;
		if (ResourceShaderProgram::ImportFile(file, name, shaderObjectsNames, outputFile))
		{
			std::vector<uint> resourcesUuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The shader program file '%s' has resources that need to be created", file);

				// 1. Shader program
				uint uuid = outputFile.empty() ? App->GenerateRandomNumber() : strtoul(outputFile.data(), NULL, 0);
				assert(uuid > 0);
				resourcesUuids.push_back(uuid);
				resourcesUuids.shrink_to_fit();

				ResourceData data;
				ResourceShaderProgramData shaderProgramData;
				data.file = file;
				if (name.empty())
					App->fs->GetFileName(file, data.name);
				else
					data.name = name.data();

				uint shaderProgram = 0;
				bool success = App->shaderImporter->LoadShaderProgram(file, shaderProgramData, shaderProgram);

				std::list<ResourceShaderObject*> shaderObjects;
				for (uint i = 0; i < shaderObjectsNames.size(); ++i)
				{
					// Check if the resource exists
					std::string outputFile;
#ifndef GAMEMODE
					outputFile = DIR_ASSETS;
#else
					outputFile = DIR_LIBRARY;
#endif
					if (App->fs->RecursiveExists(shaderObjectsNames[i].data(), outputFile.data(), outputFile))
					{
						uint uuid = 0;
						std::vector<uint> uuids;
						if (GetResourcesUuidsByFile(outputFile.data(), uuids))
							uuid = uuids.front();
						else
							// If the shader object is not a resource yet, import it
							uuid = ImportFile(outputFile.data())->GetUuid();

						if (uuid > 0)
							shaderObjects.push_back((ResourceShaderObject*)GetResource(uuid));
					}
				}

				if (shaderObjectsNames.size() == shaderObjects.size())
				{
					shaderProgramData.shaderObjects = shaderObjects;

					if (!success)
					{
						// If the binary hasn't loaded correctly, link the shader program with the shader objects
						shaderProgram = ResourceShaderProgram::Link(shaderObjects);

						if (shaderProgram > 0)
							success = true;
					}
				}

				resource = CreateResource(ResourceTypes::ShaderProgramResource, data, &shaderProgramData, uuid);
				ResourceShaderProgram* shaderProgramResource = (ResourceShaderProgram*)resource;
				shaderProgramResource->isValid = success;
				if (success)
					shaderProgramResource->shaderProgram = shaderProgram;
			}
			else
				resource = GetResource(resourcesUuids.front());

			// 2. Meta
			// TODO: only create meta if any of its fields has been modificated
			std::string outputMetaFile;
			std::string name = resource->GetName();
			int64_t lastModTime = ResourceShaderProgram::CreateMeta(file, resourcesUuids.front(), name, shaderObjectsNames, outputMetaFile);
			assert(lastModTime > 0);
		}
	}
	break;

	case ResourceTypes::ScriptResource:
	{
		
		break;
	}
	}

	return resource;
}

Resource* ModuleResourceManager::ExportFile(ResourceTypes type, ResourceData& data, void* specificData, std::string& outputFile, bool overwrite)
{
	assert(type != ResourceTypes::NoResourceType);

	Resource* resource = nullptr;

	switch (type)
	{
	case ResourceTypes::ShaderObjectResource:
	{
		if (ResourceShaderObject::ExportFile(data, *(ResourceShaderObjectData*)specificData, outputFile, overwrite))
		{
			if (!overwrite)
				resource = ImportFile(outputFile.data());
		}
	}
	break;

	case ResourceTypes::ShaderProgramResource:
	{
		if (ResourceShaderProgram::ExportFile(data, *(ResourceShaderProgramData*)specificData, outputFile, overwrite))
		{
			// Meta
			std::string outputMetaFile;

			uint uuid = 0;
			std::vector<uint> resourcesUuids;
			if (GetResourcesUuidsByFile(outputFile.data(), resourcesUuids))
				uuid = resourcesUuids.front();
			ResourceShaderProgramData shaderProgramData = *(ResourceShaderProgramData*)specificData;
			std::list<std::string> shaderObjectsNames = shaderProgramData.GetShaderObjectsNames();
			std::vector<std::string> names;
			for (std::list<std::string>::const_iterator it = shaderObjectsNames.begin(); it != shaderObjectsNames.end(); ++it)
				names.push_back(*it);

			int64_t lastModTime = ResourceShaderProgram::CreateMeta(outputFile.data(), uuid == 0 ? App->GenerateRandomNumber() : uuid, data.name, names, outputMetaFile);
			assert(lastModTime > 0);

			if (!overwrite)
				resource = ImportFile(outputFile.data());
		}
	}
	break;
	}

	return resource;
}

Resource* ModuleResourceManager::CreateResource(ResourceTypes type, ResourceData& data, void* specificData, uint forcedUuid)
{
	assert(type != ResourceTypes::NoResourceType);

	Resource* resource = nullptr;
	uint uuid = forcedUuid == 0 ? App->GenerateRandomNumber() : forcedUuid;

	switch (type)
	{
		case ResourceTypes::MeshResource:
			resource = new ResourceMesh(ResourceTypes::MeshResource, uuid, data, *(ResourceMeshData*)specificData);
			break;
		case ResourceTypes::TextureResource:
			resource = new ResourceTexture(ResourceTypes::TextureResource, uuid, data, *(ResourceTextureData*)specificData);
			break;
		case ResourceTypes::ShaderObjectResource:
			resource = new ResourceShaderObject(ResourceTypes::ShaderObjectResource, uuid, data, *(ResourceShaderObjectData*)specificData);
			break;
		case ResourceTypes::ShaderProgramResource:
			resource = new ResourceShaderProgram(ResourceTypes::ShaderProgramResource, uuid, data, *(ResourceShaderProgramData*)specificData);
			break;
		case ResourceTypes::ScriptResource:
			resource = new ResourceScript(uuid, data, *(ResourceScriptData*)specificData);
			break;
	}
	assert(resource != nullptr);

	resources[uuid] = resource;

	return resource;
}

// ----------------------------------------------------------------------------------------------------

uint ModuleResourceManager::SetAsUsed(uint uuid) const
{
	std::unordered_map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it != resources.end())
		return it->second->IncreaseReferences();

	return 0;
}

uint ModuleResourceManager::SetAsUnused(uint uuid) const
{
	std::unordered_map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it != resources.end())
		return it->second->DecreaseReferences();

	return 0;
}

bool ModuleResourceManager::DeleteResource(uint uuid)
{
	std::unordered_map<uint, Resource*>::iterator it = resources.find(uuid);

	if (it == resources.end())
		return false;

	RELEASE(it->second);
	resources.erase(uuid);
	return true;
}

bool ModuleResourceManager::DeleteResources(std::vector<uint> uuids)
{
	for (std::vector<uint>::const_iterator it = uuids.begin(); it != uuids.end(); ++it)
	{
		std::unordered_map<uint, Resource*>::iterator resource = resources.find(*it);

		if (resource == resources.end())
			return false;

		RELEASE(resource->second);
		resources.erase(*it);
	}

	return true;
}

bool ModuleResourceManager::DeleteResources()
{
	bool ret = false;

	for (std::unordered_map<uint, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		assert(!it->second->IsInMemory());
		RELEASE(it->second);
		ret = true;
	}

	return ret;
}

void ModuleResourceManager::RecursiveDeleteUnusedEntries(const char* dir, std::string& path)
{
	assert(dir != nullptr);

	const char** files = App->fs->GetFilesFromDir(path.data());
	const char** it;

	path.append("/");

	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		if (App->fs->IsDirectory(path.data()))
			RecursiveDeleteUnusedEntries(*it, path);
		else
		{
			std::string extension;
			App->fs->GetExtension(*it, extension);
			ResourceTypes type = GetResourceTypeByExtension(extension.data());

			bool resources = false;

			uint resourceUuid = 0;
			if (!GetResourceUuidByExportedFile(path.data(), resourceUuid))
				App->fs->DeleteFileOrDir(path.data());
		}

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}
}

void ModuleResourceManager::RecursiveDeleteUnusedMetas(const char* dir, std::string& path)
{
	assert(dir != nullptr);

	const char** files = App->fs->GetFilesFromDir(path.data());
	const char** it;

	path.append("/");

	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		if (App->fs->IsDirectory(path.data()))
			RecursiveDeleteUnusedMetas(*it, path);
		else
		{
			std::string extension;
			App->fs->GetExtension(*it, extension);
			if (IS_META(extension.data()))
			{
				// Search for the file associated to the meta
				std::string file;
				App->fs->GetFileFromMeta(path.data(), file);

				std::vector<uint> resourcesUuids;
				if (!GetResourcesUuidsByFile(file.data(), resourcesUuids))
					App->fs->DeleteFileOrDir(path.data());
			}
		}

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}
}

// ----------------------------------------------------------------------------------------------------

Resource* ModuleResourceManager::GetResource(uint uuid) const
{
	std::unordered_map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it != resources.end())
		return it->second;

	return nullptr;
}

bool ModuleResourceManager::GetResourcesUuidsByFile(const char* file, std::vector<uint>& resourcesUuids) const
{
	for (std::unordered_map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetFile(), file) == 0)
			resourcesUuids.push_back(it->second->GetUuid());
	}

	return resourcesUuids.size() > 0;
}

bool ModuleResourceManager::GetResourceUuidByExportedFile(const char* file, uint& resourceUuid) const
{
	for (std::unordered_map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetExportedFile(), file) == 0)
		{
			resourceUuid = it->second->GetUuid();
			return true;
		}
	}

	return false;
}

ResourceTypes ModuleResourceManager::GetResourceTypeByExtension(const char* extension) const
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
	case ASCIIfbx: case ASCIIFBX:
	case ASCIIobj: case ASCIIOBJ:
	case ASCIIdae: case ASCIIDAE:
		return ResourceTypes::MeshResource;
		break;
	case ASCIIdds: case ASCIIDDS:
	case ASCIIpng: case ASCIIPNG:
	case ASCIIjpg: case ASCIIJPG:
	case ASCIItga: case ASCIITGA:
		return ResourceTypes::TextureResource;
		break;
	case ASCIIvsh: case ASCIIVSH:
	case ASCIIfsh: case ASCIIFSH:
		return ResourceTypes::ShaderObjectResource;
		break;
	case ASCIIpsh: case ASCIIPSH:
		return ResourceTypes::ShaderProgramResource;
		break;
	case ASCIIcs: case ASCIICS:
		return ResourceTypes::ScriptResource;
		break;
	}

	return ResourceTypes::NoResourceType;
}
