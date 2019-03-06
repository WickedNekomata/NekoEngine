#ifndef __RESOURCESCENE_H__
#define __RESOURCESCENE_H__

#include "Resource.h"

struct SceneData
{
	
};

class ResourceScene : public Resource
{
public:
	ResourceScene(uint uuid, ResourceData data, SceneData customData);
	~ResourceScene();

	void OnPanelAssets();
	bool GenerateLibraryFiles() const;

	static ResourceScene* ImportFile(const char* file);
	static ResourceScene* ExportFile(const char* sceneName);
	static bool CreateMeta(ResourceScene* scene, int64_t lastModTime);
	bool UpdateFromMeta();

	static uint GetMetaSize() { return sizeof(int64_t) + sizeof(uint) * 2; }

	bool LoadInMemory();
	bool UnloadFromMemory();

	//REMEMBER TO DELETE THE MEMORY AFTER USING THE BUFFER
	uint GetSceneBuffer(char*& buffer) const;

private:
	SceneData sceneData;
};

#endif