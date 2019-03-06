#include "ModuleResourceManager.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleInternalResHandler.h"
#include "ScriptingModule.h"

#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ShaderImporter.h"
#include "BoneImporter.h"
#include "AnimationImporter.h"
#include "ModuleAnimation.h"

#include "ResourceTypes.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"
#include "ResourceAnimation.h"
#include "ResourceBone.h"
#include "ResourceScript.h"
#include "ResourcePrefab.h"
#include "ResourceMaterial.h"
#include "ResourceScene.h"

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
	for (std::unordered_map<uint, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
		RELEASE(it->second);
	resources.clear();

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

	case System_Event_Type::ImportLibraryFile:
	{
		// 1. Import Library file
		ImportLibraryFile(event.fileEvent.file);
	}
	break;

	case System_Event_Type::FileOverwritten:
	case System_Event_Type::ReImportFile:
	{
		std::string extension;
		App->fs->GetExtension(event.fileEvent.file, extension);
		ResourceTypes type = GetResourceTypeByExtension(extension.data());

		switch (type)
		{
		case ResourceTypes::ScriptResource:
		{
			App->scripting->ScriptModified(event.fileEvent.file);
		}
		break;

		case ResourceTypes::PrefabResource:
		{
			char metaFile[DEFAULT_BUF_SIZE];
			sprintf(metaFile, "%s%s", event.fileEvent.file, EXTENSION_META);
			char* metaBuffer;
			uint size = App->fs->Load(metaFile, &metaBuffer);
			if (size <= 0)
				break;

			char* cursor = metaBuffer;
			cursor += sizeof(int64_t) + sizeof(uint);

			uint uid;
			memcpy(&uid, cursor, sizeof(uint));

			ResourcePrefab* prefab = (ResourcePrefab*)App->res->GetResource(uid);
			prefab->UpdateRoot();

			delete[] metaBuffer;
		}
		break;

		case ResourceTypes::MeshResource:
		case ResourceTypes::TextureResource:
		{
			std::vector<uint> resourcesUuids;
			if (GetResourcesUuidsByFile(event.fileEvent.file, resourcesUuids))
			{
				// 1. Delete entries
				for (uint i = 0; i < resourcesUuids.size(); ++i)
					App->fs->deleteFile(GetResource(resourcesUuids[i])->GetExportedFile());

				// 2. Delete resource(s)
				DeleteResources(resourcesUuids);
			}

			// 3. Import file
			System_Event newEvent;
			newEvent.type = System_Event_Type::ImportFile;
			strcpy_s(newEvent.fileEvent.file, DEFAULT_BUF_SIZE, event.fileEvent.file);
			App->PushSystemEvent(newEvent);
		}
		break;

		case ResourceTypes::ShaderProgramResource:
		{
			std::vector<Resource*> materials = GetResourcesByType(ResourceTypes::MaterialResource);
			for (uint i = 0; i < materials.size(); ++i)
			{
				ResourceMaterial* material = (ResourceMaterial*)materials[i];
				ResourceShaderProgram* shader = (ResourceShaderProgram*)GetResource(material->GetShaderUuid());
				if (strcmp(shader->GetFile(), event.fileEvent.file) == 0)
				{
					// Update the existing material
					material->SetResourceShader(shader->GetUuid());

					// Export the existing file
					std::string outputFile;
					App->res->ExportFile(ResourceTypes::MaterialResource, material->GetData(), &material->GetSpecificData(), outputFile, true, false);
				}
			}
		}
		break;
		}
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

		std::vector<uint> resourcesUuids;
		if (GetResourcesUuidsByFile(event.fileEvent.file, resourcesUuids))
		{
			// 2. Delete entries
			for (uint i = 0; i < resourcesUuids.size(); ++i)
				App->fs->deleteFile(GetResource(resourcesUuids[i])->GetExportedFile());
			
			// 3. Delete resource(s)
			DeleteResources(resourcesUuids);
		}
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

		std::vector<uint> resourcesUuids;
		if (GetResourcesUuidsByFile(event.fileEvent.file, resourcesUuids))
		{
			// 2. Delete entries
			for (uint i = 0; i < resourcesUuids.size(); ++i)
				App->fs->deleteFile(GetResource(resourcesUuids[i])->GetExportedFile());

			// 3. Delete resource(s)
			DeleteResources(resourcesUuids);
		}

		// 4. Import file	
		System_Event newEvent;
		newEvent.type = System_Event_Type::ImportFile;
		strcpy_s(newEvent.fileEvent.file, DEFAULT_BUF_SIZE, event.fileEvent.file);
		App->PushSystemEvent(newEvent);
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

	case System_Event_Type::ResourceDestroyed:
	{
		switch (event.resEvent.resource->GetType())
		{
		case ResourceTypes::TextureResource:
		{
			uint uuid = event.resEvent.resource->GetUuid();

			// Material resource uses Texture resource
			std::vector<Resource*> materials = GetResourcesByType(ResourceTypes::MaterialResource);
			for (uint i = 0; i < materials.size(); ++i)
			{
				ResourceMaterial* material = (ResourceMaterial*)materials[i];

				std::vector<Uniform>& uniforms = material->GetUniforms();
				for (uint i = 0; i < uniforms.size(); ++i)
				{
					switch (uniforms[i].common.type)
					{
					case Uniforms_Values::Sampler2U_value:
					{
						if (uniforms[i].sampler2DU.value.uuid == uuid)
							material->SetResourceTexture(App->resHandler->defaultTexture, uniforms[i].sampler2DU.value.uuid, uniforms[i].sampler2DU.value.id);
					}
					break;
					}
				}
			}
		}
		break;

		case ResourceTypes::ShaderObjectResource:
		{
			uint uuid = event.resEvent.resource->GetUuid();

			// Shader Program resource uses Shader Object resource
			std::vector<Resource*> shaderPrograms = GetResourcesByType(ResourceTypes::ShaderProgramResource);
			for (uint i = 0; i < shaderPrograms.size(); ++i)
			{
				ResourceShaderProgram* shaderProgram = (ResourceShaderProgram*)shaderPrograms[i];

				std::vector<uint> shaderObjects;
				shaderProgram->GetShaderObjects(shaderObjects);
				for (uint i = 0; i < shaderObjects.size(); ++i)
				{
					if (shaderObjects[i] == uuid)
						shaderObjects[i] = 0;
				}
				shaderProgram->SetShaderObjects(shaderObjects);
			}
		}
		break;

		case ResourceTypes::ShaderProgramResource:
		{
			uint uuid = event.resEvent.resource->GetUuid();

			// Material resource uses Shader Program resource
			std::vector<Resource*> materials = GetResourcesByType(ResourceTypes::MaterialResource);
			for (uint i = 0; i < materials.size(); ++i)
			{
				ResourceMaterial* material = (ResourceMaterial*)materials[i];
				if (material->materialData.shaderUuid == uuid)
					material->SetResourceShader(App->resHandler->defaultShaderProgram);
			}
		}
		break;
		}
	}
	break;

	// Prefabs events
	case System_Event_Type::ScriptingDomainReloaded:
	case System_Event_Type::Stop:
	case System_Event_Type::LoadScene:
	{
		for (auto res = resources.begin(); res != resources.end(); ++res)
		{
			if (res->second->GetType() == ResourceTypes::PrefabResource)
			{
				ResourcePrefab* prefab = (ResourcePrefab*)res->second;
				prefab->UpdateRoot();
			}
		}
		break;
	}

	case System_Event_Type::GenerateLibraryFiles:
	{
		for (std::unordered_map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			if (!it->second->GetData().internal)
				it->second->GenerateLibraryFiles();
		}
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
		std::vector<std::string> mesh_files;
		std::vector<std::string> bone_files;
		std::vector<std::string> animation_files;
		if (ResourceMesh::ImportFile(file, meshImportSettings, mesh_files, bone_files,animation_files))
		{
			std::vector<uint> resourcesUuids;
			std::vector<uint> meshes_uuids;
			std::vector<uint> bones_uuids;
			std::vector<uint> animation_uuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The Mesh file '%s' has resources that need to be created", file);

				// 1. Meshes
				meshes_uuids.reserve(mesh_files.size());
				for (uint i = 0; i < mesh_files.size(); ++i)
				{
					std::string fileName;
					App->fs->GetFileName(mesh_files[i].data(), fileName);
					uint uuid = strtoul(fileName.data(), NULL, 0);
					assert(uuid > 0);

					ResourceData data;
					ResourceMeshData meshData;
					data.file = file;
					data.exportedFile = mesh_files[i].data();
					App->fs->GetFileName(file, data.name);
					meshData.meshImportSettings = meshImportSettings;
					strcpy((char*)meshData.meshImportSettings.modelPath, file);
					App->sceneImporter->Load(mesh_files[i].data(), data, meshData);

					resource = CreateResource(ResourceTypes::MeshResource, data, &meshData, uuid);
					if (resource != nullptr)
						meshes_uuids.push_back(uuid);
				}
				meshes_uuids.shrink_to_fit();

				// 2. Bones c:
				bones_uuids.reserve(bone_files.size());
				for (uint i = 0; i < bone_files.size(); ++i)
				{
					std::string fileName;
					App->fs->GetFileName(bone_files[i].data(), fileName);
					uint uuid = strtoul(fileName.data(), NULL, 0);
					assert(uuid > 0);

					ResourceData data;
					ResourceBoneData bone_data;
					data.file = file;
					data.exportedFile = bone_files[i].data();
					App->fs->GetFileName(file, data.name);
					App->boneImporter->Load(bone_files[i].data(), data, bone_data);

					resource = CreateResource(ResourceTypes::BoneResource, data, &bone_data, uuid);
					if (resource != nullptr)
						bones_uuids.push_back(uuid);
				}
				bones_uuids.shrink_to_fit();

				// 3. Anims c:
				animation_uuids.reserve(animation_files.size());
				for (uint i = 0; i < animation_files.size(); ++i)
				{
					std::string fileName;
					App->fs->GetFileName(animation_files[i].data(), fileName);
					uint uuid = strtoul(fileName.data(), NULL, 0);
					assert(uuid > 0);

					ResourceData data;
					ResourceAnimationData anim_data;
					data.file = file;
					data.exportedFile = animation_files[i].data();
					App->fs->GetFileName(file, data.name);
					App->animImporter->Load(animation_files[i].data(), data, anim_data);

					resource = CreateResource(ResourceTypes::AnimationResource, data, &anim_data, uuid);

					App->animation->SetAnimationGos((ResourceAnimation*)resource);

					if (resource != nullptr)
						animation_uuids.push_back(uuid);
				}
				animation_uuids.shrink_to_fit();
			}
			
			// TODO_G : separate mesh / bones resources uuids from resourcesUuids

			for (uint i = 0u; i < resourcesUuids.size(); i++)
			{
				Resource* tmp_res = App->res->GetResource(resourcesUuids[i]);
				if (tmp_res->GetType() == ResourceTypes::MeshResource)
					meshes_uuids.push_back(resourcesUuids[i]);
				else if (tmp_res->GetType() == ResourceTypes::BoneResource)
					bones_uuids.push_back(resourcesUuids[i]);
				else if (tmp_res->GetType() == ResourceTypes::AnimationResource)
					animation_uuids.push_back(resourcesUuids[i]);
			}

			// bone mesh etc todo

			// 2. Meta
			// TODO: only create meta if any of its fields has been modificated
			std::string outputMetaFile;
			int64_t lastModTime = ResourceMesh::CreateMeta(file, meshImportSettings, meshes_uuids, bones_uuids, animation_uuids, outputMetaFile);
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
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The Texture file '%s' has resources that need to be created", file);

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
				App->materialImporter->Load(outputFile.data(), data, textureData);

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
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The Shader Object file '%s' has resources that need to be created", file);

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
					shaderObjectData.shaderObjectType = ShaderObjectTypes::VertexType;
				else if (IS_FRAGMENT_SHADER(extension.data()))
					shaderObjectData.shaderObjectType = ShaderObjectTypes::FragmentType;
				else if (IS_GEOMETRY_SHADER(extension.data()))
					shaderObjectData.shaderObjectType = ShaderObjectTypes::GeometryType;

				uint shaderObject = 0;
				bool success = ResourceShaderObject::LoadFile(file, shaderObjectData, shaderObject);

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
		std::vector<uint> shaderObjectsUuids;
		ShaderProgramTypes shaderProgramType = ShaderProgramTypes::Custom;
		uint format = 0;
		if (ResourceShaderProgram::ImportFile(file, name, shaderObjectsUuids, shaderProgramType, format, outputFile))
		{
			std::vector<uint> resourcesUuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The Shader Program file '%s' has resources that need to be created", file);

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
				shaderProgramData.format = format;

				uint shaderProgram = 0;
				bool success = ResourceShaderProgram::LoadFile(file, shaderProgramData, shaderProgram);

				uint total = 0;
				for (uint i = 0; i < shaderObjectsUuids.size(); ++i)
				{
					// Check if the resource exists
					if (GetResource(shaderObjectsUuids[i]) != nullptr)
						++total;
				}

				if (total == shaderObjectsUuids.size())
				{
					shaderProgramData.shaderObjectsUuids = shaderObjectsUuids;

					if (!success)
					{
						// If the binary hasn't loaded correctly, link the shader program with the shader objects
						shaderProgram = ResourceShaderProgram::Link(shaderObjectsUuids);

						if (shaderProgram > 0)
							success = true;
					}
				}

				shaderProgramData.shaderProgramType = shaderProgramType;

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
			int64_t lastModTime = ResourceShaderProgram::CreateMeta(file, resourcesUuids.front(), name, shaderObjectsUuids, shaderProgramType, format, outputMetaFile);
			assert(lastModTime > 0);
		}
	}
	break;

	case ResourceTypes::MaterialResource:
	{
		std::string outputFile;
		std::string name;
		if (ResourceMaterial::ImportFile(file, name, outputFile))
		{
			std::vector<uint> resourcesUuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The Material file '%s' has resources that need to be created", file);

				// 1. Material
				uint uuid = outputFile.empty() ? App->GenerateRandomNumber() : strtoul(outputFile.data(), NULL, 0);
				assert(uuid > 0);
				resourcesUuids.push_back(uuid);
				resourcesUuids.shrink_to_fit();

				ResourceData data;
				ResourceMaterialData materialData;
				data.file = file;
				if (name.empty())
					App->fs->GetFileName(file, data.name);
				else
					data.name = name.data();
				ResourceMaterial::LoadFile(file, materialData);

				resource = CreateResource(ResourceTypes::MaterialResource, data, &materialData, uuid);
			}
			else
				resource = GetResource(resourcesUuids.front());

			// 2. Meta
			// TODO: only create meta if any of its fields has been modificated
			std::string outputMetaFile;
			std::string name = resource->GetName();
			int64_t lastModTime = ResourceMaterial::CreateMeta(file, resourcesUuids.front(), name, outputMetaFile);
			assert(lastModTime > 0);
		}
	}
	break;

	case ResourceTypes::ScriptResource:
	{
		resource = App->scripting->ImportScriptResource(file);
		break;
	}

	case ResourceTypes::PrefabResource:
	{
		resource = ResourcePrefab::ImportFile(file);
		resources[resource->GetUuid()] = resource;
		break;
	}

	case ResourceTypes::SceneResource:
	{
		resource = ResourceScene::ImportFile(file);
		resources[resource->GetUuid()] = resource;
		break;
	}

	case ResourceTypes::BoneResource:
	{
		std::string outputFile;
		std::string name;
		if (ResourceBone::ImportFile(file, name, outputFile)) {
			std::vector<uint> resourcesUuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The bone object file '%s' has resources that need to be created", file);

				// 1. Shader object
				uint uuid = outputFile.empty() ? App->GenerateRandomNumber() : strtoul(outputFile.data(), NULL, 0);
				assert(uuid > 0);
				resourcesUuids.push_back(uuid);
				resourcesUuids.shrink_to_fit();

				ResourceData data;
				ResourceBoneData boneData;
				data.file = file;
				if (name.empty())
					App->fs->GetFileName(file, data.name);
				else
					data.name = name.data();


				uint shaderObject = 0;
				bool success = ResourceBone::LoadFile(file, boneData);

				resource = CreateResource(ResourceTypes::BoneResource, data, &boneData, uuid);

			}
			else
				resource = GetResource(resourcesUuids.front());

			std::string outputMetaFile;
			std::string name = resource->GetName();
			int64_t lastModTime = ResourceShaderObject::CreateMeta(file, resourcesUuids.front(), name, outputMetaFile);
			assert(lastModTime > 0);
		}
		break;
	}

	case ResourceTypes::AnimationResource:
	{
		std::string outputFile;
		std::string name;
		if (ResourceAnimation::ImportFile(file, name, outputFile))
		{
			std::vector<uint> resourcesUuids;
			if (!GetResourcesUuidsByFile(file, resourcesUuids))
			{
				// Create the resources
				CONSOLE_LOG(LogTypes::Normal, "RESOURCE MANAGER: The AnimationResource file '%s' has resources that need to be created", file);
			
				// UUID
				uint uuid = outputFile.empty() ? App->GenerateRandomNumber() : strtoul(outputFile.data(), NULL, 0);
				assert(uuid > 0);
				resourcesUuids.push_back(uuid);
				resourcesUuids.shrink_to_fit();


				ResourceData data;
				ResourceAnimationData animationData;
				data.file = file;
				if (name.empty())
					App->fs->GetFileName(file, data.name);
				else
					data.name = name.data();


				uint shaderObject = 0;
				bool success = ResourceAnimation::LoadFile(file, animationData);

				resource = CreateResource(ResourceTypes::AnimationResource, data, &animationData, uuid);
			}
			else
				resource = GetResource(resourcesUuids.front());

			// 2. Meta
			// TODO: only create meta if any of its fields has been modificated
			std::string outputMetaFile;
			std::string name = resource->GetName();
			int64_t lastModTime = ResourceAnimation::CreateMeta(file, resourcesUuids.front(), name, outputMetaFile);
			assert(lastModTime > 0);
		}
	}
	break;

	}

	return resource;
}

Resource* ModuleResourceManager::ImportLibraryFile(const char* file)
{
	assert(file != nullptr);

	Resource* resource = nullptr;

	std::string extension;
	App->fs->GetExtension(file, extension);
	ResourceTypes type = GetLibraryResourceTypeByExtension(extension.data());

	switch (type)
	{
	case ResourceTypes::MeshResource:
	{
		std::string fileName;
		App->fs->GetFileName(file, fileName);
		uint uuid = strtoul(fileName.data(), NULL, 0);
		assert(uuid > 0);

		ResourceData data;
		ResourceMeshData meshData;
		data.exportedFile = file;

		App->sceneImporter->Load(file, data, meshData);

		resource = CreateResource(ResourceTypes::MeshResource, data, &meshData, uuid);
	}
	break;

	case ResourceTypes::TextureResource:
	{
		std::string fileName;
		App->fs->GetFileName(file, fileName);
		uint uuid = strtoul(fileName.data(), NULL, 0);
		assert(uuid > 0);

		ResourceData data;
		ResourceTextureData textureData;
		data.exportedFile = file;

		App->materialImporter->Load(file, data, textureData);

		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(file) + 1, file); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		if (App->fs->Exists(metaFile))
		{
			int64_t lastModTime = 0;
			uint textureUuid = 0;
			ResourceTexture::ReadMeta(metaFile, lastModTime, textureData.textureImportSettings, textureUuid);
		}

		resource = CreateResource(ResourceTypes::TextureResource, data, &textureData, uuid);
	}
	break;

	case ResourceTypes::ShaderObjectResource:
	{
		ResourceData data;
		ResourceShaderObjectData shaderObjectData;
		data.exportedFile = file;

		if (IS_VERTEX_SHADER(extension.data()))
			shaderObjectData.shaderObjectType = ShaderObjectTypes::VertexType;
		else if (IS_FRAGMENT_SHADER(extension.data()))
			shaderObjectData.shaderObjectType = ShaderObjectTypes::FragmentType;

		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(file) + 1, file); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		uint uuid = 0;
		if (App->fs->Exists(metaFile))
		{
			int64_t lastModTime = 0;		
			ResourceShaderObject::ReadMeta(metaFile, lastModTime, uuid, data.name);
		}

		uint shaderObject = 0;
		bool success = ResourceShaderObject::LoadFile(file, shaderObjectData, shaderObject);

		resource = CreateResource(ResourceTypes::ShaderObjectResource, data, &shaderObjectData, uuid);
		ResourceShaderObject* shaderObjectResource = (ResourceShaderObject*)resource;
		shaderObjectResource->isValid = success;
		if (success)
			shaderObjectResource->shaderObject = shaderObject;
	}
	break;

	case ResourceTypes::ShaderProgramResource:
	{
		ResourceData data;
		ResourceShaderProgramData shaderProgramData;
		data.exportedFile = file;

		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(file) + 1, file); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		uint uuid = 0;
		std::vector<uint> shaderObjectsUuids;
		if (App->fs->Exists(metaFile))
		{
			int64_t lastModTime = 0;
			ResourceShaderProgram::ReadMeta(metaFile, lastModTime, uuid, data.name, shaderObjectsUuids, shaderProgramData.shaderProgramType, shaderProgramData.format);
		}

		uint shaderProgram = 0;
		bool success = ResourceShaderProgram::LoadFile(file, shaderProgramData, shaderProgram);

		uint total = 0;
		for (uint i = 0; i < shaderObjectsUuids.size(); ++i)
		{
			// Check if the resource exists
			if (GetResource(shaderObjectsUuids[i]) != nullptr)
				++total;
		}

		if (total == shaderObjectsUuids.size())
		{
			shaderProgramData.shaderObjectsUuids = shaderObjectsUuids;

			if (!success)
			{
				// If the binary hasn't loaded correctly, link the shader program with the shader objects
				shaderProgram = ResourceShaderProgram::Link(shaderObjectsUuids);

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
	break;

	case ResourceTypes::MaterialResource:
	{
		ResourceData data;
		ResourceMaterialData materialData;
		data.exportedFile = file;

		// Search for the meta associated to the file
		char metaFile[DEFAULT_BUF_SIZE];
		strcpy_s(metaFile, strlen(file) + 1, file); // file
		strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

		uint uuid = 0;
		std::vector<std::string> shaderObjectsNames;
		if (App->fs->Exists(metaFile))
		{
			int64_t lastModTime = 0;
			ResourceMaterial::ReadMeta(metaFile, lastModTime, uuid, data.name);
		}

		ResourceMaterial::LoadFile(file, materialData);

		resource = CreateResource(ResourceTypes::MaterialResource, data, &materialData, uuid);
	}
	break;

	case ResourceTypes::ScriptResource:
	{
		resource = App->scripting->ImportScriptResource(file);
	}
	break;

	case ResourceTypes::PrefabResource:
	{
		resource = ResourcePrefab::ImportFile(file);
		resources[resource->GetUuid()] = resource;
	}
	break;

	case ResourceTypes::SceneResource:
	{
		resource = ResourceScene::ImportFile(file);
		resources[resource->GetUuid()] = resource;
	}
	break;

	case ResourceTypes::BoneResource:
	{
		std::string fileName;
		App->fs->GetFileName(file, fileName);
		uint uuid = strtoul(fileName.data(), NULL, 0);
		assert(uuid > 0);

		ResourceData data;
		ResourceBoneData boneData;
		data.file = file;

		App->boneImporter->Load(file, data, boneData);
		//ResourceBone::LoadFile(file, boneData);

		resource = CreateResource(ResourceTypes::BoneResource, data, &boneData, uuid);
	}
	break;

	case ResourceTypes::AnimationResource:
	{
		std::string fileName;
		App->fs->GetFileName(file, fileName);
		uint uuid = strtoul(fileName.data(), NULL, 0);
		assert(uuid > 0);

		ResourceData data;
		ResourceAnimationData animationData;
		data.file = file;

		ResourceAnimation::LoadFile(file, animationData);

		resource = CreateResource(ResourceTypes::AnimationResource, data, &animationData, uuid);
		App->animation->SetAnimationGos((ResourceAnimation*)resource);
	}
	break;
	}

	return resource;
}

Resource* ModuleResourceManager::ExportFile(ResourceTypes type, ResourceData& data, void* specificData, std::string& outputFile, bool overwrite, bool resources)
{
	assert(type != ResourceTypes::NoResourceType);

	Resource* resource = nullptr;

	switch (type)
	{
	case ResourceTypes::ShaderObjectResource:
	{
		if (ResourceShaderObject::ExportFile(data, *(ResourceShaderObjectData*)specificData, outputFile, overwrite))
		{
			if (resources)
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

			int64_t lastModTime = ResourceShaderProgram::CreateMeta(outputFile.data(), uuid == 0 ? App->GenerateRandomNumber() : uuid, data.name, shaderProgramData.shaderObjectsUuids, shaderProgramData.shaderProgramType, shaderProgramData.format, outputMetaFile);
			assert(lastModTime > 0);

			if (resources)
				resource = ImportFile(outputFile.data());
		}
	}
	break;

	case ResourceTypes::MaterialResource:
	{
		if (ResourceMaterial::ExportFile(data, *(ResourceMaterialData*)specificData, outputFile, overwrite))
		{
			if (resources)
				resource = ImportFile(outputFile.data());
		}
	}
	break;

	case ResourceTypes::PrefabResource:
	{
		resource = (Resource*)ResourcePrefab::ExportFile(data.name.data(), (*(PrefabData*)specificData).root);
		this->resources[resource->GetUuid()] = resource;
	}
	break;

	// Add new resource
	case ResourceTypes::BoneResource:
	{
		if (ResourceBone::ExportFile(data, *(ResourceBoneData*)specificData, outputFile, overwrite))
		{
			if (!overwrite)
				resource = ImportFile(outputFile.data());
		}
	}
	break;
	case ResourceTypes::AnimationResource:
	{
		if (ResourceAnimation::ExportFile(data, *(ResourceAnimationData*)specificData, outputFile, overwrite))
		{
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
		case ResourceTypes::MaterialResource:
			resource = new ResourceMaterial(ResourceTypes::MaterialResource, uuid, data, *(ResourceMaterialData*)specificData);
			break;
		case ResourceTypes::ScriptResource:
			resource = new ResourceScript(uuid, data, *(ResourceScriptData*)specificData);
			break;
		case ResourceTypes::PrefabResource:
			resource = new ResourcePrefab(uuid, data, *(PrefabData*)specificData);
			break;
		case ResourceTypes::BoneResource:
			resource = new ResourceBone(ResourceTypes::BoneResource, uuid, data, *(ResourceBoneData*)specificData);
			break;
		case ResourceTypes::AnimationResource:
			resource = new ResourceAnimation(ResourceTypes::AnimationResource, uuid, data, *(ResourceAnimationData*)specificData);
			break;
		case ResourceTypes::SceneResource:
			resource = new ResourceScene(uuid, data, *(SceneData*)specificData);
	}

	assert(resource != nullptr);

	resources[uuid] = resource;

	return resource;
}

// ----------------------------------------------------------------------------------------------------

uint ModuleResourceManager::SetAsUsed(uint uuid) const
{
	std::unordered_map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it != resources.end() && it->second != nullptr)
		return it->second->IncreaseReferences();

	return 0;
}

uint ModuleResourceManager::SetAsUnused(uint uuid) const
{
	std::unordered_map<uint, Resource*>::const_iterator it = resources.find(uuid);

	if (it != resources.end() && it->second != nullptr)
		return it->second->DecreaseReferences();

	return 0;
}

bool ModuleResourceManager::DeleteResource(uint uuid)
{
	std::unordered_map<uint, Resource*>::iterator it = resources.find(uuid);

	if (it == resources.end())
		return false;

	System_Event newEvent;
	newEvent.type = System_Event_Type::ResourceDestroyed;
	newEvent.resEvent.resource = it->second;
	App->PushSystemEvent(newEvent);

	return true;
}

bool ModuleResourceManager::DeleteResources(std::vector<uint> uuids)
{
	for (std::vector<uint>::const_iterator it = uuids.begin(); it != uuids.end(); ++it)
	{
		std::unordered_map<uint, Resource*>::iterator resource = resources.find(*it);

		if (resource == resources.end())
			return false;

		System_Event newEvent;
		newEvent.type = System_Event_Type::ResourceDestroyed;
		newEvent.resEvent.resource = resource->second;
		App->PushSystemEvent(newEvent);
	}

	return true;
}

bool ModuleResourceManager::DeleteResources()
{
	bool ret = false;

	for (std::unordered_map<uint, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		System_Event newEvent;
		newEvent.type = System_Event_Type::ResourceDestroyed;
		newEvent.resEvent.resource = it->second;
		App->PushSystemEvent(newEvent);

		ret = true;
	}

	return ret;
}

bool ModuleResourceManager::EraseResource(Resource* toErase)
{
	assert(toErase != nullptr);
	bool ret = false;

	std::unordered_map<uint, Resource*>::iterator it = resources.find(toErase->GetUuid());
	ret = it != resources.end();

	if (ret)
		resources.erase(it);

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
			if (strcmp(extension.data(), EXTENSION_SCRIPT) == 0)
				continue;
			ResourceTypes type = GetResourceTypeByExtension(extension.data());

			//TODO: INSPECT WHY THIS ERROR HAPPENS
			if (type == ResourceTypes::NoResourceType)
				continue;

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
	case ASCIIgsh: case ASCIIGSH:
		return ResourceTypes::ShaderObjectResource;
		break;
	case ASCIIpsh: case ASCIIPSH:
		return ResourceTypes::ShaderProgramResource;
		break;
	case ASCIImat: case ASCIIMAT:
		return ResourceTypes::MaterialResource;
		break;
	case ASCIIcs: case ASCIICS:
		return ResourceTypes::ScriptResource;
		break;
	case ASCIIpfb: case ASCIIPFB:
		return ResourceTypes::PrefabResource;
		break;
	case ASCIISCN: case ASCIIscn:
		return ResourceTypes::SceneResource;
	}
	
	return ResourceTypes::NoResourceType;
}

ResourceTypes ModuleResourceManager::GetLibraryResourceTypeByExtension(const char* extension) const
{
	if (strcmp(extension, EXTENSION_MESH) == 0)
		return ResourceTypes::MeshResource;
	else if (strcmp(extension, EXTENSION_TEXTURE) == 0)
		return ResourceTypes::TextureResource;
	else if (strcmp(extension, EXTENSION_VERTEX_SHADER_OBJECT) == 0
		|| strcmp(extension, EXTENSION_FRAGMENT_SHADER_OBJECT) == 0
		|| strcmp(extension, EXTENSION_GEOMETRY_SHADER_OBJECT) == 0)
		return ResourceTypes::ShaderObjectResource;
	else if (strcmp(extension, EXTENSION_SHADER_PROGRAM) == 0)
		return ResourceTypes::ShaderProgramResource;
	else if (strcmp(extension, EXTENSION_MATERIAL) == 0)
		return ResourceTypes::MaterialResource;
	else if (strcmp(extension, EXTENSION_ANIMATION) == 0)
		return ResourceTypes::AnimationResource;
	else if (strcmp(extension, EXTENSION_BONE) == 0)
		return ResourceTypes::BoneResource;
	else if (strcmp(extension, EXTENSION_SCRIPT) == 0)
		return ResourceTypes::ScriptResource;
	else if (strcmp(extension, EXTENSION_PREFAB) == 0)
		return ResourceTypes::PrefabResource;
	else if (strcmp(extension, EXTENSION_SCENE) == 0)
		return ResourceTypes::SceneResource;

	return ResourceTypes::NoResourceType;
}

std::vector<Resource*> ModuleResourceManager::GetResourcesByType(ResourceTypes type)
{
	std::vector<Resource*> ret;
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == type)
		{
			ret.push_back(it->second);
		}
	}
	return ret;
}
