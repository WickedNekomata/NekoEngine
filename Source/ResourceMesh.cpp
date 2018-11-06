#include "ResourceMesh.h"
#include "Application.h"

#include "glew/include/GL/glew.h"

ResourceMesh::ResourceMesh(ResourceType type, uint uuid) : Resource(type, uuid)
{
}

ResourceMesh::~ResourceMesh()
{
}

void ResourceMesh::OnUniqueEditor()
{
}

bool ResourceMesh::LoadInMemory()
{
	App->sceneImporter->Load(exportedFileName.data(), this);

	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize * 3, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, (GLuint*)&textureCoordsID);
	glBindBuffer(GL_ARRAY_BUFFER, textureCoordsID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize * 2, textureCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

bool ResourceMesh::UnloadFromMemory()
{
	glDeleteBuffers(1, (GLuint*)&verticesID);
	glDeleteBuffers(1, (GLuint*)&indicesID);
	glDeleteBuffers(1, (GLuint*)&textureCoordsID);

	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(indices);
	RELEASE_ARRAY(textureCoords);

	return true;
}
