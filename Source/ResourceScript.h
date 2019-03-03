#ifndef __RESOURCESCRIPT_H__
#define __RESOURCESCRIPT_H__

#include "Resource.h"
#include "Globals.h"

struct _MonoMethod;
struct _MonoImage;
struct _MonoAssembly;

struct ResourceScriptData
{
	
};

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
	TRANSFORM,
	LAYERMASK, 
	ENUM
};

class ResourceScript : public Resource
{
public:

	ResourceScript(uint uuid, ResourceData data, ResourceScriptData scriptData);
	virtual ~ResourceScript();

	bool LoadInMemory() override { return true; }
	bool UnloadFromMemory() override { return true; }

	void OnPanelAssets();

	bool GenerateLibraryFiles() const;

public:

	void SerializeToMeta(char*& cursor) const;
	void DeSerializeFromMeta(char*& cursor);
	uint bytesToSerializeMeta() const;

	bool referenceMethods();

	static std::vector<std::string> getScriptNames() { return scriptNames; }

private:
	std::string pathToWindowsNotation(const std::string& path) const;
	uint getBytes() const;

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

private:
	static std::vector<std::string> scriptNames;

	ResourceScriptData scriptData;
};

#endif