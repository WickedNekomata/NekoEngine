#ifndef __RESOURCEPREFAB_H__
#define __RESOURCEPREFAB_H__

#include "Resource.h"

class GameObject;

struct PrefabData
{
};

class ResourcePrefab : public Resource
{
	ResourcePrefab(uint uuid, ResourceData data, PrefabData customData);
	~ResourcePrefab();

	void OnPanelAssets();

	bool LoadInMemory();
	bool UnloadFromMemory();

private:
	GameObject* root = nullptr;
};

#endif