#ifndef __SCRIPTINGMODULE_H__
#define __SCRIPTINGMODULE_H__

#include "Module.h"

#include <string>
#include <vector>
#include <map>

#include "pugui/pugixml.hpp"

#include <mono/metadata/object.h>

class ComponentScript;
class ResourceScript;

struct Directory;

bool exec(const char* cmd, std::string& error = std::string());

class ScriptingModule : public Module
{
	friend MonoObject* InstantiateGameObject(MonoObject* templateMO, MonoArray* position, MonoArray* rotation);
	friend MonoObject* GetComponentByType(MonoObject* monoObject, MonoObject* type);

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
	void ClearScriptComponent(ComponentScript* script);

	MonoObject* MonoObjectFrom(GameObject* gameObject);
	GameObject* GameObjectFrom(MonoObject* monoObject);

	MonoObject* MonoComponentFrom(Component* component);
	Component* ComponentFrom(MonoObject* monoComponent);

	bool alreadyCreated(std::string scriptName);

	void CreateScriptingProject();
	void ExecuteScriptingProject();
	void IncludeCSFiles();
	void IncludeCSFiles(pugi::xml_node& nodeToAppend, const Directory& dir);

	void CreateInternalCSProject();

	std::string getReferencePath()const;
	
	std::string clearSpaces(std::string& scriptName = std::string());

	void CreateDomain();
	void UpdateScriptingReferences();

	void ReInstance();

	void ClearMap();

	Resource* ImportScriptResource(const char* fileAssets);
	void ScriptModified(const char* scriptPath);
	void RecompileScripts();

	void GameObjectKilled(GameObject* killed);

private:
	void UpdateMethods();
	void ExecuteCallbacks(GameObject* gameObject);

public:
	_MonoDomain*			domain				= nullptr;
	_MonoAssembly*			internalAssembly	= nullptr;
	_MonoImage*				internalImage		= nullptr;

	_MonoAssembly*			scriptsAssembly		= nullptr;
	_MonoImage*				scriptsImage		= nullptr;

	std::vector<uint32_t>	monoObjectHandles;
	std::vector<uint32_t>	monoComponentHandles;

private:

	bool someScriptModified = false;
	bool engineOpened = true;
	std::vector<ComponentScript*> scripts;
};

#endif