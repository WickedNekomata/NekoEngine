#ifndef __RESOURCEPREFAB_H__
#define __RESOURCEPREFAB_H__

#include "Resource.h"

class GameObject;

struct PrefabData
{
	GameObject* root = nullptr;
};

class ResourcePrefab : public Resource
{
public:
	ResourcePrefab(uint uuid, ResourceData data, PrefabData customData);
	~ResourcePrefab();

	void OnPanelAssets();

	static bool ImportFile(const char* file, std::string& name, std::string& outputFile);
	static bool ExportFile(ResourceData& data, PrefabData& prefabData, std::string& outputFile, bool overwrite = false);

	bool LoadInMemory();
	bool UnloadFromMemory();

};

#endif