#ifndef __RESOURCEPREFAB_H__
#define __RESOURCEPREFAB_H__

#include "Resource.h"

class GameObject;

struct PrefabData
{
	GameObject* root = nullptr;
	char* buffer;
	uint size;
};

class ResourcePrefab : public Resource
{
public:
	ResourcePrefab(ResourceTypes type, uint uuid, ResourceData data, PrefabData customData);
	~ResourcePrefab();

	void OnPanelAssets();

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, PrefabData& prefabData, std::string& outputFile, bool overwrite = false);
	static uint CreateMeta(const char* file, uint prefab_uuid, std::string& name, std::string& outputMetaFile);
	static bool ReadMeta(const char* metaFile, int64_t& lastModTime, uint& prefab_uuid, std::string& name);
	static bool LoadFile(const char* file, PrefabData& prefab_data_output);

	bool LoadInMemory();
	bool UnloadFromMemory();

	//tmp
};

#endif