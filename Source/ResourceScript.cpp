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

	bytes = sizeof(ScriptState);
	memcpy(cursor, &state, bytes);
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

	bytes = sizeof(ScriptState);
	memcpy(&state, cursor, bytes);
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
	return sizeof(int64_t) + sizeof(uint) + sizeof(uint32_t) + sizeof(ScriptState) + sizeof(uint) + scriptName.size();
}

uint ResourceScript::getBytes() const
{
	return sizeof(ResourceScript);
}

bool ResourceScript::preCompileErrors()
{
	std::string goRoot(R"(cd\ )");
	std::string goMonoBin(" cd \"" + App->fs->getAppPath() + "\\Mono\\bin\"");

	std::string compileCommand(" mcs -target:library ");

	std::string fileName = data.file.substr(data.file.find_last_of("/") + 1);
	std::string windowsFormattedPath = pathToWindowsNotation(data.file);

	std::string path = std::string("\"" + std::string(App->fs->getAppPath())) + windowsFormattedPath + "\"";

	std::string redirectOutput(" 1> \"" + pathToWindowsNotation(App->fs->getAppPath()) + "LogError.txt\"" + std::string(" 2>&1"));

	std::string error;

	std::string temp(goRoot + "&" + goMonoBin + "&" + compileCommand + path + " " + App->scripting->getReferencePath() + redirectOutput);
	if (!exec(std::string(goRoot + "&" + goMonoBin + "&" + compileCommand + path + " " + App->scripting->getReferencePath() + redirectOutput).data(), error))
	{
		CONSOLE_LOG(LogTypes::Error, "Error compiling the script %s:", fileName.data());

		char* buffer; 
		int size = App->fs->Load("LogError.txt", &buffer);

		std::string outPut(buffer);
		outPut.resize(size);

		CONSOLE_LOG(LogTypes::Error, outPut.data());

		delete[] buffer;

		state = ScriptState::COMPILED_WITH_ERRORS;

		//Deleting the .dll
		std::string pathNoExt = data.file.substr(0, data.file.find_last_of("."));
		App->fs->deleteFile(pathNoExt + ".dll");

		return true;
	}

	//Deleting the .dll
	std::string pathNoExt = data.file.substr(0, data.file.find_last_of("."));
	App->fs->deleteFile(pathNoExt + ".dll");
	return false;
}

bool ResourceScript::Compile()
{
	bool ret = true;

	std::string goRoot(R"(cd\ )");
	std::string goMonoBin(" cd \"" + App->fs->getAppPath() + "\\Mono\\bin\"");

	std::string compileCommand(" mcs -target:library ");

	std::string fileName = data.file.substr(data.file.find_last_of("/") + 1);
	std::string windowsFormattedPath = pathToWindowsNotation(data.file);

	std::string path = std::string("\"" + std::string(App->fs->getAppPath())) + windowsFormattedPath + "\"";

	std::string redirectOutput(" 1> \"" + pathToWindowsNotation(App->fs->getAppPath()) + "LogError.txt\"" + std::string(" 2>&1"));

	std::string error;

	if (!exec(std::string(goRoot + "&" + goMonoBin + "&" + compileCommand + path + " " + App->scripting->getReferencePath() + redirectOutput).data(), error))
	{
		//When you create the script it's compiled. When you modify the script this is being precompiled first, and already logged errors, so we don't log them again.
		if (firstCompiled)
		{
			firstCompiled = false;
			
			//Debug.LogError("Error compiling the script %s:", fileName.data());

			char* buffer; 
			int size = App->fs->Load("LogError.txt", &buffer);

			std::string outPut(buffer);
			outPut.resize(size);

			//Debug.LogError(outPut.data());

			delete[] buffer;
		}

		ret = false;
		state = ScriptState::COMPILED_WITH_ERRORS;
	}

	if (ret)
	{
		state = ScriptState::COMPILED_FINE;

		//Move the dll to the Library folder.
		std::string dllPath = data.file;
		dllPath = dllPath.substr(0, dllPath.find_last_of("."));
		dllPath += ".dll";

		std::string fileNameNoExt = data.file.substr(data.file.find_last_of("/") + 1);
		fileNameNoExt = fileNameNoExt.substr(0, fileNameNoExt.find("."));

		if (!App->fs->MoveFileInto(dllPath, "Library/Scripts/" + fileNameNoExt + ".dll"))
			return false;
		
		data.exportedFile = "Library/Scripts/" + fileNameNoExt + ".dll";

		referenceMethods();
	}

	return ret;
}

bool ResourceScript::referenceMethods()
{
	state = ScriptState::COMPILED_FINE;

	std::string fileNameNoExt = data.file.substr(data.file.find_last_of("/") + 1);
	fileNameNoExt = fileNameNoExt.substr(0, fileNameNoExt.find("."));

	data.exportedFile = "Library/Scripts/" + fileNameNoExt + ".dll";

	//Referencing the assembly from memory
	char* buffer;
	int size = App->fs->Load(data.exportedFile, &buffer);
	if(size <= 0)
		return false;

	//Loading assemblies from data instead of from file
	MonoImageOpenStatus status = MONO_IMAGE_ERROR_ERRNO;
	image = mono_image_open_from_data(buffer, size, 1, &status);

	assembly = mono_assembly_load_from_full(image, (std::string("assembly") + std::to_string(uuid)).data(), &status, false);

	delete[] buffer;

	if (!assembly || !image)
	{
		//Somehow the .dll could not be found.
		return false;
	}

	//Referencing callback methods
	MonoMethodDesc* desc = mono_method_desc_new((scriptName + ":Awake()").data(), false);
	awakeMethod = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":Start()").data(), false);
	startMethod = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":PreUpdate()").data(), false);
	preUpdateMethod = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":Update()").data(), false);
	updateMethod = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":PostUpdate()").data(), false);
	postUpdateMethod = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":OnEnable()").data(), false);
	enableMethod = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":OnDisable()").data(), false);
	disableMethod = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);

	desc = mono_method_desc_new((scriptName + ":OnStop()").data(), false);
	stopMethod = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);
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
