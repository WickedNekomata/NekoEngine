#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"

#include "glew\include\GL\glew.h"

struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat tangent[3];
	GLfloat bitangent[3];
	GLubyte color[4];
	GLfloat texCoord[2];
};

class ResourceMesh : public Resource
{
public:

	ResourceMesh(ResourceType type, uint uuid);
	~ResourceMesh();

	void GetIndices(int* indices) const;
	void GetVerts(float* verts) const;
	void GetNormals(float* normals) const;
	int  GetVertsCount() const;
	int  GetIndicesCount() const;
	uint GetVBO() const;
	uint GetIBO() const;
	uint GetVAO() const;

private:

	bool LoadInMemory();
	bool UnloadFromMemory();

	void GenerateVBO();
	void GenerateIBO();
	void GenerateVAO();

public: //TODO: set this to private and fix errors

	Vertex* vertices = nullptr;
	uint verticesSize = 0;

	GLuint* indices = nullptr;
	uint indicesSize = 0;

	GLuint VBO = 0;
	GLuint IBO = 0;
	GLuint VAO = 0;
};

#endif