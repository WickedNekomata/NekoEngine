#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"

class ResourceMesh : public Resource
{
public:

	ResourceMesh(ResourceType type, uint uuid);
	virtual ~ResourceMesh();

private:

	virtual void OnUniqueEditor();

	virtual bool LoadInMemory();
	virtual bool UnloadFromMemory();

private:

	float* vertices = nullptr;
	uint verticesSize = 0;
	uint verticesID = 0;

	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;

	float* textureCoords = nullptr;
	uint textureCoordsID = 0;
	uint textureCoordsSize = 0;
};


#endif