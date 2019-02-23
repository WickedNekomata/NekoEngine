#include "ResourcePrefab.h"
#include "imgui/imgui.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleGOs.h"
#include "ModuleFileSystem.h"

ResourcePrefab::ResourcePrefab(ResourceTypes type, uint uuid, ResourceData data, PrefabData customData) : Resource(type, uuid, data)
{
}

ResourcePrefab::~ResourcePrefab()
{
	//TODO: DESTROY THE ROOT IF LOADED
}

void ResourcePrefab::OnPanelAssets()
{
	ImGuiTreeNodeFlags flags = 0;
	flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

	if (App->scene->selectedObject == this)
		flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;

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
		Resource* res = this;
		ImGui::SetDragDropPayload("PREFAB_RESOURCE", &res, sizeof(Resource*));
		ImGui::EndDragDropSource();

		//TODO: RECEIVE THIS DRAG AND DROP IN THE HIERARCHY AND INSTANTIATE A COPY OF THE LOADED ROOT WITH THE COPY CONSTRUCTOR
	}
}

bool ResourcePrefab::ImportFile(const char * file, std::string & name, std::string & outputFile)
{

	assert(file != nullptr);

	bool imported = false;

	// TODO_G

	// meta?
	// look for uuid and force force it (outputFile)

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file) + 1, file); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	if (App->fs->Exists(metaFile))
	{
		uint uuid = 0;
		int64_t lastModTime = 0;
		std::string prefab_name;
		ResourcePrefab::ReadMeta(metaFile, lastModTime, uuid, prefab_name);
		assert(uuid > 0);

		name = prefab_name.data();

		char entry[DEFAULT_BUF_SIZE];
		sprintf_s(entry, "%u", uuid);
		outputFile = entry;
	}

	return true;//sorry
}

bool ResourcePrefab::ExportFile(ResourceData & data, PrefabData & prefabData, std::string & outputFile, bool overwrite)
{
	bool ret = false;
	uint size = 0u;
	char* buffer = nullptr;
	prefabData.size = 0u;
	
	App->GOs->SerializeFromNode(prefabData.root, prefabData.buffer, prefabData.size);

	if (overwrite)
		outputFile = data.file;
	else
		outputFile = data.name;

	ret = App->fs->SaveInGame(buffer, size, FileType::PrefabFile, outputFile, overwrite) > 0;

	if (ret) {
		CONSOLE_LOG(LogTypes::Normal, "PREFAB EXPORT: Successfully saved prefab '%s'", outputFile.data());
	}else
		CONSOLE_LOG(LogTypes::Normal, "PREFAB EXPORT: Error saving prefab '%s'", outputFile.data());

	return true;
}

uint ResourcePrefab::CreateMeta(const char * file, uint prefab_uuid, std::string & name, std::string & outputMetaFile)
{
	assert(file != nullptr);

	uint uuidsSize = 1;
	uint nameSize = DEFAULT_BUF_SIZE;

	// Name
	char shaderName[DEFAULT_BUF_SIZE];
	strcpy_s(shaderName, DEFAULT_BUF_SIZE, name.data());

	uint size =
		sizeof(int64_t) +
		sizeof(uint) +
		sizeof(uint) * uuidsSize +

		sizeof(char) * nameSize;

	char* data = new char[size];
	char* cursor = data;

	// 1. Store last modification time
	int64_t lastModTime = App->fs->GetLastModificationTime(file);
	assert(lastModTime > 0);
	uint bytes = sizeof(int64_t);
	memcpy(cursor, &lastModTime, bytes);

	cursor += bytes;

	// 3. Store shader object uuid
	bytes = sizeof(uint);
	memcpy(cursor, &prefab_uuid, bytes);

	cursor += bytes;

	// 4. Store shader object name size
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);

	cursor += bytes;

	// 5. Store prefab object name
	bytes = sizeof(char) * nameSize;
	memcpy(cursor, shaderName, bytes);

	// --------------------------------------------------

	// Build the path of the meta file and save it
	outputMetaFile = file;
	outputMetaFile.append(EXTENSION_META);
	uint resultSize = App->fs->Save(outputMetaFile.data(), data, size);
	if (resultSize > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "Resource Prefab Object: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Prefab Object: Could not save meta '%s'", outputMetaFile.data());
		return 0;
	}

	return lastModTime;
}

bool ResourcePrefab::ReadMeta(const char * metaFile, int64_t & lastModTime, uint & prefab_uuid, std::string & name)
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

		// 3. Load prefab object uuid
		bytes = sizeof(uint);
		memcpy(&prefab_uuid, cursor, bytes);

		cursor += bytes;

		// 4. Load prefab object name size
		uint nameSize = 0;
		bytes = sizeof(uint);
		memcpy(&nameSize, cursor, bytes);
		assert(nameSize > 0);

		cursor += bytes;

		// 5. Load prefab object name
		name.resize(nameSize);
		bytes = sizeof(char) * nameSize;
		memcpy(&name[0], cursor, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource Prefab Object: Successfully loaded meta '%s'", metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource Prefab Object: Could not load meta '%s'", metaFile);
		return false;
	}

	return true;
}

bool ResourcePrefab::LoadFile(const char * file, PrefabData & prefab_data_output)
{
	assert(file != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);

	if (size > 0) {
		memcpy(&prefab_data_output, buffer, size);

		CONSOLE_LOG(LogTypes::Normal, "Resource Prefab: Successfully loaded prefab '%s'", file);
		RELEASE_ARRAY(buffer);
	}
	else {
		CONSOLE_LOG(LogTypes::Error, "Resource Prefab: error loadeding prefab '%s'", file);
	}

	return true;
}

bool ResourcePrefab::LoadInMemory()
{
	//TODO: LOAD THE ROOT

	//App->GOs->LoadScene(my_data.buffer, my_data.size); todo

	return true;
}

bool ResourcePrefab::UnloadFromMemory()
{
	//TODO: DELETE THE ROOT FROM MEMORY
	return true;
}
