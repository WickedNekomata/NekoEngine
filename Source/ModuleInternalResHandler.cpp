#include "ModuleInternalResHandler.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"

bool ModuleInternalResHandler::Start()
{
	memset(this, 0, sizeof(ModuleInternalResHandler));
	return true;
}

void ModuleInternalResHandler::CreatePlane()
{
	ResourceMeshData specificData;

	uint verticesSize = 4;
	Vertex* vertices = new Vertex[verticesSize];

	float verticesPosition[12]
	{
		-0.5f, -0.5f, 0.0f, // a
		 0.5f, -0.5f, 0.0f, // b
		-0.5f,  0.5f, 0.0f, // c
		 0.5f,  0.5f, 0.0f, // d
	};

	uint textCordsSize = 8;
	float textCordsPosition[8]
	{
		 0.0f,  0.0f, // a
		 1.0f,  0.0f, // b
		 0.0f,  1.0f, // c
		 1.0f,  1.0f, // d
	};

	uint normalSize = 12;
	float normalPosition[12]
	{
		0.0f, 1.0f, 0.0f, // a
		0.0f, 1.0f, 0.0f, // a
		0.0f, 1.0f, 0.0f, // a
		0.0f, 1.0f, 0.0f, // a
	};

	specificData.indicesSize = 6;
	specificData.indices = new uint[specificData.indicesSize];
	uint indices[]
	{
		// Front
		0, 1, 2, // ABC
		1, 3, 2, // BDC
	};

	memcpy(&specificData, indices, sizeof(int) * specificData.indicesSize);

	// Vertices
	/// Position
	float* cursor = verticesPosition;
	for (uint i = 0; i < verticesSize / 3; ++i)
	{
		memcpy(vertices[i].position, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	///Normals
	cursor = normalPosition;
	for (uint i = 0; i < normalSize / 3; ++i)
	{
		memcpy(vertices[i].normal, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	///TextCords
	cursor = textCordsPosition;
	for (uint i = 0; i < textCordsSize / 2; ++i)
	{
		memcpy(vertices[i].texCoord, cursor, sizeof(float) * 2);
		cursor += 2;
	}

	ResourceData data;
	data.name = "Default Plane";

	defaultPlane = App->res->CreateResource(ResourceTypes::MeshResource, data, &specificData)->GetUuid();
}

void ModuleInternalResHandler::CreateCube()
{
	ResourceMeshData specificData;

	uint verticesSize = 8;
	Vertex* vertices = new Vertex[verticesSize];

	float verticesPosition[24]
	{
		-1, -1, -1,
		 1, -1, -1,
		 1,  1, -1,
		-1,  1, -1,
		-1, -1,  1,
		 1, -1,  1,
		 1,  1,  1,
		-1,  1,  1
	};

	int texCoordsSize = 12;
	float texCoords[12]
	{
		0, 0,
		1, 0,
		0, 1,
		0, 1,
		1, 0,
		0, 1
	};

	int normalSize = 18;
	float normalPosition[18]
	{
		 0,  0,  1,
		 1,  0,  0,
		 0,  0, -1,
		-1,  0,  0,
		 0,  1,  0,
		 0, -1,  0
	};

	specificData.indicesSize = 36;
	specificData.indices = new uint[specificData.indicesSize];
	uint indices[]
	{
		0, 1, 3, 3, 1, 2,
		1, 5, 2, 2, 5, 6,
		5, 4, 6, 6, 4, 7,
		4, 0, 7, 7, 0, 3,
		3, 2, 7, 7, 2, 6,
		4, 5, 0, 0, 5, 1
	};

	memcpy(&specificData, indices, sizeof(int) * specificData.indicesSize);

	// Vertices
	/// Position
	float* cursor = verticesPosition;
	for (uint i = 0; i < verticesSize / 3; ++i)
	{
		memcpy(vertices[i].position, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	///Normals
	cursor = normalPosition;
	for (uint i = 0; i < normalSize / 3; ++i)
	{
		memcpy(vertices[i].normal, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	///TextCords
	cursor = texCoords;
	for (uint i = 0; i < texCoordsSize / 2; ++i)
	{
		memcpy(vertices[i].texCoord, cursor, sizeof(float) * 2);
		cursor += 2;
	}

	ResourceData data;
	data.name = "Default Cube";

	defaultCube = App->res->CreateResource(ResourceTypes::MeshResource, data, &specificData)->GetUuid();
}

void ModuleInternalResHandler::CreateDefaultShaderProgram()
{
	ResourceData data;
	ResourceShaderObjectData ObjspecificData;
	data.name = "Default vertex object";
	ObjspecificData.shaderType = ShaderTypes::VertexShaderType;
	ObjspecificData.SetSource(vShaderTemplate, strlen(vShaderTemplate));
	ResourceShaderObject* vObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, data, &ObjspecificData);
	data.name = "Default fragment object";
	ObjspecificData.shaderType = ShaderTypes::FragmentShaderType;
	ObjspecificData.SetSource(fShaderTemplate, strlen(fShaderTemplate));
	ResourceShaderObject* fObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, data, &ObjspecificData);
	data.name = "Shader vertex object";
	ResourceShaderProgramData programData;
	programData.shaderObjects.push_back(vObj);
	programData.shaderObjects.push_back(fObj);
	App->res->CreateResource(ResourceTypes::ShaderProgramResource, data, &programData);
}

void ModuleInternalResHandler::CreateCubemapShaderProgram()
{
}
