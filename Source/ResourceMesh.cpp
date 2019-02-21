#include "ResourceMesh.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"

#include "SceneImporter.h"

#include "imgui\imgui.h"

#include <assert.h>

ResourceMesh::ResourceMesh(ResourceTypes type, uint uuid, ResourceData data, ResourceMeshData meshData) : Resource(type, uuid, data), meshData(meshData) {}

ResourceMesh::~ResourceMesh()
{
	RELEASE_ARRAY(meshData.vertices);
	RELEASE_ARRAY(meshData.indices);
}

void ResourceMesh::OnPanelAssets()
{
	ImGuiTreeNodeFlags flags = 0;
	flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

	if (App->scene->selectedObject == this)
		flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

	/*if (App->scene->selectedObject == ((MeshImportSettings*)child->importSettings))
		treeNodeFlags |= ImGuiTreeNodeFlags_Selected;*/

		//Create the leaf

	char id[DEFAULT_BUF_SIZE];
	sprintf(id, "%s##%d", data.name.data(), uuid);

	if (ImGui::TreeNodeEx(id, flags))
		ImGui::TreePop();

	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered() /*&& (mouseDelta.x == 0 && mouseDelta.y == 0)*/)
	{
		SELECT(this);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("MESH_INSPECTOR_SELECTOR", &uuid, sizeof(uint));
		ImGui::EndDragDropSource();
	}
}

// ----------------------------------------------------------------------------------------------------

bool ResourceMesh::ImportFile(const char* file, ResourceMeshImportSettings& meshImportSettings, std::vector<std::string>& outputFiles) // NewFile
{
	assert(file != nullptr);

	bool imported = false;

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// CASE 1 (file). The file has no meta associated (the file is new)
	if (!App->fs->Exists(metaFile))
	{
		// Import the file (using the default import settings)
		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: The file '%s' needs to be imported", file);

		imported = App->sceneImporter->Import(file, outputFiles, meshImportSettings);
	}
	else
	{
		std::vector<uint> uuids;
		std::vector<std::string> entryFiles;
		assert(ResourceMesh::ReadMeshesUuidsFromMeta(metaFile, uuids));

		char entry[DEFAULT_BUF_SIZE];
		std::string entryFile;
		for (uint i = 0; i < uuids.size(); ++i)
		{
			sprintf_s(entry, "%u%s", uuids[i], EXTENSION_MESH);
			entryFile = DIR_LIBRARY;
			if (App->fs->RecursiveExists(entry, DIR_LIBRARY, entryFile))
				entryFiles.push_back(entryFile);
			entryFile.clear();
		}

		// CASE 2 (file + meta + Library file(s)). The resource(s) do(es)n't exist
		if (entryFiles.size() == uuids.size())
		{
			outputFiles = entryFiles;
			imported = true;
		}			
		// CASE 3 (file + meta). The file(s) in Libray associated to the meta do(es)n't exist
		else
		{
			// Import the file (using the import settings from the meta)
			CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: The file '%s' has Library file(s) that need(s) to be reimported", file);

			ResourceMesh::ReadMeshImportSettingsFromMeta(metaFile, meshImportSettings);
			imported = App->sceneImporter->Import(file, outputFiles, meshImportSettings, uuids);
		}
	}

	return imported;
}

// Returns the last modification time of the file
uint ResourceMesh::CreateMeta(const char* file, ResourceMeshImportSettings& meshImportSettings, std::vector<uint>& meshesUuids, std::string& outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = meshesUuids.size();

	uint size = 
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(int) +
		sizeof(uint) +
		sizeof(uint);

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	int64_t lastModTime = App->fs->GetLastModificationTime(file);
	assert(lastModTime > 0);
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store meshes uuids
	bytes = sizeof(uint) * meshesUuids.size();
	memcpy(cursor, &meshesUuids[0], bytes);

	cursor += bytes;

	// 4. Store import settings
	bytes = sizeof(int);
	memcpy(cursor, &meshImportSettings.postProcessConfigurationFlags, bytes);

	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &meshImportSettings.customConfigurationFlags, bytes);

	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &meshImportSettings.size, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourceMesh::ReadMeta(const char* metaFile, int64_t& lastModTime, ResourceMeshImportSettings& meshImportSettings, std::vector<uint>& meshesUuids)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{		
		char* cursor = (char*)buffer;

		// 1. Load last modification time
		uint bytes = sizeof(int64_t);
		memcpy(&lastModTime, cursor, bytes);

		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load meshes uuids
		meshesUuids.resize(uuidsSize);
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&meshesUuids[0], cursor, bytes);

		cursor += bytes;

		// 4. Load import settings
		bytes = sizeof(int);
		memcpy(&meshImportSettings.postProcessConfigurationFlags, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(uint);
		memcpy(&meshImportSettings.customConfigurationFlags, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(uint);
		memcpy(&meshImportSettings.size, cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

// Returns true if the meshes uuids vector is not empty. Else, returns false
bool ResourceMesh::ReadMeshesUuidsFromBuffer(const char* buffer, std::vector<uint>& meshesUuids)
{
	char* cursor = (char*)buffer;

	// 1. (Last modification time)
	uint bytes = sizeof(int64_t);
	cursor += bytes;

	// 2. Load uuids size
	uint uuidsSize = 0;
	bytes = sizeof(uint);
	memcpy(&uuidsSize, cursor, bytes);
	assert(uuidsSize > 0);

	cursor += bytes;

	// 3. Load meshes uuids
	meshesUuids.resize(uuidsSize);
	bytes = sizeof(uint) * uuidsSize;
	memcpy(&meshesUuids[0], cursor, bytes);

	if (meshesUuids.size() > 0)
		return true;
	else
		return false;
}

uint ResourceMesh::SetMeshImportSettingsToMeta(const char* metaFile, const ResourceMeshImportSettings& meshImportSettings)
{
	assert(metaFile != nullptr);

	int64_t lastModTime = 0;
	ResourceMeshImportSettings oldMeshImportSettings;
	std::vector<uint> meshesUuids;
	ReadMeta(metaFile, lastModTime, oldMeshImportSettings, meshesUuids);

	uint uuidsSize = meshesUuids.size();

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(int) +
		sizeof(uint) +
		sizeof(uint);

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 2. Store uuids size
	bytes = sizeof(uint);
	memcpy(cursor, &uuidsSize, bytes);

	cursor += bytes;

	// 3. Store meshes uuids
	bytes = sizeof(uint) * meshesUuids.size();
	memcpy(cursor, &meshesUuids[0], bytes);

	cursor += bytes;

	// 4. Store import settings
	bytes = sizeof(int);
	memcpy(cursor, &meshImportSettings.postProcessConfigurationFlags, bytes);

	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &meshImportSettings.customConfigurationFlags, bytes);

	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &meshImportSettings.size, bytes);

	// --------------------------------------------------

	// Save the meta file
	uint resultSize = App->fs->Save(metaFile, data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully saved meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not save meta '%s'", metaFile);
		return 0;
	}

	return lastModTime;
}

// ----------------------------------------------------------------------------------------------------

void ResourceMesh::GetVerticesReference(Vertex*& vertices) const
{
	vertices = meshData.vertices;
}

void ResourceMesh::GetTris(float* verticesPosition) const
{
	for (int i = 0, j = -1; i < meshData.verticesSize; ++i)
	{
		verticesPosition[++j] = meshData.vertices[i].position[0];
		verticesPosition[++j] = meshData.vertices[i].position[1];
		verticesPosition[++j] = meshData.vertices[i].position[2];
	}
}

void ResourceMesh::GetIndicesReference(uint*& indices) const
{
	indices = meshData.indices;
}

void ResourceMesh::GetIndices(uint* indices) const
{
	memcpy(indices, meshData.indices, meshData.indicesSize * sizeof(uint));
}

uint ResourceMesh::GetVerticesCount() const
{
	return meshData.verticesSize;
}

uint ResourceMesh::GetIndicesCount() const
{
	return meshData.indicesSize;
}

uint ResourceMesh::GetVBO() const
{
	return VBO;
}

uint ResourceMesh::GetIBO() const
{
	return IBO;
}

uint ResourceMesh::GetVAO() const
{
	return VAO;
}

// ----------------------------------------------------------------------------------------------------

// Returns true if the meshes uuids vector is not empty. Else, returns false
bool ResourceMesh::ReadMeshesUuidsFromMeta(const char* metaFile, std::vector<uint>& meshesUuids)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. (Last modification time)
		uint bytes = sizeof(int64_t);
		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. Load meshes uuids
		meshesUuids.resize(uuidsSize);
		bytes = sizeof(uint) * uuidsSize;
		memcpy(&meshesUuids[0], cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not load meta '%s'", metaFile);
		return false;
	}

	if (meshesUuids.size() > 0)
		return true;
	else
		return false;
}

bool ResourceMesh::ReadMeshImportSettingsFromMeta(const char* metaFile, ResourceMeshImportSettings& meshImportSettings)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. (Last modification time)
		uint bytes = sizeof(int64_t);
		cursor += bytes;

		// 2. Load uuids size
		uint uuidsSize = 0;
		bytes = sizeof(uint);
		memcpy(&uuidsSize, cursor, bytes);
		assert(uuidsSize > 0);

		cursor += bytes;

		// 3. (Meshes uuids)
		bytes = sizeof(uint) * uuidsSize;
		cursor += bytes;

		// 4. Load import settings
		bytes = sizeof(int);
		memcpy(&meshImportSettings.postProcessConfigurationFlags, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(uint);
		memcpy(&meshImportSettings.customConfigurationFlags, cursor, bytes);

		cursor += bytes;

		bytes = sizeof(uint);
		memcpy(&meshImportSettings.size, cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Mesh: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Mesh: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

bool ResourceMesh::LoadInMemory()
{
	assert(meshData.vertices != nullptr && meshData.verticesSize > 0
		&& meshData.indices != nullptr && meshData.indicesSize > 0);

	App->sceneImporter->GenerateVBO(VBO, meshData.vertices, meshData.verticesSize);
	App->sceneImporter->GenerateIBO(IBO, meshData.indices, meshData.indicesSize);
	App->sceneImporter->GenerateVAO(VAO, VBO);

	return true;
}

bool ResourceMesh::UnloadFromMemory()
{
	App->sceneImporter->DeleteBufferObject(VBO);
	App->sceneImporter->DeleteBufferObject(IBO);
	App->sceneImporter->DeleteVertexArrayObject(VAO);

	return true;
}