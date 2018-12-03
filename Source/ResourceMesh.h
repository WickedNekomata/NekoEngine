#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"

#include "glew\include\GL\glew.h"

struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLubyte color[4];
	GLfloat texCoord[4];
};

class ResourceMesh : public Resource
{
public:

	ResourceMesh(ResourceType type, uint uuid);
	~ResourceMesh();

private:

	bool LoadInMemory();
	bool UnloadFromMemory();

	void GenerateVBO();
	void GenerateIBO();
	void GenerateVAO();

public:

	const char* name = nullptr;

	Vertex* vertices = nullptr;
	uint verticesSize = 0;

	uint* indices = nullptr;
	uint indicesSize = 0;

	GLuint VBO = 0;
	GLuint IBO = 0;
	GLuint VAO = 0;
};

#endif