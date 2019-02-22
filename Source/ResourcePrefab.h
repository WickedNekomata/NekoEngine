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
	ResourcePrefab(uint uuid, ResourceData data, PrefabData customData);
	~ResourcePrefab();

	void OnPanelAssets();

	bool LoadInMemory();
	bool UnloadFromMemory();

};

#endif