#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"

#include "ResourceScript.h"

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>

#include "ScriptingModule.h"
#include "imgui/imgui.h"

#include <assert.h>

std::vector<std::string>ResourceScript::scriptNames;

ResourceScript::ResourceScript(uint uuid, ResourceData data, ResourceScriptData scriptData) : Resource(ResourceTypes::ScriptResource, uuid, data), scriptData(scriptData) 
{
	scriptName = data.name;
	scriptNames.push_back(scriptName);
}

ResourceScript::~ResourceScript()
{
	for (int i = 0; i < scriptNames.size(); ++i)
	{
		if (scriptNames[i] == scriptName)
			scriptNames.erase(scriptNames.begin() + i);
	}
}

void ResourceScript::OnPanelAssets()
{
#ifndef GAMEMODE
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
		ImGui::SetDragDropPayload("SCRIPT_RESOURCE", &res, sizeof(Resource*));
		ImGui::EndDragDropSource();
	}
#endif
}

bool ResourceScript::GenerateLibraryFiles() const
{
	assert(data.file.data() != nullptr);

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(data.file.data()) + 1, data.file.data()); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	// 1. Copy meta
	if (App->fs->Exists(metaFile))
	{
		// Read the info of the meta
		char* buffer;
		uint size = App->fs->Load(metaFile, &buffer);
		if (size > 0)
		{
			// Create a new name for the meta
			std::string extension;

			App->fs->GetExtension(metaFile, extension);

			char newMetaFile[DEFAULT_BUF_SIZE];
			sprintf_s(newMetaFile, "%s/%s%s%s", DIR_LIBRARY_SCRIPTS, data.name.data(), ".cs", extension.data());

			// Save the new meta (info + new name)
			size = App->fs->Save(newMetaFile, buffer, size);
			if (size > 0)
				delete[] buffer;						
		}
	}

	//2 Copy .cs file
	// Read the info of the meta
	if (App->fs->Exists(data.file.data()))
	{
		char* buffer;
		uint size = App->fs->Load(data.file.data(), &buffer);
		if (size > 0)
		{
			char newFile[DEFAULT_BUF_SIZE];
			sprintf_s(newFile, "%s/%s%s", DIR_LIBRARY_SCRIPTS, data.name.data(), ".cs");

			// Save the new file
			size = App->fs->Save(newFile, buffer, size);
			if (size > 0)
				delete[] buffer;
		}
	}

	return true;
}

void ResourceScript::SerializeToMeta(char*& cursor) const
{
	//Skip lastModTime
	uint bytes = sizeof(int64_t);
	cursor += bytes;

	bytes = sizeof(uint);
	uint numUUIDS = 1;
	memcpy(cursor, &numUUIDS, bytes);
	cursor += bytes;

	bytes = sizeof(uint32_t);
	memcpy(cursor, &uuid, bytes);
	cursor += bytes;

	uint nameSize = scriptName.size();
	bytes = sizeof(uint);
	memcpy(cursor, &nameSize, bytes);
	cursor += bytes;

	memcpy(cursor, scriptName.c_str(), nameSize);
	cursor += nameSize;
}

void ResourceScript::DeSerializeFromMeta(char*& cursor)
{
	//lastModTime + numUids + uid + Script State + nameLenght + name
	uint bytes = sizeof(int64_t);
	cursor += bytes;

	bytes = sizeof(uint);
	cursor += bytes;

	bytes = sizeof(uint32_t);
	memcpy(&uuid, cursor, bytes);
	cursor += bytes;

	uint nameSize;
	bytes = sizeof(uint);
	memcpy(&nameSize, cursor, bytes);
	cursor += bytes;

	scriptName.resize(nameSize);

	memcpy((void*)scriptName.c_str(), cursor, nameSize);
	cursor += nameSize;
}

uint ResourceScript::bytesToSerializeMeta() const
{
	//lastModTime + numUids + uid + Script State + nameLenght + name
	return sizeof(int64_t) + sizeof(uint) + sizeof(uint32_t) + sizeof(uint) + scriptName.size();
}

uint ResourceScript::getBytes() const
{
	return sizeof(ResourceScript);
}

bool ResourceScript::referenceMethods()
{
	//Referencing callback methods
	MonoMethodDesc* desc = mono_method_desc_new((scriptName + ":Awake()").data(), false);
	awakeMethod = mono_method_desc_search_in_image(desc, App->scripting->scriptsImage);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":Start()").data(), false);
	startMethod = mono_method_desc_search_in_image(desc, App->scripting->scriptsImage);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":PreUpdate()").data(), false);
	preUpdateMethod = mono_method_desc_search_in_image(desc, App->scripting->scriptsImage);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":Update()").data(), false);
	updateMethod = mono_method_desc_search_in_image(desc, App->scripting->scriptsImage);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":PostUpdate()").data(), false);
	postUpdateMethod = mono_method_desc_search_in_image(desc, App->scripting->scriptsImage);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":OnEnable()").data(), false);
	enableMethod = mono_method_desc_search_in_image(desc, App->scripting->scriptsImage);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":OnDisable()").data(), false);
	disableMethod = mono_method_desc_search_in_image(desc, App->scripting->scriptsImage);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":OnStop()").data(), false);
	stopMethod = mono_method_desc_search_in_image(desc, App->scripting->scriptsImage);
	mono_method_desc_free(desc);

	return true;
}

std::string ResourceScript::pathToWindowsNotation(const std::string& path) const
{
	std::string ret = path;
	while (ret.find("/") != std::string::npos)
	{
		ret = ret.replace(ret.find("/"), 1, "\\");
	}
	return ret;
}
