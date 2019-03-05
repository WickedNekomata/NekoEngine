#include "ScriptingModule.h"
#include "ComponentScript.h"
#include "ResourceScript.h"
#include "ComponentTransform.h"
#include "ComponentNavAgent.h"
#include "ComponentAnimation.h"
#include "ComponentEmitter.h"
#include "ComponentRectTransform.h"
#include "ComponentButton.h"

#include "GameObject.h"

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/metadata/mono-config.h>

#include <array>
#include <iostream>

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h"
#include "ModuleTimeManager.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleGOs.h"
#include "ModuleGui.h"
#include "ModuleLayers.h"
#include "ModulePhysics.h"
#include "SceneQueries.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleUI.h"

#include "MathGeoLib/include/MathGeoLib.h"
#include "Brofiler/Brofiler.h"


#define NAVMESHAGENT_ASCII 1299603790

bool exec(const char* cmd, std::string& error)
{
	std::array<char, 128> buffer;
	bool result;
	auto pipe = _popen(cmd, "r");

	if (!pipe) throw std::runtime_error("popen() failed!");

	while (!feof(pipe))
	{
		if (fgets(buffer.data(), 128, pipe) != nullptr)
			error += buffer.data();
	}

	auto rc = _pclose(pipe);

	if (rc == EXIT_SUCCESS)
	{
		std::cout << "SUCCESS\n";
		result = true;
	}
	else
	{
		std::cout << "FAILED\n";
		result = false;
	}

	return result;
}

bool ScriptingModule::Init(JSON_Object* data)
{
	//Locate the lib and etc folders in the mono installation
	std::string MonoLib, MonoEtc, gamePath = App->fs->getAppPath();
	MonoLib = gamePath + "Mono\\lib";
	MonoEtc = gamePath + "Mono\\etc";

	mono_set_dirs(MonoLib.data(), MonoEtc.data());
	mono_config_parse(NULL);

	//Initialize the mono domain
	domain = mono_jit_init("Scripting");
	if (!domain)
		return false;

	CreateDomain();

	if (!internalAssembly)
		return true;

	char* args[1];
	args[0] == "InternalAssembly";
	mono_jit_exec(domain, internalAssembly, 1, args);

	return true;
}

bool ScriptingModule::Start()
{

#ifndef GAMEMODE
	CreateScriptingProject();
	IncludeCSFiles();
#endif

	return true;
}

update_status ScriptingModule::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ScriptingModule::Update()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif
	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
	{
		UpdateMethods();
	}

	return UPDATE_CONTINUE;
}

update_status ScriptingModule::PostUpdate()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);
#endif // !GAMEMODE
	if (someScriptModified || engineOpened)
	{
#ifndef GAMEMODE
		RecompileScripts();
		someScriptModified = false;
		engineOpened = false;
#else
		//Engine opened recently, import the .dll if found

		/*System_Event event;
		event.type = System_Event_Type::ScriptingDomainReloaded;
		App->PushSystemEvent(event);

		App->scripting->CreateDomain();
		App->scripting->UpdateScriptingReferences();

		std::vector<Resource*> scriptResources = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
		for (int i = 0; i < scriptResources.size(); ++i)
		{
			ResourceScript* scriptRes = (ResourceScript*)scriptResources[i];
			scriptRes->referenceMethods();
		}

		App->scripting->ReInstance();

		engineOpened = false;*/

#endif
	}

	return UPDATE_CONTINUE;
}

bool ScriptingModule::CleanUp()
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->GetParent()->EraseComponent(scripts[i]);
		delete scripts[i];
	}

	scripts.clear();

	if(domain)
		mono_jit_cleanup(domain);

	domain = nullptr;

	return true;
}

void ScriptingModule::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::Play:
		{
			//TODO: Check if some files have compile errors and don't let the user hit the play.

			for (int i = 0; i < scripts.size(); ++i)
			{
				if (scripts[i]->IsTreeActive())
					scripts[i]->OnEnableMethod();
			}

			for (int i = 0; i < scripts.size(); ++i)
			{
				if (scripts[i]->IsTreeActive() && !scripts[i]->awaked)
					scripts[i]->Awake();
			}

			for (int i = 0; i < scripts.size(); ++i)
			{
				if (scripts[i]->IsTreeActive())
					scripts[i]->Start();
			}
		
			//Call the Awake and Start for all the Enabled script in the Play instant.
			break;
		}
	
		case System_Event_Type::Stop:
		{
			for (int i = 0; i < scripts.size(); ++i)
			{
				if (scripts[i]->IsTreeActive())
				{
					scripts[i]->OnStop();
				}
			}

			ClearMap();

			break;
		}

		case System_Event_Type::LoadScene:
		{
			ClearMap();
			break;
		}

		case System_Event_Type::ResourceDestroyed:
		{		
			for (int i = 0; i < scripts.size(); ++i)
			{
				bool somethingDestroyed = false;

				if (scripts[i]->scriptResUUID == event.resEvent.resource->GetUuid())
				{
					somethingDestroyed = true;
					scripts[i]->GetParent()->EraseComponent(scripts[i]);
					delete scripts[i];
					scripts.erase(scripts.begin() + i);

					i--;
				}
				if (somethingDestroyed)
				{
					IncludeCSFiles();
				}
			}			
		}

		case System_Event_Type::GameObjectDestroyed:
		{
			for (int i = 0; i < monoObjectHandles.size(); ++i)
			{
				MonoObject* monoObject = mono_gchandle_get_target(monoObjectHandles[i]);

				int address;
				mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

				GameObject* gameObject = (GameObject*)address;

				bool destroyed = false;
				while (gameObject)
				{
					if (gameObject == event.goEvent.gameObject)
					{
						destroyed = true;
						break;
					}
					gameObject = gameObject->GetParent() ? gameObject->GetParent() : nullptr;
				}

				if (destroyed)				
				{
					MonoClass* monoObjectClass = mono_object_get_class(monoObject);

					MonoClassField* deletedField = mono_class_get_field_from_name(monoObjectClass, "destroyed");

					bool temp = true;
					mono_field_set_value(monoObject, deletedField, &temp);

					mono_gchandle_free(monoObjectHandles[i]);

					monoObjectHandles.erase(monoObjectHandles.begin() + i);
					i--;
				}
			}		
			break;
		}

		case System_Event_Type::ComponentDestroyed:
		{
			Component* toDelete = event.compEvent.component;
			MonoObject* monoComponent = toDelete->GetMonoComponent();
			if (monoComponent)
			{
				bool destroyed = true;
				mono_field_set_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "destroyed"), &destroyed);

				for (int i = 0; i < monoComponentHandles.size(); ++i)
				{
					if (monoComponent == mono_gchandle_get_target(monoComponentHandles[i]))
					{
						mono_gchandle_free(monoComponentHandles[i]);
						monoComponentHandles.erase(monoComponentHandles.begin() + i);
						i--;
					}						
				}
			}
			break;
		}

		case System_Event_Type::LoadGMScene:
		{
			//Engine opened recently, import the .dll if found
			if (engineOpened)
			{
				System_Event event;
				event.type = System_Event_Type::ScriptingDomainReloaded;
				App->PushSystemEvent(event);

				App->scripting->CreateDomain();
				App->scripting->UpdateScriptingReferences();

				std::vector<Resource*> scriptResources = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
				for (int i = 0; i < scriptResources.size(); ++i)
				{
					ResourceScript* scriptRes = (ResourceScript*)scriptResources[i];
					scriptRes->referenceMethods();
				}

				App->scripting->ReInstance();

				engineOpened = false;
			}		
			break;
		}
	}
}

ComponentScript* ScriptingModule::CreateScriptComponent(std::string scriptName, bool createCS)
{
	while (scriptName.find(" ") != std::string::npos)
	{
		scriptName = scriptName.replace(scriptName.find(" "), 1, "");
	}

	ComponentScript* script = new ComponentScript(scriptName);
	char* buffer;
	int size;

	if (createCS)
	{
		size = App->fs->Load("Internal/SampleScript/SampleScript.cs", &buffer);

		std::string scriptStream = buffer;
		scriptStream.resize(size);

		while (scriptStream.find("SampleScript") != std::string::npos)
		{
			scriptStream = scriptStream.replace(scriptStream.find("SampleScript"), 12, scriptName);
		}

		App->fs->Save("Assets/Scripts/" + scriptName + ".cs", (char*)scriptStream.c_str(), scriptStream.size());

		IncludeCSFiles();

		delete[] buffer;
	}

	ResourceScript* scriptRes = nullptr;

	if (App->fs->Exists("Assets/Scripts/" + scriptName + ".cs.meta"))
	{
		char* metaBuffer;
		uint metaSize;

		metaSize = App->fs->Load("Assets/Scripts/" + scriptName + ".cs.meta", &metaBuffer);

		char* cursor = metaBuffer;
		cursor += sizeof(int64_t) + sizeof(uint);

		uint32_t UID;
		memcpy(&UID, cursor, sizeof(uint32_t));

		scriptRes = (ResourceScript*)App->res->GetResource(UID);

		delete[] metaBuffer;
	}
	
	if (!scriptRes)
	{
		//Here we have to reference a new ResourceScript with the .cs we have created, but the ResourceManager will still be sending file created events, and we would have data duplication.
		//We disable this behavior and control the script creation only with this method, so we do not care for external files out-of-engine created.
		
		ResourceData data;
		data.name = scriptName;
		data.file = "Assets/Scripts/" + scriptName + ".cs";
		data.exportedFile = "";
	
		scriptRes = (ResourceScript*)App->res->CreateResource(ResourceTypes::ScriptResource, data, &ResourceScriptData());

		//Create the .meta, to make faster the search in the map storing the uid.
		uint bytes = scriptRes->bytesToSerializeMeta();
		char* buffer = new char[bytes];
		char* cursor = buffer;
		scriptRes->SerializeToMeta(cursor);

		App->fs->Save("Assets/Scripts/" + scriptName + ".cs.meta", buffer, bytes);

		someScriptModified = true;

		delete[] buffer;
	}

	App->res->SetAsUsed(scriptRes->GetUuid());
	script->scriptResUUID = scriptRes->GetUuid();

	scripts.push_back(script);

	return script;
}

bool ScriptingModule::DestroyScript(ComponentScript* script)
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		if (scripts[i] == script)
		{
			delete script;
			scripts.erase(scripts.begin() + i);
			return true;
		}
	}

	return false;
}

void ScriptingModule::ClearScriptComponent(ComponentScript* script)
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		if (scripts[i] == script)
		{
			scripts.erase(scripts.begin() + i);
			break;
		}
	}
}

MonoObject* ScriptingModule::MonoObjectFrom(GameObject* gameObject)
{
	MonoObject* monoObject = gameObject->GetMonoObject();

	if (monoObject)
		return monoObject;

	MonoClass* gameObjectClass = mono_class_from_name(internalImage, "JellyBitEngine", "GameObject");
	monoObject = mono_object_new(domain, gameObjectClass);
	mono_runtime_object_init(monoObject);

	int address = (int)gameObject;
	mono_field_set_value(monoObject, mono_class_get_field_from_name(gameObjectClass, "cppAddress"), &address);

	uint32_t handleID = mono_gchandle_new(monoObject, true);

	gameObject->SetMonoObject(handleID);

	monoObjectHandles.push_back(handleID);

	return monoObject;
}

GameObject* ScriptingModule::GameObjectFrom(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	return (GameObject*)address;
	
	//We only can create MonoObjects though a GameObject*, not viceversa.
}

MonoObject* ScriptingModule::MonoComponentFrom(Component* component)
{
	MonoObject* monoComponent = nullptr;
	monoComponent = component->GetMonoComponent();
	if (monoComponent)
		return monoComponent;

	switch (component->GetType())
	{
		case ComponentTypes::CameraComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Camera"));			
			break;
		}
		case ComponentTypes::BoxColliderComponent:
		case ComponentTypes::CapsuleColliderComponent:
		case ComponentTypes::PlaneColliderComponent:
		case ComponentTypes::SphereColliderComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Collider"));
			break;
		}

		case ComponentTypes::NavAgentComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "NavMeshAgent"));
			break;
		}

		case ComponentTypes::AnimationComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Animator"));
			break;
		}
		case ComponentTypes::EmitterComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "ParticleEmitter"));
			break;
		}
		case ComponentTypes::RectTransformComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine.UI", "RectTransform"));
			break;
		}
		case ComponentTypes::ButtonComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine.UI", "Button"));
			break;
		}
		case ComponentTypes::ImageComponent:
		{
			monoComponent = mono_object_new(App->scripting->domain, mono_class_from_name(App->scripting->internalImage, "JellyBitEngine.UI", "Image"));
			break;
		}
	}

	if (!monoComponent)
		return nullptr;

	mono_runtime_object_init(monoComponent);

	int gameObjectAddress = (int)component->GetParent();
	int componentAddress = (int)component;

	mono_field_set_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "gameObjectAddress"), &gameObjectAddress);
	mono_field_set_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "componentAddress"), &componentAddress);
	mono_field_set_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "gameObject"), MonoObjectFrom(component->GetParent()));

	uint32_t handleID = mono_gchandle_new(monoComponent, true);
	component->SetMonoComponent(handleID);

	monoComponentHandles.push_back(handleID);

	return monoComponent;
}

Component* ScriptingModule::ComponentFrom(MonoObject* monoComponent)
{
	int componentAddress;
	mono_field_get_value(monoComponent, mono_class_get_field_from_name(mono_object_get_class(monoComponent), "componentAddress"), &componentAddress);	

	return (Component*)componentAddress;
}

bool ScriptingModule::alreadyCreated(std::string scriptName)
{
	clearSpaces(scriptName);

	for (int i = 0; i < scripts.size(); ++i)
	{
		if (scriptName == scripts[i]->scriptName)
			return true;
	}

	return false;
}

void ScriptingModule::CreateScriptingProject()
{
	if (App->fs->Exists("ScriptingProject.sln"))
		return;

	App->fs->CopyDirectoryAndContentsInto("Internal/ScriptingProject", "", false);
}

void ScriptingModule::ExecuteScriptingProject()
{
	//We need the path to the Visual Studio .exe and after that ask the program to start opening the right solution. No idea how to do that for now.
#if 0
	CreateScriptingProject();

	IncludecsFiles();

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	bool ret = CreateProcess("C:\\Users\\Jony635\\Desktop\\Proyectos 3o\\GitHub\\Flan3DEngine\\Flan3DEngine\\Game\\ScriptingProject.sln", "C:\\Users\\Jony635\\Desktop\\Proyectos 3o\\GitHub\\Flan3DEngine\\Flan3DEngine\\Game\\ScriptingProject.sln", 0, 0, false, 0, 0, 0, &si, &pi) != 0;

	if (!ret)
	{
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);

		Debug.LogError("Could not open \"ScriptingProject.sln\". Error: %s", message.data());

		//Free the buffer.
		LocalFree(messageBuffer);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#endif
}

void ScriptingModule::IncludeCSFiles()
{
	Directory scripts = App->fs->RecursiveGetFilesFromDir("Assets/Scripts");

	//Modify the project settings file, stored in a xml

	char* buffer;
	int size = App->fs->Load("Assembly-CSharp.csproj", &buffer);
	if (size <= 0)
		return;

	pugi::xml_document configFile;
	configFile.load_buffer(buffer, size);
	pugi::xml_node projectNode = configFile.child("Project");
	pugi::xml_node itemGroup = projectNode.child("ItemGroup").next_sibling();
	
	for (pugi::xml_node compile = itemGroup.child("Compile"); compile != nullptr;)
	{
		pugi::xml_node next = compile.next_sibling();

		//Do not delete the internal files
		if (std::string(compile.attribute("Include").as_string()).find("internal") != std::string::npos)
		{
			compile = next;
			continue;
		}
			
		itemGroup.remove_child(compile);
		compile = next;
	}

	IncludeCSFiles(itemGroup, scripts);

	std::ostringstream stream;
	configFile.save(stream, "\r\n");
	App->fs->Save("Assembly-CSharp.csproj", (char*)stream.str().data(), stream.str().size());	
}

void ScriptingModule::IncludeCSFiles(pugi::xml_node& nodeToAppend, const Directory& dir)
{
	for (int i = 0; i < dir.files.size(); ++i)
	{
		std::string extension;
		App->fs->GetExtension(dir.files[i].name.data(), extension);
		if (extension != ".cs")
			continue;

		nodeToAppend.append_child("Compile").append_attribute("Include").set_value(std::string("Assets\\Scripts\\" + dir.files[i].name).data());
	}

	for (int i = 0; i < dir.directories.size(); ++i)
	{
		IncludeCSFiles(nodeToAppend, dir.directories[i]);
	}
}

void ScriptingModule::CreateInternalCSProject()
{
	if (App->fs->Exists("JellyBitCS"))
		return;

	App->fs->CopyDirectoryAndContentsInto("Internal/JellyBitCS", "", true);
}

std::string ScriptingModule::getReferencePath() const
{
	std::string internalCommand = std::string("-r:") + std::string("\"") + App->fs->getAppPath() + std::string("JellyBitCS.dll\" " + std::string("-lib:\"") + App->fs->getAppPath() + "\\Library\\Scripts\" ");
	std::string referencesCommand;

	/*std::vector<Resource*> scripts = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
	for (int i = 0; i < scripts.size(); ++i)
	{
		ResourceScript* script = (ResourceScript*)scripts[i];
		referencesCommand += std::string("-r:") + std::string("\"") + App->fs->getAppPath() + std::string("Library\\Scripts\\") + script->scriptName + ".dll" + std::string("\" ");
	}*/

	return internalCommand + referencesCommand;
}

std::string ScriptingModule::clearSpaces(std::string& scriptName)
{
	while (scriptName.find(" ") != std::string::npos)
	{
		scriptName = scriptName.replace(scriptName.find(" "), 1, "");
	}
	return scriptName;
}

void ScriptingModule::ReInstance()
{
	for (int i = 0; i < scripts.size(); ++i)
	{	
		scripts[i]->InstanceClass();
	}
}

void ScriptingModule::ClearMap()
{
	for (int i = 0; i < monoObjectHandles.size(); ++i)
	{
		mono_gchandle_free(monoObjectHandles[i]);
	}
	monoObjectHandles.clear();
}

Resource* ScriptingModule::ImportScriptResource(const char* file)
{
	//May be new file or generic file event

	std::string fileString = file;
	std::string metaFile = fileString + ".meta";

	std::string scriptName = fileString.substr(fileString.find_last_of("/") + 1);
	scriptName = scriptName.substr(0, scriptName.find_last_of("."));

	ResourceData data;
	data.name = scriptName;
	data.file = file;
	data.exportedFile = "";

	ResourceScript* scriptRes = nullptr;

	bool scriptModified = false;

	if (!App->fs->Exists(data.file + ".meta"))
	{
		scriptRes = (ResourceScript*)App->res->CreateResource(ResourceTypes::ScriptResource, data, &ResourceScriptData());

		//Create the .meta
		uint bytes = scriptRes->bytesToSerializeMeta();
		char* buffer = new char[bytes];
		char* cursor = buffer;
		scriptRes->SerializeToMeta(cursor);

		App->fs->Save(fileString + ".meta", buffer, bytes);

		delete[] buffer;
	}
	else
	{
		char* metaBuffer;
		uint size = App->fs->Load(metaFile, &metaBuffer);
		if (size > 0)
		{
			char* cursor = metaBuffer;
			int64_t lastSavedModTime;
			memcpy(&lastSavedModTime, cursor, sizeof(int64_t));
			cursor += sizeof(int64_t);
			cursor += sizeof(uint);
			uint uid;
			memcpy(&uid, cursor, sizeof(uint));

			int64_t lastModTime = App->fs->GetLastModificationTime(file);
			
			scriptModified = lastSavedModTime != lastModTime;

			if (scriptModified)
			{
				cursor = metaBuffer;
				memcpy(cursor, &lastModTime, sizeof(int64_t));
				App->fs->Save(metaFile, metaBuffer, size);

				someScriptModified = true;
			}

			scriptRes = (ResourceScript*)App->res->CreateResource(ResourceTypes::ScriptResource, data, &ResourceScriptData(), uid);

			cursor = metaBuffer;
			scriptRes->DeSerializeFromMeta(cursor);
			delete[] metaBuffer;
		}
	}
		
	return scriptRes;
}

void ScriptingModule::ScriptModified(const char* scriptPath)
{
	/*char metaFile[DEFAULT_BUF_SIZE];
	strcpy(metaFile, scriptPath);
	strcat(metaFile, ".meta");

	char* metaBuffer;
	uint size = App->fs->Load(metaFile, &metaBuffer);
	if (size < 0)
		return;

	char* cursor = metaBuffer;
	cursor += sizeof(int64_t) + sizeof(uint);
	uint UID;
	memcpy(&UID, cursor, sizeof(uint32_t));

	ResourceScript* scriptModified = (ResourceScript*)App->res->GetResource(UID);
	if (scriptModified->preCompileErrors())
		return;

	System_Event event;
	event.type = System_Event_Type::ScriptingDomainReloaded;
	App->PushSystemEvent(event);*/

	someScriptModified = true;
}

void ScriptingModule::RecompileScripts()
{
	std::string goRoot(R"(cd\ )");
	std::string goMonoBin(" cd \"" + App->fs->getAppPath() + "\\Mono\\bin\"");
	std::string compileCommand(" mcs -target:library ");

	/*std::string fileName = data.file.substr(data.file.find_last_of("/") + 1);
	std::string windowsFormattedPath = pathToWindowsNotation(data.file);*/

	std::string path = std::string("\"" + std::string(App->fs->getAppPath())) + std::string("\\Assets\\Scripts\\*.cs") + "\"";
	std::string redirectOutput(" 1> \"" + /*pathToWindowsNotation*/(App->fs->getAppPath()) + "LogError.txt\"" + std::string(" 2>&1"));
	std::string outputFile(" -out:..\\..\\Library\\Scripts\\Scripting.dll ");

	std::string error;
	if (!exec(std::string(goRoot + "&" + goMonoBin + "&" + compileCommand + path + " " + outputFile + App->scripting->getReferencePath() + redirectOutput).data(), error))
	{
		char* buffer;
		int size = App->fs->Load("LogError.txt", &buffer);
		if (size > 0)
		{
			std::string outPut(buffer);
			outPut.resize(size);

			CONSOLE_LOG(LogTypes::Error, "Error compiling Scripting assembly. Error: %s", outPut.data());

			delete[] buffer;
		}		
	}
	else
	{
		//Send the DomainReloaded event
		System_Event event;
		event.type = System_Event_Type::ScriptingDomainReloaded;
		App->PushSystemEvent(event);

		App->scripting->CreateDomain();
		App->scripting->UpdateScriptingReferences();

		std::vector<Resource*> scriptResources = App->res->GetResourcesByType(ResourceTypes::ScriptResource);
		for (int i = 0; i < scriptResources.size(); ++i)
		{
			ResourceScript* scriptRes = (ResourceScript*)scriptResources[i];
			scriptRes->referenceMethods();
		}

		App->scripting->ReInstance();
	}
}

void ScriptingModule::GameObjectKilled(GameObject* killed)
{
	for (int i = 0; i < monoObjectHandles.size(); ++i)
	{
		if (mono_gchandle_get_target(monoObjectHandles[i]) == killed->GetMonoObject())
		{
			monoObjectHandles.erase(monoObjectHandles.begin() + i);
			break;
		}
	}
}

void ScriptingModule::UpdateMethods()
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->PreUpdate();
	}
	
	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->Update();
	}

	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->PostUpdate();
	}
}

void ScriptingModule::ExecuteCallbacks(GameObject* gameObject)
{
	for (int i = 0; i < gameObject->components.size(); ++i)
	{
		Component* comp = gameObject->components[i];
		if (comp->GetType() == ComponentTypes::ScriptComponent)
		{
			ComponentScript* script = (ComponentScript*)comp;

			script->OnEnableMethod();
			script->Awake();
			script->Start();
		}
	}

	for (int i = 0; i < gameObject->children.size(); ++i)
	{
		ExecuteCallbacks(gameObject->children[i]);
	}
}

//-----------------------------

void DebugLogTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	CONSOLE_LOG(LogTypes::Normal, string);

	mono_free(string);
}

void DebugLogWarningTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	CONSOLE_LOG(LogTypes::Warning, string);

	mono_free(string);
}

void DebugLogErrorTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	CONSOLE_LOG(LogTypes::Error, string)

	mono_free(string);
}

void ClearConsole() 
{ 
#ifndef GAMEMODE
	App->gui->ClearConsole();
#endif
}

int32_t GetKeyStateCS(int32_t key)
{
	return App->input->GetKey(key);
}

int32_t GetMouseStateCS(int32_t button)
{
	return App->input->GetMouseButton(button);
}

MonoArray* GetMousePosCS()
{
	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 2);
	int x = App->input->GetMouseX();
	int y = App->input->GetMouseY();
	mono_array_set(ret, float, 0, x);
	mono_array_set(ret, float, 1, y);

	return ret;
}

MonoArray* GetMouseDeltaPosCS()
{
	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 2);
	mono_array_set(ret, float, 0, App->input->GetMouseXMotion());
	mono_array_set(ret, float, 1, App->input->GetMouseYMotion());

	return ret;
}

int GetWheelMovementCS()
{
	return App->input->GetMouseZ();
}

MonoObject* InstantiateGameObject(MonoObject* templateMO, MonoArray* position, MonoArray* rotation)
{
	if (!templateMO)
	{
		//Instantiate an empty GameObject and returns the MonoObject

		GameObject* instance = App->GOs->CreateGameObject("default", App->scene->root);

		MonoClass* gameObjectClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "GameObject");
		MonoObject* monoInstance = mono_object_new(App->scripting->domain, gameObjectClass);
		mono_runtime_object_init(monoInstance);

		if (position)
		{
			math::float3 newPos{mono_array_get(position, float, 0), mono_array_get(position, float, 1), mono_array_get(position, float, 2)};
			instance->transform->position = newPos;
		}

		if (rotation)
		{
			math::Quat newRotation{ mono_array_get(position, float, 0), mono_array_get(position, float, 1), mono_array_get(position, float, 2), mono_array_get(position, float, 3)};
			instance->transform->rotation = newRotation;
		}

		uint32_t handleID = mono_gchandle_new(monoInstance, true);

		instance->SetMonoObject(handleID);

		int address = (int)instance;
		mono_field_set_value(monoInstance, mono_class_get_field_from_name(gameObjectClass, "cppAddress"), &instance);

		App->scripting->monoObjectHandles.push_back(handleID);

		return monoInstance;
	}

	else
	{
		//Search for the monoTemplate and his GameObject representation in the map, create 2 new copies,
		//add the GameObject to the Scene Hierarchy and returns the monoObject. Store this new Instantiated objects in the map.

		GameObject* templateGO = nullptr;

		for (int i = 0; i < App->scripting->monoObjectHandles.size(); ++i)
		{
			uint32_t handleID = App->scripting->monoObjectHandles[i];
			MonoObject* temp = mono_gchandle_get_target(handleID);

			if (temp == templateMO)
			{
				int address;
				mono_field_get_value(temp, mono_class_get_field_from_name(mono_object_get_class(temp), "cppAddress"), &address);
				templateGO = (GameObject*)address;
				break;
			}
		}

		if (!templateGO)
		{
			//The user may be trying to instantiate a GameObject created through script. 
			//This feature is not implemented for now.
			CONSOLE_LOG(LogTypes::Error,	"Missing GameObject/MonoObject pair when instantiating from a MonoObject template.\n"
											"Instantiating from a GameObject created through script is not supported for now.\n");
			return nullptr;
		}

		GameObject* newGameObject = App->GOs->Instanciate(templateGO, App->scene->root);
		MonoObject* moInstance = App->scripting->MonoObjectFrom(newGameObject);

		if (position)
		{
			math::float3 newPos{ mono_array_get(position, float, 0), mono_array_get(position, float, 1), mono_array_get(position, float, 2) };
			newGameObject->transform->position = newPos;
		}

		if (rotation)
		{
			math::Quat newRotation{ mono_array_get(position, float, 0), mono_array_get(position, float, 1), mono_array_get(position, float, 2), mono_array_get(position, float, 3) };
			newGameObject->transform->rotation = newRotation;
		}

		App->scripting->ExecuteCallbacks(newGameObject);

		return moInstance;
	}
}

void DestroyObj(MonoObject* obj)
{
	bool found = false;

	std::string className = mono_class_get_name(mono_object_get_class(obj));

	if (className == "GameObject")
	{
		for (int i = 0; i < App->scripting->monoObjectHandles.size(); ++i)
		{
			if (obj == mono_gchandle_get_target(App->scripting->monoObjectHandles[i]))
			{
				found = true;

				MonoClass* monoClass = mono_object_get_class(obj);
				MonoClassField* destroyed = mono_class_get_field_from_name(monoClass, "destroyed");
				mono_field_set_value(obj, destroyed, &found);

				int address;
				mono_field_get_value(obj, mono_class_get_field_from_name(monoClass, "cppAddress"), &address);

				GameObject* toDelete = (GameObject*)address;

				mono_gchandle_free(App->scripting->monoObjectHandles[i]);

				App->scripting->monoObjectHandles.erase(App->scripting->monoObjectHandles.begin() + i);

				//Destroy this GameObject
				App->GOs->DeleteGameObject(toDelete);				

				break;
			}
		}
	}
}

MonoArray* QuatMult(MonoArray* q1, MonoArray* q2)
{
	math::Quat _q1(mono_array_get(q1, float, 0), mono_array_get(q1, float, 1), mono_array_get(q1, float, 2), mono_array_get(q1, float, 3));
	math::Quat _q2(mono_array_get(q2, float, 0), mono_array_get(q2, float, 1), mono_array_get(q2, float, 2), mono_array_get(q2, float, 3));

	_q1.Normalize();
	_q2.Normalize();

	math::Quat result = _q1 * _q2;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);
	mono_array_set(ret, float, 0, result.x);
	mono_array_set(ret, float, 1, result.y);
	mono_array_set(ret, float, 2, result.z);
	mono_array_set(ret, float, 3, result.w);

	return ret;
}

MonoArray* QuatVec3(MonoArray* q, MonoArray* vec)
{
	math::Quat _q(mono_array_get(q, float, 0), mono_array_get(q, float, 1), mono_array_get(q, float, 2), mono_array_get(q, float, 3));
	_q.Normalize();

	math::float3 _vec(mono_array_get(vec, float, 0), mono_array_get(vec, float, 1), mono_array_get(vec, float, 2));

	math::float3 res = _q * _vec;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, res.x);
	mono_array_set(ret, float, 1, res.y);
	mono_array_set(ret, float, 2, res.z);

	return ret;
}

MonoArray* ToEuler(MonoArray* quat)
{
	math::Quat _q(mono_array_get(quat, float, 0), mono_array_get(quat, float, 1), mono_array_get(quat, float, 2), mono_array_get(quat, float, 3));

	math::float3 axis;
	float angle;
	_q.ToAxisAngle(axis, angle);

	math::float3 euler = axis * math::RadToDeg(angle);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, euler.x);
	mono_array_set(ret, float, 1, euler.y);
	mono_array_set(ret, float, 2, euler.z);

	return ret;
}

MonoArray* RotateAxisAngle(MonoArray* axis, float deg)
{
	math::float3 _axis({ mono_array_get(axis, float, 0), mono_array_get(axis, float, 1), mono_array_get(axis, float, 2)});

	float rad = math::DegToRad(deg);

	math::Quat rotation = math::Quat::RotateAxisAngle(_axis, rad);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);
	mono_array_set(ret, float, 0, rotation.x);
	mono_array_set(ret, float, 1, rotation.y);
	mono_array_set(ret, float, 2, rotation.z);
	mono_array_set(ret, float, 3, rotation.w);

	return ret;
}

MonoString* GetGOName(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	return mono_string_new(App->scripting->domain, gameObject->GetName());
}

void SetGOName(MonoObject* monoObject, MonoString* monoString)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return;

	char* newName = mono_string_to_utf8(monoString);
	gameObject->SetName(newName);
	mono_free(newName);
}

float GetDeltaTime()
{
	return App->timeManager->GetDt();
}

float GetRealDeltaTime()
{
	return App->timeManager->GetRealDt();
}

float GetTime()
{
	return App->timeManager->GetGameTime();
}

float GetRealTime()
{
	return App->timeManager->GetRealTime();
}

MonoArray* GetLocalPosition(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);

	mono_array_set(ret, float, 0, gameObject->transform->position.x);
	mono_array_set(ret, float, 1, gameObject->transform->position.y);
	mono_array_set(ret, float, 2, gameObject->transform->position.z);

	return ret;
}

void SetLocalPosition(MonoObject* monoObject, MonoArray* position)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return;

	if (!gameObject->transform)
		return;

	gameObject->transform->position.x = mono_array_get(position, float, 0);
	gameObject->transform->position.y = mono_array_get(position, float, 1);
	gameObject->transform->position.z = mono_array_get(position, float, 2);
}

MonoArray* GetLocalRotation(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);

	mono_array_set(ret, float, 0, gameObject->transform->rotation.x);
	mono_array_set(ret, float, 1, gameObject->transform->rotation.y);
	mono_array_set(ret, float, 2, gameObject->transform->rotation.z);
	mono_array_set(ret, float, 3, gameObject->transform->rotation.w);

	return ret;
}

void SetLocalRotation(MonoObject* monoObject, MonoArray* rotation)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return;

	if (!gameObject->transform)
		return;

	gameObject->transform->rotation.x = mono_array_get(rotation, float, 0);
	gameObject->transform->rotation.y = mono_array_get(rotation, float, 1);
	gameObject->transform->rotation.z = mono_array_get(rotation, float, 2);
	gameObject->transform->rotation.w = mono_array_get(rotation, float, 3);
}

MonoArray* GetLocalScale(MonoObject* monoObject)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);

	mono_array_set(ret, float, 0, gameObject->transform->scale.x);
	mono_array_set(ret, float, 1, gameObject->transform->scale.y);
	mono_array_set(ret, float, 2, gameObject->transform->scale.z);

	return ret;
}

void SetLocalScale(MonoObject* monoObject, MonoArray* scale)
{
	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	GameObject* gameObject = (GameObject*)address;

	if (!gameObject)
		return;

	if (!gameObject->transform)
		return;

	gameObject->transform->scale.x = mono_array_get(scale, float, 0);
	gameObject->transform->position.y = mono_array_get(scale, float, 1);
	gameObject->transform->scale.z = mono_array_get(scale, float, 2);
}

MonoArray* GetGlobalPos(MonoObject* monoObject)
{
	GameObject* gameObject = nullptr;

	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	math::float3 position, scale;
	math::Quat rotation;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, position.x);
	mono_array_set(ret, float, 1, position.y);
	mono_array_set(ret, float, 2, position.z);

	return ret;
}

MonoArray* GetGlobalRotation(MonoObject* monoObject)
{
	GameObject* gameObject = nullptr;

	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	math::float3 position, scale;
	math::Quat rotation;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);
	mono_array_set(ret, float, 0, rotation.x);
	mono_array_set(ret, float, 1, rotation.y);
	mono_array_set(ret, float, 2, rotation.z);
	mono_array_set(ret, float, 3, rotation.w);

	return ret;
}

MonoArray* GetGlobalScale(MonoObject* monoObject)
{
	GameObject* gameObject = nullptr;

	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	if (!gameObject->transform)
		return nullptr;

	math::float3 position, scale;
	math::Quat rotation;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, scale.x);
	mono_array_set(ret, float, 1, scale.y);
	mono_array_set(ret, float, 2, scale.z);

	return ret;
}

void SetGlobalPos(MonoObject* monoObject, MonoArray* globalPos)
{
	math::float3 position, scale, newGlobalPos;
	math::Quat rotation;

	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	if (!gameObject || !gameObject->transform)
		return;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	newGlobalPos = {mono_array_get(globalPos, float, 0), mono_array_get(globalPos, float, 1), mono_array_get(globalPos, float, 2)};

	math::float4x4 newGlobal = math::float4x4::FromTRS(newGlobalPos, rotation, scale);
	gameObject->transform->SetMatrixFromGlobal(newGlobal);
}

void SetGlobalRot(MonoObject* monoObject, MonoArray* globalRot)
{
	math::float3 position, scale;
	math::Quat rotation, newGlobalRot;

	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	if (!gameObject || !gameObject->transform)
		return;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	newGlobalRot = { mono_array_get(globalRot, float, 0), mono_array_get(globalRot, float, 1), mono_array_get(globalRot, float, 2),  mono_array_get(globalRot, float, 3)};

	math::float4x4 newGlobal = math::float4x4::FromTRS(position, newGlobalRot, scale);
	gameObject->transform->SetMatrixFromGlobal(newGlobal);
}

void SetGlobalScale(MonoObject* monoObject, MonoArray* globalScale)
{
	math::float3 position, scale, newGlobalScale;
	math::Quat rotation;

	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	if (!gameObject || !gameObject->transform)
		return;

	math::float4x4 global = gameObject->transform->GetGlobalMatrix();
	global.Decompose(position, rotation, scale);

	newGlobalScale = { mono_array_get(globalScale, float, 0), mono_array_get(globalScale, float, 1), mono_array_get(globalScale, float, 2) };

	math::float4x4 newGlobal = math::float4x4::FromTRS(position, rotation, newGlobalScale);
	gameObject->transform->SetMatrixFromGlobal(newGlobal);
}

MonoObject* GetComponentByType(MonoObject* monoObject, MonoObject* type)
{
	MonoObject* monoComp = nullptr;

	std::string className = mono_class_get_name(mono_object_get_class(type));

	if (className == "NavMeshAgent")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::NavAgentComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "Animator")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::AnimationComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "ParticleEmitter")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::EmitterComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "RectTransform")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::RectTransformComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "Button")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::ButtonComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else if (className == "Image")
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		Component* comp = gameObject->GetComponent(ComponentTypes::ImageComponent);

		if (!comp)
			return nullptr;

		return App->scripting->MonoComponentFrom(comp);
	}
	else
	{
		GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
		if (!gameObject)
			return nullptr;

		//Find a script named as this class

		for (int i = 0; i < gameObject->components.size(); ++i)
		{
			Component* comp = gameObject->components[i];
			if (comp->GetType() == ComponentTypes::ScriptComponent)
			{
				ComponentScript* script = (ComponentScript*)comp;
				if (script->scriptName == className)
				{
					return script->classInstance;
				}
			}
		}
	}

	return monoComp;
}

MonoObject* GetGameCamera()
{
	return App->scripting->MonoComponentFrom(App->renderer3D->GetCurrentCamera());
}

MonoObject* ScreenToRay(MonoArray* screenCoordinates, MonoObject* cameraComponent)
{
	math::float2 screenPoint{ mono_array_get(screenCoordinates, float, 0),mono_array_get(screenCoordinates, float, 1) };

	//Get the camera component attached

	int compAddress;
	mono_field_get_value(cameraComponent, mono_class_get_field_from_name(mono_object_get_class(cameraComponent) , "componentAddress"), &compAddress);

	ComponentCamera* camera = (ComponentCamera*)compAddress;
	if (!camera)
		return nullptr;

	//Get the MathGeoLib Ray
	math::Ray ray = camera->ScreenToRay(screenPoint);
	
	//Create the CSharp Ray object
	MonoClass* RayClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Ray");
	MonoObject* ret = mono_object_new(App->scripting->domain, RayClass);
	mono_runtime_object_init(ret);

	//SetUp the created Ray fields
	MonoClassField* positionField = mono_class_get_field_from_name(RayClass, "position");
	MonoClassField* directionField = mono_class_get_field_from_name(RayClass, "direction");
	
	MonoObject* positionObj; mono_field_get_value(ret, positionField, &positionObj);
	MonoObject* directionObj; mono_field_get_value(ret, directionField, &directionObj);

	mono_field_set_value(positionObj, mono_class_get_field_from_name(mono_object_get_class(positionObj), "_x"), &ray.pos.x);
	mono_field_set_value(positionObj, mono_class_get_field_from_name(mono_object_get_class(positionObj), "_y"), &ray.pos.y);
	mono_field_set_value(positionObj, mono_class_get_field_from_name(mono_object_get_class(positionObj), "_z"), &ray.pos.z);

	mono_field_set_value(directionObj, mono_class_get_field_from_name(mono_object_get_class(directionObj), "_x"), &ray.dir.x);
	mono_field_set_value(directionObj, mono_class_get_field_from_name(mono_object_get_class(directionObj), "_y"), &ray.dir.y);
	mono_field_set_value(directionObj, mono_class_get_field_from_name(mono_object_get_class(directionObj), "_z"), &ray.dir.z);

	return ret;
}

uint LayerToBit(MonoString* layerName)
{
	char* layerCName = mono_string_to_utf8(layerName);

	uint bits = 0; 
	int res = App->layers->NameToNumber(layerCName);
	res != -1 ? bits |= 1 << res : bits = 0;

	mono_free(layerCName);

	return bits;
}

bool Raycast(MonoArray* origin, MonoArray* direction, MonoObject** hitInfo, float maxDistance, uint filterMask, SceneQueryFlags sceneQueryFlags)
{
	math::float3 originCpp{mono_array_get(origin, float, 0), mono_array_get(origin, float, 1), mono_array_get(origin, float, 2)};
	math::float3 directionCpp{mono_array_get(direction, float, 0), mono_array_get(direction, float, 1), mono_array_get(direction, float, 2)};

	RaycastHit hitInfocpp;
	bool ret = App->physics->Raycast(originCpp, directionCpp, hitInfocpp, maxDistance, filterMask, sceneQueryFlags);

	if (ret)
	{
		//Create the HitInfo object
		MonoClass* raycastHitClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "RaycastHit");
		*hitInfo = mono_object_new(App->scripting->domain, raycastHitClass);
		mono_runtime_object_init(*hitInfo);

		//Setup the gameObject and collider fields
		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "gameObject"), App->scripting->MonoObjectFrom(hitInfocpp.GetGameObject()));
		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "collider"), App->scripting->MonoComponentFrom((Component*)hitInfocpp.GetCollider()));

		//Setup the point field
		MonoClass* Vector3Class = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Vector3");
		MonoObject* pointObj = mono_object_new(App->scripting->domain, Vector3Class);
		mono_runtime_object_init(pointObj);

		math::float3 point = hitInfocpp.GetPoint();
		mono_field_set_value(pointObj, mono_class_get_field_from_name(Vector3Class, "_x"), &point.x);
		mono_field_set_value(pointObj, mono_class_get_field_from_name(Vector3Class, "_y"), &point.y);
		mono_field_set_value(pointObj, mono_class_get_field_from_name(Vector3Class, "_z"), &point.z);

		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "point"), pointObj);

		//Setup the normal field
		MonoObject* normalObj = mono_object_new(App->scripting->domain, Vector3Class);
		mono_runtime_object_init(normalObj);

		math::float3 normal = hitInfocpp.GetNormal();
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector3Class, "_x"), &normal.x);
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector3Class, "_y"), &normal.y);
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector3Class, "_z"), &normal.z);

		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "normal"), normalObj);

		//Setup the texCoord field
		MonoClass* Vector2Class = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "Vector2");
		MonoObject* texCoordObj = mono_object_new(App->scripting->domain, Vector2Class);
		mono_runtime_object_init(texCoordObj);

		math::float2 texCoord = hitInfocpp.GetTexCoord();
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector2Class, "x"), &texCoord.x);
		mono_field_set_value(normalObj, mono_class_get_field_from_name(Vector2Class, "y"), &texCoord.y);

		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "texCoord"), texCoordObj);

		//Setup the distance and the faceIndex fields
		float distance = hitInfocpp.GetDistance();
		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "distance"), &distance);

		uint faceIndex = hitInfocpp.GetFaceIndex();
		mono_field_set_value(*hitInfo, mono_class_get_field_from_name(raycastHitClass, "faceIndex"), &faceIndex);
	}
	else
		*hitInfo = nullptr;

	return ret;
}

void SetDestination(MonoObject* navMeshAgent, MonoArray* newDestination)
{
	math::float3 newDestinationcpp(mono_array_get(newDestination, float, 0), mono_array_get(newDestination, float, 1), mono_array_get(newDestination, float, 2));
	int compAddress;
	mono_field_get_value(navMeshAgent, mono_class_get_field_from_name(mono_object_get_class(navMeshAgent), "componentAddress"), &compAddress);
	ComponentNavAgent* agent = (ComponentNavAgent*)compAddress;
	agent->SetDestination(newDestinationcpp.ptr());
}

bool OverlapSphere(float radius, MonoArray* center, MonoArray** overlapHit, uint filterMask, SceneQueryFlags sceneQueryFlags)
{
	math::float3 centercpp(mono_array_get(center, float, 0), mono_array_get(center, float, 1), mono_array_get(center, float, 2));

	std::vector<OverlapHit> hits;
	if (App->physics->OverlapSphere(radius, centercpp, hits, filterMask, sceneQueryFlags))
	{
		uint hitsCount = hits.size();

		MonoClass* overlapClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "OverlapHit");

		*overlapHit = mono_array_new(App->scripting->domain, overlapClass, hitsCount);

		for (int i = 0; i < hitsCount; ++i)
		{
			MonoObject* hitCSharp = mono_object_new(App->scripting->domain, overlapClass);
			mono_runtime_object_init(hitCSharp);

			OverlapHit hitCpp = hits[i];
			mono_field_set_value(hitCSharp, mono_class_get_field_from_name(overlapClass, "gameObject"), App->scripting->MonoObjectFrom(hitCpp.GetGameObject()));
			mono_field_set_value(hitCSharp, mono_class_get_field_from_name(overlapClass, "collider"), App->scripting->MonoComponentFrom((Component*)hitCpp.GetCollider()));

			uint faceIndex = hitCpp.GetFaceIndex();
			mono_field_set_value(hitCSharp, mono_class_get_field_from_name(overlapClass, "faceIndex"), &faceIndex);

			mono_array_setref(*overlapHit, i, hitCSharp);
		}

		return true;
	}
	else
		return false;
}

void SetCompActive(MonoObject* monoComponent, bool active)
{
	Component* component = App->scripting->ComponentFrom(monoComponent);
	component->IsActive() != active ? component->ToggleIsActive() : void();
}

void SetGameObjectActive(MonoObject* monoObject, bool active)
{
	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	gameObject->IsActive() != active ? gameObject->ToggleIsActive() : void();
}

bool GetGameObjectActive(MonoObject* monoObject, bool active)
{
	GameObject* gameObject = App->scripting->GameObjectFrom(monoObject);
	return gameObject->IsActive();
}

bool PlayAnimation(MonoObject* animatorComp, MonoString* animUUID)
{
	char* anim = mono_string_to_utf8(animUUID);

	ComponentAnimation* animator = (ComponentAnimation*)App->scripting->ComponentFrom(animatorComp);
	bool ret = animator->PlayAnimation(anim);
	
	mono_free(anim);

	return ret;
}

void ParticleEmitterPlay(MonoObject* particleComp)
{
	ComponentEmitter* emitter = (ComponentEmitter*)App->scripting->ComponentFrom(particleComp);
	if (emitter)
	{
		emitter->StartEmitter();
	}
}

void ParticleEmitterStop(MonoObject* particleComp)
{
	ComponentEmitter* emitter = (ComponentEmitter*)App->scripting->ComponentFrom(particleComp);
	if (emitter)
	{
		emitter->ClearEmitter();
	}
}

bool UIHovered()
{
	return App->ui->IsUIHovered();
}

MonoArray* RectTransform_GetRect(MonoObject* rectComp)
{
	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_uint32_class(), 4);

	ComponentRectTransform* rectCpp = (ComponentRectTransform*)App->scripting->ComponentFrom(rectComp);
	if (!rectCpp)
		return nullptr;

	uint* rectVector = rectCpp->GetRect();
	mono_array_set(ret, uint, 0, rectVector[0]);
	mono_array_set(ret, uint, 1, rectVector[1]);
	mono_array_set(ret, uint, 2, rectVector[2]);
	mono_array_set(ret, uint, 3, rectVector[3]);

	return ret;
}

void RectTransform_SetRect(MonoObject* rectComp, MonoArray* newRect)
{
	ComponentRectTransform* rectCpp = (ComponentRectTransform*)App->scripting->ComponentFrom(rectComp);
	if (!rectCpp)
		return;

	uint rectVector[4];
	rectVector[0] = mono_array_get(newRect, uint, 0);
	rectVector[1] = mono_array_get(newRect, uint, 1);
	rectVector[2] = mono_array_get(newRect, uint, 2);
	rectVector[3] = mono_array_get(newRect, uint, 3);
	
	rectCpp->SetRect(rectVector[0], rectVector[1], rectVector[2], rectVector[3]);
}

void ButtonSetKey(MonoObject* buttonComp, uint key)
{
	ComponentButton* button = (ComponentButton*)App->scripting->ComponentFrom(buttonComp);
	if (button)
	{
		button->SetNewKey(key);
	}
}

int ButtonGetState(MonoObject* buttonComp)
{
	ComponentButton* button = (ComponentButton*)App->scripting->ComponentFrom(buttonComp);
	if (button)
	{
		return button->GetState();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void ScriptingModule::CreateDomain()
{
	static bool firstDomain = true;

	MonoDomain* nextDom = mono_domain_create_appdomain("The reloaded domain", NULL);
	if (!nextDom)
		return;

	if (!mono_domain_set(nextDom, false))
		return;
	
	//Make sure we do not delete the main domain
	if (!firstDomain)
	{
		mono_domain_unload(domain);		
	}

	domain = nextDom;

	char* buffer;
	int size = App->fs->Load("JellyBitCS.dll", &buffer);
	if(size <= 0)
		return;

	//Loading assemblies from data instead of from file
	MonoImageOpenStatus status = MONO_IMAGE_ERROR_ERRNO;
	internalImage = mono_image_open_from_data(buffer, size, 1, &status);
	internalAssembly = mono_assembly_load_from(internalImage, "InternalAssembly", &status);

	delete[] buffer;

	//SetUp Internal Calls
	mono_add_internal_call("JellyBitEngine.Debug::Log", (const void*)&DebugLogTranslator);
	mono_add_internal_call("JellyBitEngine.Debug::LogWarning", (const void*)&DebugLogWarningTranslator);
	mono_add_internal_call("JellyBitEngine.Debug::LogError", (const void*)&DebugLogErrorTranslator);
	mono_add_internal_call("JellyBitEngine.Debug::ClearConsole", (const void*)&ClearConsole);
	mono_add_internal_call("JellyBitEngine.GameObject::_Instantiate", (const void*)&InstantiateGameObject);
	mono_add_internal_call("JellyBitEngine.Input::GetKeyState", (const void*)&GetKeyStateCS);
	mono_add_internal_call("JellyBitEngine.Input::GetMouseButtonState", (const void*)&GetMouseStateCS);
	mono_add_internal_call("JellyBitEngine.Input::GetMousePos", (const void*)&GetMousePosCS);
	mono_add_internal_call("JellyBitEngine.Input::GetWheelMovement", (const void*)&GetWheelMovementCS);
	mono_add_internal_call("JellyBitEngine.Input::GetMouseDeltaPos", (const void*)&GetMouseDeltaPosCS);
	mono_add_internal_call("JellyBitEngine.Object::Destroy", (const void*)&DestroyObj);
	mono_add_internal_call("JellyBitEngine.Quaternion::quatMult", (const void*)&QuatMult);
	mono_add_internal_call("JellyBitEngine.Quaternion::quatVec3", (const void*)&QuatVec3);
	mono_add_internal_call("JellyBitEngine.Quaternion::toEuler", (const void*)&ToEuler);
	mono_add_internal_call("JellyBitEngine.Quaternion::RotateAxisAngle", (const void*)&RotateAxisAngle);
	mono_add_internal_call("JellyBitEngine.GameObject::getName", (const void*)&GetGOName);
	mono_add_internal_call("JellyBitEngine.GameObject::setName", (const void*)&SetGOName);
	mono_add_internal_call("JellyBitEngine.Time::getDeltaTime", (const void*)&GetDeltaTime);
	mono_add_internal_call("JellyBitEngine.Time::getRealDeltaTime", (const void*)&GetRealDeltaTime);
	mono_add_internal_call("JellyBitEngine.Time::getTime", (const void*)&GetTime);
	mono_add_internal_call("JellyBitEngine.Time::getRealTime", (const void*)&GetRealTime);
	mono_add_internal_call("JellyBitEngine.Transform::getLocalPosition", (const void*)&GetLocalPosition);
	mono_add_internal_call("JellyBitEngine.Transform::setLocalPosition", (const void*)&SetLocalPosition);
	mono_add_internal_call("JellyBitEngine.Transform::getLocalRotation", (const void*)&GetLocalRotation);
	mono_add_internal_call("JellyBitEngine.Transform::setLocalRotation", (const void*)&SetLocalRotation);
	mono_add_internal_call("JellyBitEngine.Transform::getLocalScale", (const void*)&GetLocalScale);
	mono_add_internal_call("JellyBitEngine.Transform::setLocalScale", (const void*)&SetLocalScale);
	mono_add_internal_call("JellyBitEngine.Transform::getGlobalPos", (const void*)&GetGlobalPos);
	mono_add_internal_call("JellyBitEngine.Transform::getGlobalRotation", (const void*)&GetGlobalRotation);
	mono_add_internal_call("JellyBitEngine.Transform::getGlobalScale", (const void*)&GetGlobalScale);
	mono_add_internal_call("JellyBitEngine.Transform::setGlobalPos", (const void*)&SetGlobalPos);
	mono_add_internal_call("JellyBitEngine.Transform::setGlobalRotation", (const void*)&SetGlobalRot);
	mono_add_internal_call("JellyBitEngine.Transform::setGlobalScale", (const void*)&SetGlobalScale);
	mono_add_internal_call("JellyBitEngine.GameObject::GetComponentByType", (const void*)&GetComponentByType);
	mono_add_internal_call("JellyBitEngine.Camera::getMainCamera", (const void*)&GetGameCamera);
	mono_add_internal_call("JellyBitEngine.Physics::_ScreenToRay", (const void*)&ScreenToRay);
	mono_add_internal_call("JellyBitEngine.LayerMask::GetMaskBit", (const void*)&LayerToBit);
	mono_add_internal_call("JellyBitEngine.Physics::_Raycast", (const void*)&Raycast);
	mono_add_internal_call("JellyBitEngine.NavMeshAgent::_SetDestination", (const void*)&SetDestination);
	mono_add_internal_call("JellyBitEngine.Physics::_OverlapSphere", (const void*)&OverlapSphere);
	mono_add_internal_call("JellyBitEngine.Component::SetActive", (const void*)&SetCompActive);
	mono_add_internal_call("JellyBitEngine.Animator::PlayAnimation", (const void*)&PlayAnimation);
	mono_add_internal_call("JellyBitEngine.ParticleEmitter::Play", (const void*)&ParticleEmitterPlay);
	mono_add_internal_call("JellyBitEngine.ParticleEmitter::Stop", (const void*)&ParticleEmitterStop);
	mono_add_internal_call("JellyBitEngine.UI.UI::UIHovered", (const void*)&UIHovered);
	mono_add_internal_call("JellyBitEngine.UI.RectTransform::GetRect", (const void*)&RectTransform_GetRect);
	mono_add_internal_call("JellyBitEngine.UI.RectTransform::SetRect", (const void*)&RectTransform_SetRect);
	mono_add_internal_call("JellyBitEngine.UI.Button::SetKey", (const void*)&ButtonSetKey);
	mono_add_internal_call("JellyBitEngine.UI.Button::GetState", (const void*)&ButtonGetState);
	mono_add_internal_call("JellyBitEngine.GameObject::GetActive", (const void*)&GetGameObjectActive);
	mono_add_internal_call("JellyBitEngine.GameObject::SetActive", (const void*)&SetGameObjectActive);

	ClearMap();

	firstDomain = false;
}

void ScriptingModule::UpdateScriptingReferences()
{
	char* buffer;
	int size = App->fs->Load("Library/Scripts/Scripting.dll", &buffer);
	if (size <= 0)
		return;

	//Loading assemblies from data instead of from file
	MonoImageOpenStatus status = MONO_IMAGE_ERROR_ERRNO;
	scriptsImage = mono_image_open_from_data(buffer, size, 1, &status);
	scriptsAssembly = mono_assembly_load_from(scriptsImage, "ScriptingAssembly", &status);

	delete[] buffer;
}
