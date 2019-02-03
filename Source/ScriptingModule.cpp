#include "ScriptingModule.h"
#include "ComponentScript.h"
#include "ResourceScript.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ModuleInput.h"

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

#include "MathGeoLib/include/MathGeoLib.h"

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

	char* args[1];
	args[0] == "InternalAssembly";
	mono_jit_exec(domain, internalAssembly, 1, args);

	if (!internalAssembly)
		return false;

	return true;
}

bool ScriptingModule::Start()
{
	CreateScriptingProject();
	IncludeCSFiles();
	return true;
}

update_status ScriptingModule::PreUpdate()
{
	if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		CreateInternalCSProject();

	return UPDATE_CONTINUE;
}

update_status ScriptingModule::Update()
{
	if (App->GetEngineState() == engine_states::ENGINE_PLAY)
	{
		UpdateMethods();
	}

	return UPDATE_CONTINUE;
}

update_status ScriptingModule::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ScriptingModule::CleanUp()
{
	for (int i = 0; i < scripts.size(); ++i)
	{
		scripts[i]->GetParent()->ClearComponent(scripts[i]);
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
			//Check if some files have compile errors and don't let the user hit the play.

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

		case System_Event_Type::ResourceDestroyed:
		{		
			for (int i = 0; i < scripts.size(); ++i)
			{
				bool somethingDestroyed = false;
				if (scripts[i]->scriptRes == event.resEvent.resource)
				{
					somethingDestroyed = true;
					scripts[i]->GetParent()->ClearComponent(scripts[i]);
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
					gameObject = gameObject->GetParent();
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

		uint32_t UID;
		memcpy(&UID, metaBuffer, sizeof(uint32_t));

		scriptRes = (ResourceScript*)App->res->GetResource(UID);

		delete[] metaBuffer;
	}
	
	if (!scriptRes)
	{
		//Here we have to reference a new ResourceScript with the .cs we have created, but the ResourceManager will still be sending file created events, and we would have data duplication.
		//We disable this behavior and control the script creation only with this method, so we do not care for external files out-of-engine created.
		scriptRes = new ResourceScript();
		scriptRes->file = "Assets/Scripts/" + scriptName + ".cs";
		scriptRes->scriptName = scriptName;

		//Create the .meta, to make faster the search in the map storing the uid.
		uint bytes = scriptRes->bytesToSerializeMeta();
		char* buffer = new char[bytes];
		char* cursor = buffer;
		scriptRes->SerializeToMeta(cursor);

		App->fs->Save("Assets/Scripts/" + scriptName + ".cs.meta", buffer, bytes);

		delete[] buffer;

		scriptRes->Compile();

		App->res->InsertResource(scriptRes);
	}

	App->res->SetAsUsed(scriptRes->GetUUID());
	script->scriptRes = scriptRes;

	scripts.push_back(script);

	App->fs->AddMeta(std::string("Assets/Scripts/" + scriptName + ".cs.meta").data(), App->fs->GetLastModificationTime(std::string("Assets/Scripts/" + scriptName + ".cs.meta").data()));

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
	return std::string("-r:") + std::string("\"") + App->fs->getAppPath() + std::string("JellyBitCS.dll\" ");
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

bool ScriptingModule::ImportScriptResource(const char* fileAssets, const char* metaFile, const char* exportedFile)
{
	std::string file = fileAssets;

	std::string scriptName = file.substr(file.find_last_of("/") + 1);
	scriptName = scriptName.substr(0, scriptName.find_last_of("."));

	//Creating script resource
	ResourceScript* scriptRes = new ResourceScript();
	scriptRes->scriptName = scriptName;
	scriptRes->file = file;

	if (!metaFile)
	{
		//Create the .meta
		uint bytes = scriptRes->bytesToSerializeMeta();
		char* buffer = new char[bytes];
		char* cursor = buffer;
		scriptRes->SerializeToMeta(cursor);

		App->fs->Save(file + ".meta", buffer, bytes);

		delete[] buffer;
	}
	else
	{
		char* metaBuffer;
		uint size = App->fs->Load(metaFile, &metaBuffer);
		if (size > 0)
		{
			char* cursor = metaBuffer;
			scriptRes->DeSerializeFromMeta(cursor);
			delete[] metaBuffer;
		}
	}

	if (!exportedFile)
		scriptRes->Compile();
	else
	{
		scriptRes->referenceMethods();
		scriptRes->exportedFile = exportedFile;
	}
		
	App->res->InsertResource(scriptRes);

	return true;
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

MonoObject* InstantiateGameObject(MonoObject* templateMO)
{
	if (!templateMO)
	{
		//Instantiate an empty GameObject and returns the MonoObject

		GameObject* instance = App->GOs->CreateGameObject("default", App->GOs->getRoot());

		MonoClass* gameObjectClass = mono_class_from_name(App->scripting->internalImage, "JellyBitEngine", "GameObject");
		MonoObject* monoInstance = mono_object_new(App->scripting->domain, gameObjectClass);
		mono_runtime_object_init(monoInstance);

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

		//TODO: IMPLEMENT THE NEW PREFAB / INSTANTIATION SYSTEM

		/*GameObject* goInstance = new GameObject("default", App->GOs->getRoot());

		App->GOs->AddGameObject(goInstance);*/

		//*goInstance = *templateGO;
		//goInstance->ReGenerate();
		//goInstance->initAABB();
		//goInstance->transformAABB();

		//goInstance->parent = App->scene->getRootNode();

		////App->scene->UpdateQuadtree();

		//MonoObject* moInstance = App->scripting->MonoObjectFrom(goInstance);

		//goInstance->InstantiateEvents();

		//return moInstance;
		
		//Temp
		return nullptr;
	}
}

void DestroyObj(MonoObject* obj)
{
	bool found = false;
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

			//Send the event to destroy this gameObject

			System_Event event;
			event.goEvent.type = System_Event_Type::GameObjectDestroyed;
			event.goEvent.gameObject = toDelete;
			App->PushSystemEvent(event);

			break;
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

MonoArray* GetGlobalPos(MonoObject* monoObject)
{
	GameObject* gameObject = nullptr;

	int address;
	mono_field_get_value(monoObject, mono_class_get_field_from_name(mono_object_get_class(monoObject), "cppAddress"), &address);

	gameObject = (GameObject*)address;

	if (!gameObject)
		return nullptr;

	math::float3 position, scale;
	math::Quat rotation;

	gameObject->transform->GetGlobalMatrix().Decompose(position, rotation, scale);

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

	math::float3 position, scale;
	math::Quat rotation;

	gameObject->transform->GetGlobalMatrix().Decompose(position, rotation, scale);

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

	gameObject->transform->scale.x = mono_array_get(scale, float, 0);
	gameObject->transform->position.y = mono_array_get(scale, float, 1);
	gameObject->transform->scale.z = mono_array_get(scale, float, 2);
}

//---------------------------------

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
	mono_add_internal_call("JellyBitEngine.GameObject::Instantiate", (const void*)&InstantiateGameObject);
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
	mono_add_internal_call("JellyBitEngine.Transform::getGlobalPos", (const void*)&GetGlobalPos);
	mono_add_internal_call("JellyBitEngine.Transform::getGlobalRotation", (const void*)&GetGlobalRotation);
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

	ClearMap();

	firstDomain = false;
}
