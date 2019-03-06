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
	bool GenerateLibraryFiles() const;

	static ResourcePrefab* ImportFile(const char* file);
	static ResourcePrefab* ExportFile(const char* prefabName, GameObject* templateRoot);
	static bool CreateMeta(ResourcePrefab* prefab, int64_t lastModTime);
	bool UpdateFromMeta();
	bool UpdateRoot();

	static uint GetMetaSize() { return sizeof(int64_t) + sizeof(uint) * 2; }

	bool LoadInMemory();
	bool UnloadFromMemory();

	inline GameObject* GetRoot() { return prefabData.root; }

private:
	PrefabData prefabData;
};

#endif