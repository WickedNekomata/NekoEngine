#ifndef __RESOURCESCRIPT_H__
#define __RESOURCESCRIPT_H__

#include "Resource.h"

struct _MonoMethod;
struct _MonoImage;
struct _MonoAssembly;

enum class VarType
{
	NO_TYPE = -1,
	BOOL,
	FLOAT, 
	DOUBLE,
	INT8,
	UINT8,
	INT16,
	UINT16,
	INT,
	UINT,
	INT64,
	UINT64,
	CHAR,
	STRING,
	GAMEOBJECT,
	TRANSFORM
};

class ResourceScript : public Resource
{
public:

	enum class ScriptState
	{
		NO_STATE = -1,
		NO_COMPILED,
		COMPILED_WITH_ERRORS,
		COMPILED_FINE
		
	} state = ScriptState::NO_COMPILED;

	ResourceScript() : Resource(ResourceType::ScriptResource, App->GenerateRandomNumber()) {}
	virtual ~ResourceScript();

	bool LoadInMemory() override { return true; }
	bool UnloadFromMemory() override { return true; }

public:
	void SerializeToMeta(char*& cursor) const;
	void DeSerializeFromMeta(char*& cursor);
	uint bytesToSerializeMeta() const;

	bool preCompileErrors();
	bool Compile();
	bool referenceMethods();

private:
	std::string pathToWindowsNotation(const std::string& path) const;

public:

	std::string scriptName;

	//Callback methods references
	_MonoMethod* awakeMethod = nullptr;
	_MonoMethod* startMethod = nullptr;
	_MonoMethod* preUpdateMethod = nullptr;
	_MonoMethod* updateMethod = nullptr;
	_MonoMethod* postUpdateMethod = nullptr;
	_MonoMethod* enableMethod = nullptr;
	_MonoMethod* disableMethod = nullptr;
	_MonoMethod* stopMethod = nullptr;

	//The assembly and image containing all the .cs code
	_MonoAssembly* assembly = nullptr;
	_MonoImage* image = nullptr;

private:
	bool firstCompiled = true;
	uint getBytes() const;
};

#endif