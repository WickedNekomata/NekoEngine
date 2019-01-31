#ifndef __SCRIPTINGMODULE_H__
#define __SCRIPTINGMODULE_H__

#include "Module.h"

#include <string>
#include <vector>
#include <map>

#include "pugui/pugixml.hpp"

class ComponentScript;
struct _MonoDomain;
struct _MonoAssembly;
class ResourceScript;
struct _MonoObject;
struct _MonoImage;
struct _MonoClass;
struct _MonoClassField;
struct MonoVTable;
struct Directory;


bool exec(const char* cmd, std::string& error = std::string());

class ScriptingModule : public Module
{
public:
	ScriptingModule(bool start_enabled = true) : Module(start_enabled) { name = "ScriptingModule"; }
	~ScriptingModule() {}

	bool Init(JSON_Object* data) override;
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void OnSystemEvent(System_Event event) override;

public:
	ComponentScript* CreateScriptComponent(std::string scriptName, bool createCS = true);
	bool DestroyScript(ComponentScript* script);
	inline void AddScriptComponent(ComponentScript* script){scripts.push_back(script);}
	_MonoObject* MonoObjectFrom(GameObject* gameObject);
	GameObject* GameObjectFrom(_MonoObject* monoObject);
	void GameCameraChanged();

	bool alreadyCreated(std::string scriptName);

	void CreateScriptingProject();
	void ExecuteScriptingProject();
	void IncludeCSFiles();
	void IncludeCSFiles(pugi::xml_node& nodeToAppend, const Directory& dir);

	void CreateInternalCSProject();

	std::string getReferencePath()const;
	
	std::string clearSpaces(std::string& scriptName = std::string());

	void CreateDomain();
	void ReInstance();

	void UpdateMonoObjects();
	void GameObjectChanged(GameObject* gameObject);

	void UpdateGameObjects();
	void MonoObjectChanged(uint32_t handleID);

	void ClearMap();

	bool ImportScriptResource(const char* fileAssets, const char* metaFile, const char* exportedFile);

private:
	void UpdateMethods();

public:
	_MonoDomain* domain = nullptr;
	_MonoAssembly* internalAssembly = nullptr;
	_MonoImage* internalImage = nullptr;

	//The relationship between the actual GameObjects and their CSharp representation
	std::vector<std::pair<GameObject*, uint32_t>> gameObjectsMap;

private:
	std::vector<ComponentScript*> scripts;

	//TODO: DELETE THOSE
	MonoVTable* timeVTable = nullptr;
	_MonoClassField* deltaTime = nullptr;
	_MonoClassField* realDeltaTime = nullptr;
	_MonoClassField* time = nullptr;
	_MonoClassField* realTime = nullptr;
	_MonoClass* timeClass = nullptr;
};

#endif