#include "ScriptingModule.h"
#include "ComponentScript.h"
#include "ResourceScript.h"
#include "ComponentTransform.h"
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

bool ScriptingModule::Init()
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

	//TODO: MAKE ALL CSHARP TIME STUFF PROPERTIES WITH GET AND SET, AND ASK FOR THE C++ REAL VALUES INSTEAD OF UPDATING COPIES.

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
		delete scripts[i];
	}
	scripts.clear();

	mono_jit_cleanup(domain);
	domain = nullptr;

	return true;
}

void ScriptingModule::ReceiveEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::Play:
		{
			//Check if some files have compile errors and don't let the user hit the play.

			UpdateMonoObjects();

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
		
			UpdateGameObjects();

			//Call the Awake and Start for all the Enabled script in the Play instant.
			break;
		}
	
		case System_Event_Type::Stop:
		{
			UpdateMonoObjects();

			for (int i = 0; i < scripts.size(); ++i)
			{
				if (scripts[i]->IsTreeActive())
				{
					scripts[i]->OnStop();
				}
			}

			UpdateGameObjects();

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
			for (int i = 0; i < gameObjectsMap.size(); ++i)
			{
				GameObject* gameObject = gameObjectsMap[i].first;
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
					MonoObject* monoObject = mono_gchandle_get_target(gameObjectsMap[i].second);
					MonoClass* monoObjectClass = mono_object_get_class(monoObject);

					MonoClassField* deletedField = mono_class_get_field_from_name(monoObjectClass, "destroyed");

					bool temp = true;
					mono_field_set_value(monoObject, deletedField, &temp);

					mono_gchandle_free(gameObjectsMap[i].second);

					gameObjectsMap.erase(gameObjectsMap.begin() + i);
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
		App->fs->OpenRead("Internal/SampleScript/SampleScript.cs", &buffer, size);

		std::string scriptStream = buffer;
		scriptStream.resize(size);

		while (scriptStream.find("SampleScript") != std::string::npos)
		{
			scriptStream = scriptStream.replace(scriptStream.find("SampleScript"), 12, scriptName);
		}

		App->fs->OpenWriteBuffer("Assets/Scripts/" + scriptName + ".cs", (char*)scriptStream.c_str(), scriptStream.size());

		IncludeCSFiles();

		delete buffer;
	}

	ResourceScript* scriptRes = (ResourceScript*)App->resources->FindByFile("Assets/Scripts/" + scriptName + ".cs");
	if (!scriptRes)
	{
		//Here we have to reference a new ResourceScript with the .cs we have created, but the ResourceManager will still be sending file created events, and we would have data duplication.
		//We disable this behavior and control the script creation only with this method, so we do not care for external files out-of-engine created.
		scriptRes = new ResourceScript();
		scriptRes->setFile("Assets/Scripts/" + scriptName + ".cs");
		scriptRes->scriptName = scriptName;

		//Create the .meta, to make faster the search in the map storing the uid.
		uint bytes = scriptRes->bytesToSerializeMeta();
		char* buffer = new char[bytes];
		char* cursor = buffer;
		scriptRes->SerializeToMeta(cursor);

		App->fs->OpenWriteBuffer("Assets/Scripts/" + scriptName + ".cs.meta", buffer, bytes);

		delete buffer;

		scriptRes->Compile();
		App->resources->PushResourceScript(scriptRes);
	}

	script->scriptRes = scriptRes;

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

MonoObject* ScriptingModule::MonoObjectFrom(GameObject* gameObject)
{
	for (int i = 0; i < gameObjectsMap.size(); ++i)
	{
		if (gameObjectsMap[i].first == gameObject)
		{
			MonoObject* ret = mono_gchandle_get_target(gameObjectsMap[i].second);
			GameObjectChanged(gameObject);
			return ret;
		}
	}

	MonoClass* gameObjectClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "GameObject");
	MonoObject* monoInstance = mono_object_new(App->scripting->domain, gameObjectClass);
	mono_runtime_object_init(monoInstance);

	uint32_t handleID = mono_gchandle_new(monoInstance, true);

	App->scripting->gameObjectsMap.push_back(std::pair<GameObject*, uint32_t>(gameObject, handleID));

	App->scripting->GameObjectChanged(gameObject);

	return monoInstance;
}

GameObject* ScriptingModule::GameObjectFrom(_MonoObject* monoObject)
{
	for (int i = 0; i < gameObjectsMap.size(); ++i)
	{
		uint32_t handleID = gameObjectsMap[i].second;

		if (mono_gchandle_get_target(handleID) == monoObject)
		{
			GameObject* ret = gameObjectsMap[i].first;
			return ret;
		}
	}
	return nullptr;
}

void ScriptingModule::GameCameraChanged()
{
	GameObject* mainCamera = App->camera->gameCamera;
	if (mainCamera == nullptr)
	{
		MonoClass* cameraClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Camera");
		MonoClassField* mainCamField = mono_class_get_field_from_name(cameraClass, "main");
		MonoVTable* cameraClassvTable = mono_class_vtable(App->scripting->domain, cameraClass);
		mono_field_static_set_value(cameraClassvTable, mainCamField, NULL);
		return;
	}

	for (int i = 0; i < App->scripting->gameObjectsMap.size(); ++i)
	{
		if (App->scripting->gameObjectsMap[i].first == mainCamera)
		{
			MonoObject* monoObject = mono_gchandle_get_target(App->scripting->gameObjectsMap[i].second);

			MonoClass* cameraClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Camera");
			MonoClassField* mainCamField = mono_class_get_field_from_name(cameraClass, "main");
			MonoVTable* cameraClassvTable = mono_class_vtable(App->scripting->domain, cameraClass);

			mono_field_static_set_value(cameraClassvTable, mainCamField, monoObject);

			return;
		}
	}

	//This gameObject is not saved in the map
	MonoObject* monoObject = MonoObjectFrom(mainCamera);
	MonoClass* cameraClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Camera");
	MonoClassField* mainCamField = mono_class_get_field_from_name(cameraClass, "main");
	MonoVTable* cameraClassvTable = mono_class_vtable(App->scripting->domain, cameraClass);
	mono_field_static_set_value(cameraClassvTable, mainCamField, monoObject);
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
	Directory scripts = App->fs->getDirFiles("Assets/Scripts");

	//Modify the project settings file, stored in a xml

	char* buffer;
	int size;
	if (!App->fs->OpenRead("Assembly-CSharp.csproj", &buffer, size))
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
	App->fs->OpenWriteBuffer("Assembly-CSharp.csproj", (char*)stream.str().data(), stream.str().size());	
}

void ScriptingModule::IncludeCSFiles(pugi::xml_node& nodeToAppend, const Directory& dir)
{
	for (int i = 0; i < dir.files.size(); ++i)
	{
		if (App->fs->getExt(dir.files[i].name) != ".cs")
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
	if (App->fs->Exists("FlanCS"))
		return;

	App->fs->CopyDirectoryAndContentsInto("Internal/FlanCS", "", true);
}

std::string ScriptingModule::getReferencePath() const
{
	return std::string("-r:") + std::string("\"") + App->fs->getAppPath() + std::string("FlanCS.dll\" ");
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

void ScriptingModule::UpdateMonoObjects()
{
	for (int i = 0; i < App->scripting->gameObjectsMap.size(); ++i)
	{
		App->scripting->GameObjectChanged(App->scripting->gameObjectsMap[i].first);
	}
}

void ScriptingModule::GameObjectChanged(GameObject* gameObject)
{
	MonoObject* monoObject = nullptr;
	//Find for a pair coincidence in the map
	for (int i = 0; i < gameObjectsMap.size(); ++i)
	{
		if (gameObjectsMap[i].first == gameObject)
		{
			monoObject = mono_gchandle_get_target(gameObjectsMap[i].second);

			if (!monoObject)
				continue;

			//SetUp all the GameObject* fields to the MonoObject

			MonoClass* gameObjectClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "GameObject");

			//SetUp the name
			MonoClassField* name = mono_class_get_field_from_name(gameObjectClass, "name");
			MonoString* nameCS = mono_string_new(App->scripting->domain, gameObject->name.data());
			mono_field_set_value(monoObject, name, (void*)nameCS);

			//SetUp the transform
			MonoClassField* transformField = mono_class_get_field_from_name(gameObjectClass, "transform");
			MonoClass* transformClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Transform");

			MonoClassField* positionField = mono_class_get_field_from_name(transformClass, "position");
			MonoClassField* rotationField = mono_class_get_field_from_name(transformClass, "rotation");
			MonoClassField* scaleField = mono_class_get_field_from_name(transformClass, "scale");

			MonoClass* vector3Class = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Vector3");
			MonoClass* quaternionClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Quaternion");
			
			MonoClassField* vector3_x_Field = mono_class_get_field_from_name(vector3Class, "x");
			MonoClassField* vector3_y_Field = mono_class_get_field_from_name(vector3Class, "y");
			MonoClassField* vector3_z_Field = mono_class_get_field_from_name(vector3Class, "z");

			MonoClassField* quat_x_Field = mono_class_get_field_from_name(quaternionClass, "x");
			MonoClassField* quat_y_Field = mono_class_get_field_from_name(quaternionClass, "y");
			MonoClassField* quat_z_Field = mono_class_get_field_from_name(quaternionClass, "z");
			MonoClassField* quat_w_Field = mono_class_get_field_from_name(quaternionClass, "w");

			MonoObject* transformOBJ;
			mono_field_get_value(monoObject, transformField, &transformOBJ);

			MonoObject* positionOBJ; mono_field_get_value(transformOBJ, positionField, &positionOBJ);
			mono_field_set_value(positionOBJ, vector3_x_Field, &gameObject->transform->position.x);
			mono_field_set_value(positionOBJ, vector3_y_Field, &gameObject->transform->position.y);
			mono_field_set_value(positionOBJ, vector3_z_Field, &gameObject->transform->position.z);

			MonoObject* rotationOBJ; mono_field_get_value(transformOBJ, rotationField, &rotationOBJ);
			mono_field_set_value(rotationOBJ, quat_x_Field, &gameObject->transform->rotation.x);
			mono_field_set_value(rotationOBJ, quat_y_Field, &gameObject->transform->rotation.y);
			mono_field_set_value(rotationOBJ, quat_z_Field, &gameObject->transform->rotation.z);
			mono_field_set_value(rotationOBJ, quat_w_Field, &gameObject->transform->rotation.w);

			MonoObject* scaleOBJ; mono_field_get_value(transformOBJ, scaleField, &scaleOBJ);
			mono_field_set_value(scaleOBJ, vector3_x_Field, &gameObject->transform->scale.x);
			mono_field_set_value(scaleOBJ, vector3_y_Field, &gameObject->transform->scale.y);
			mono_field_set_value(scaleOBJ, vector3_z_Field, &gameObject->transform->scale.z);

			//TODO: CONTINUE UPDATING THINGS
			break;
		}
	}
}

void ScriptingModule::UpdateGameObjects()
{
	for (int i = 0; i < App->scripting->gameObjectsMap.size(); ++i)
	{
		App->scripting->MonoObjectChanged(App->scripting->gameObjectsMap[i].second);
	}
}

void ScriptingModule::MonoObjectChanged(uint32_t handleID)
{
	//Find for a coincidence in the map.

	GameObject* gameObject = nullptr;
	for (int i = 0; i < gameObjectsMap.size(); ++i)
	{
		if (gameObjectsMap[i].second == handleID)
		{
			MonoObject* monoObject = mono_gchandle_get_target(handleID);
			if (!monoObject)
				continue;

			gameObject = gameObjectsMap[i].first;

			//SetUp the name
			MonoClass* gameObjectClass = mono_class_from_name(internalImage, "FlanEngine", "GameObject");
			MonoClassField* nameCS = mono_class_get_field_from_name(gameObjectClass, "name");

			MonoString* name = (MonoString*)mono_field_get_value_object(domain, nameCS, monoObject);
			char* newName = mono_string_to_utf8(name);
			gameObject->name = newName;

			//SetUp the transform
			MonoClassField* transformField = mono_class_get_field_from_name(gameObjectClass, "transform");
			MonoClass* transformClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Transform");

			MonoClassField* positionField = mono_class_get_field_from_name(transformClass, "position");
			MonoClassField* rotationField = mono_class_get_field_from_name(transformClass, "rotation");
			MonoClassField* scaleField = mono_class_get_field_from_name(transformClass, "scale");

			MonoClass* vector3Class = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Vector3");
			MonoClass* quaternionClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "Quaternion");

			MonoClassField* vector3_x_Field = mono_class_get_field_from_name(vector3Class, "x");
			MonoClassField* vector3_y_Field = mono_class_get_field_from_name(vector3Class, "y");
			MonoClassField* vector3_z_Field = mono_class_get_field_from_name(vector3Class, "z");

			MonoClassField* quat_x_Field = mono_class_get_field_from_name(quaternionClass, "x");
			MonoClassField* quat_y_Field = mono_class_get_field_from_name(quaternionClass, "y");
			MonoClassField* quat_z_Field = mono_class_get_field_from_name(quaternionClass, "z");
			MonoClassField* quat_w_Field = mono_class_get_field_from_name(quaternionClass, "w");

			MonoObject* transformOBJ; 
			mono_field_get_value(monoObject, transformField, &transformOBJ);

			MonoObject* positionOBJ;
			mono_field_get_value(transformOBJ, positionField, &positionOBJ);

			mono_field_get_value(positionOBJ, vector3_x_Field, &gameObject->transform->position.x);
			mono_field_get_value(positionOBJ, vector3_y_Field, &gameObject->transform->position.y);
			mono_field_get_value(positionOBJ, vector3_z_Field, &gameObject->transform->position.z);

			MonoObject* rotationOBJ;
			mono_field_get_value(transformOBJ, rotationField, &rotationOBJ);

			mono_field_get_value(rotationOBJ, quat_x_Field, &gameObject->transform->rotation.x);
			mono_field_get_value(rotationOBJ, quat_y_Field, &gameObject->transform->rotation.y);
			mono_field_get_value(rotationOBJ, quat_z_Field, &gameObject->transform->rotation.z);
			mono_field_get_value(rotationOBJ, quat_w_Field, &gameObject->transform->rotation.w);

			MonoObject* scaleOBJ;
			mono_field_get_value(transformOBJ, scaleField, &scaleOBJ);

			mono_field_get_value(scaleOBJ, vector3_x_Field, &gameObject->transform->scale.x);
			mono_field_get_value(scaleOBJ, vector3_y_Field, &gameObject->transform->scale.y);
			mono_field_get_value(scaleOBJ, vector3_z_Field, &gameObject->transform->scale.z);

			//TODO: CONTINUE UPDATING THINGS
		}
	}
}

void ScriptingModule::ClearMap()
{
	for (int i = 0; i < gameObjectsMap.size(); ++i)
	{
		mono_gchandle_free(gameObjectsMap[i].second);
	}
	gameObjectsMap.clear();
}

void ScriptingModule::UpdateMethods()
{
	for (int i = 0; i < gameObjectsMap.size(); ++i)
	{
		GameObjectChanged(gameObjectsMap[i].first);
	}
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

	for (int i = 0; i < gameObjectsMap.size(); ++i)
	{
		MonoObjectChanged(gameObjectsMap[i].second);
	}
}

//-----------------------------

void DebugLogTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	Debug.Log(string);

	mono_free(string);
}

void DebugLogWarningTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	Debug.LogWarning(string);

	mono_free(string);
}

void DebugLogErrorTranslator(MonoString* msg)
{
	MonoError error;
	char* string = mono_string_to_utf8_checked(msg, &error);

	if (!mono_error_ok(&error))
		return;

	Debug.LogError(string);

	mono_free(string);
}

void ClearConsole() { Debug.Clear(); }

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

		GameObject* instance = App->scene->CreateGameObject(App->scene->getRootNode(), false);

		MonoClass* gameObjectClass = mono_class_from_name(App->scripting->internalImage, "FlanEngine", "GameObject");
		MonoObject* monoInstance = mono_object_new(App->scripting->domain, gameObjectClass);
		mono_runtime_object_init(monoInstance);

		uint32_t handleID = mono_gchandle_new(monoInstance, true);

		App->scripting->gameObjectsMap.push_back(std::pair<GameObject*, uint32_t>(instance, handleID));

		App->scripting->GameObjectChanged(instance);

		return monoInstance;
	}

	else
	{
		//Search for the monoTemplate and his GameObject representation in the map, create 2 new copies,
		//add the GameObject to the Scene Hierarchy and returns the monoObject. Store this new Instantiated objects in the map.

		GameObject* templateGO = nullptr;

		for (int i = 0; i < App->scripting->gameObjectsMap.size(); ++i)
		{
			uint32_t handleID = App->scripting->gameObjectsMap[i].second;
			MonoObject* temp = mono_gchandle_get_target(handleID);

			if (temp == templateMO)
			{
				templateGO = App->scripting->gameObjectsMap[i].first;
				break;
			}
		}

		if (!templateGO)
		{
			//The user may be trying to instantiate a GameObject created through script. 
			//This feature is not implemented for now.
			Debug.LogError(	"Missing GameObject/MonoObject pair when instantiating from a MonoObject template.\n"
							"Instantiating from a GameObject created through script is not supported for now.\n");
			return nullptr;
		}

		GameObject* goInstance = new GameObject(App->scene->getRootNode());
		App->scene->AddGameObject(goInstance);

		*goInstance = *templateGO;
		goInstance->ReGenerate();
		goInstance->initAABB();
		goInstance->transformAABB();

		goInstance->parent = App->scene->getRootNode();

		//App->scene->UpdateQuadtree();

		MonoObject* moInstance = App->scripting->MonoObjectFrom(goInstance);

		goInstance->InstantiateEvents();

		return moInstance;
	}
}

void DestroyObj(MonoObject* obj)
{
	bool found = false;
	for (int i = 0; i < App->scripting->gameObjectsMap.size(); ++i)
	{
		if (obj == mono_gchandle_get_target(App->scripting->gameObjectsMap[i].second))
		{
			found = true;

			MonoClass* monoClass = mono_object_get_class(obj);
			MonoClassField* destroyed = mono_class_get_field_from_name(monoClass, "destroyed");
			mono_field_set_value(obj, destroyed, &found);

			GameObject* toDelete = App->scripting->gameObjectsMap[i].first;

			mono_gchandle_free(App->scripting->gameObjectsMap[i].second);

			App->scripting->gameObjectsMap.erase(App->scripting->gameObjectsMap.begin() + i);

			App->scene->DestroyGameObject(toDelete);

			break;
		}
	}
}

MonoArray* QuatMult(MonoArray* q1, MonoArray* q2)
{
	Quat _q1(mono_array_get(q1, float, 0), mono_array_get(q1, float, 1), mono_array_get(q1, float, 2), mono_array_get(q1, float, 3));
	Quat _q2(mono_array_get(q2, float, 0), mono_array_get(q2, float, 1), mono_array_get(q2, float, 2), mono_array_get(q2, float, 3));

	_q1.Normalize();
	_q2.Normalize();

	Quat result = _q1 * _q2;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);
	mono_array_set(ret, float, 0, result.x);
	mono_array_set(ret, float, 1, result.y);
	mono_array_set(ret, float, 2, result.z);
	mono_array_set(ret, float, 3, result.w);

	return ret;
}

MonoArray* QuatVec3(MonoArray* q, MonoArray* vec)
{
	Quat _q(mono_array_get(q, float, 0), mono_array_get(q, float, 1), mono_array_get(q, float, 2), mono_array_get(q, float, 3));
	_q.Normalize();

	float3 _vec(mono_array_get(vec, float, 0), mono_array_get(vec, float, 1), mono_array_get(vec, float, 2));

	float3 res = _q * _vec;

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, res.x);
	mono_array_set(ret, float, 1, res.y);
	mono_array_set(ret, float, 2, res.z);

	return ret;
}

MonoArray* ToEuler(MonoArray* quat)
{
	Quat _q(mono_array_get(quat, float, 0), mono_array_get(quat, float, 1), mono_array_get(quat, float, 2), mono_array_get(quat, float, 3));

	float3 axis;
	float angle;
	_q.ToAxisAngle(axis, angle);

	float3 euler = axis * RadToDeg(angle);

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, euler.x);
	mono_array_set(ret, float, 1, euler.y);
	mono_array_set(ret, float, 2, euler.z);

	return ret;
}

MonoArray* RotateAxisAngle(MonoArray* axis, float deg)
{
	float3 _axis({ mono_array_get(axis, float, 0), mono_array_get(axis, float, 1), mono_array_get(axis, float, 2)});

	float rad = DegToRad(deg);

	Quat rotation = Quat::RotateAxisAngle(_axis, rad);

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

	for (int i = 0; i < App->scripting->gameObjectsMap.size(); ++i)
	{
		uint32_t handleID = App->scripting->gameObjectsMap[i].second;		

		if (monoObject == mono_gchandle_get_target(handleID))
		{
			gameObject = App->scripting->gameObjectsMap[i].first;
		}
	}

	if (!gameObject)
		return nullptr;

	ComponentTransform global = gameObject->transform->getGlobal();

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 3);
	mono_array_set(ret, float, 0, global.position.x);
	mono_array_set(ret, float, 1, global.position.y);
	mono_array_set(ret, float, 2, global.position.z);

	return ret;
}

MonoArray* GetGlobalRotation(MonoObject* monoObject)
{
	GameObject* gameObject = nullptr;

	for (int i = 0; i < App->scripting->gameObjectsMap.size(); ++i)
	{
		uint32_t handleID = App->scripting->gameObjectsMap[i].second;
		if (mono_gchandle_get_target(handleID) == monoObject)
		{
			gameObject = App->scripting->gameObjectsMap[i].first;
		}
	}

	if (!gameObject)
		return nullptr;

	ComponentTransform global = gameObject->transform->getGlobal();

	MonoArray* ret = mono_array_new(App->scripting->domain, mono_get_int32_class(), 4);
	mono_array_set(ret, float, 0, global.rotation.x);
	mono_array_set(ret, float, 1, global.rotation.y);
	mono_array_set(ret, float, 2, global.rotation.z);
	mono_array_set(ret, float, 3, global.rotation.w);

	return ret;
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
	int size;
	if (!App->fs->OpenRead("FlanCS.dll", &buffer, size, false))
		return;

	//Loading assemblies from data instead of from file
	MonoImageOpenStatus status = MONO_IMAGE_ERROR_ERRNO;
	internalImage = mono_image_open_from_data(buffer, size, 1, &status);
	internalAssembly = mono_assembly_load_from(internalImage, "InternalAssembly", &status);

	delete buffer;

	timeClass = mono_class_from_name(internalImage, "FlanEngine", "Time");
	deltaTime = mono_class_get_field_from_name(timeClass, "deltaTime");
	realDeltaTime = mono_class_get_field_from_name(timeClass, "realDeltaTime");
	time = mono_class_get_field_from_name(timeClass, "time");
	realTime = mono_class_get_field_from_name(timeClass, "realTime");
	timeVTable = mono_class_vtable(domain, timeClass);

	//SetUp Internal Calls
	mono_add_internal_call("FlanEngine.Debug::Log", (const void*)&DebugLogTranslator);
	mono_add_internal_call("FlanEngine.Debug::LogWarning", (const void*)&DebugLogWarningTranslator);
	mono_add_internal_call("FlanEngine.Debug::LogError", (const void*)&DebugLogErrorTranslator);
	mono_add_internal_call("FlanEngine.Debug::ClearConsole", (const void*)&ClearConsole);
	mono_add_internal_call("FlanEngine.GameObject::Instantiate", (const void*)&InstantiateGameObject);
	mono_add_internal_call("FlanEngine.Input::GetKeyState", (const void*)&GetKeyStateCS);
	mono_add_internal_call("FlanEngine.Input::GetMouseButtonState", (const void*)&GetMouseStateCS);
	mono_add_internal_call("FlanEngine.Input::GetMousePos", (const void*)&GetMousePosCS);
	mono_add_internal_call("FlanEngine.Input::GetWheelMovement", (const void*)&GetWheelMovementCS);
	mono_add_internal_call("FlanEngine.Input::GetMouseDeltaPos", (const void*)&GetMouseDeltaPosCS);
	mono_add_internal_call("FlanEngine.Object::Destroy", (const void*)&DestroyObj);
	mono_add_internal_call("FlanEngine.Quaternion::quatMult", (const void*)&QuatMult);
	mono_add_internal_call("FlanEngine.Quaternion::quatVec3", (const void*)&QuatVec3);
	mono_add_internal_call("FlanEngine.Quaternion::toEuler", (const void*)&ToEuler);
	mono_add_internal_call("FlanEngine.Quaternion::RotateAxisAngle", (const void*)&RotateAxisAngle);
	mono_add_internal_call("FlanEngine.Transform::getGlobalPos", (const void*)&GetGlobalPos);
	mono_add_internal_call("FlanEngine.Transform::getGlobalRotation", (const void*)&GetGlobalRotation);

	ClearMap();

	firstDomain = false;
}
