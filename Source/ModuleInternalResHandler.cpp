#include "ModuleInternalResHandler.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "MaterialImporter.h"

#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"
#include "ResourceShaderObject.h"
#include "ResourceShaderProgram.h"

bool ModuleInternalResHandler::Start()
{
	CreatePlane();
	CreateCube();
	CreateDefaultShaderProgram(vShaderTemplate, fShaderTemplate, ShaderProgramTypes::Standard);
	CreateDefaultShaderProgram(Particle_vShaderTemplate, Particle_fShaderTemplate, ShaderProgramTypes::Particles);
	CreateDefaultMaterial();

	return true;
}

void ModuleInternalResHandler::CreatePlane()
{
	ResourceMeshData specificData;

	specificData.verticesSize = 4;
	specificData.vertices = new Vertex[specificData.verticesSize];

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
	specificData.indices = new uint[specificData.indicesSize]
	{
		// Front
		2, 1, 0, // ABC
		2, 3, 1, // BDC
	};

	// Vertices
	/// Position
	float* cursor = verticesPosition;
	for (uint i = 0; i < specificData.verticesSize; ++i)
	{
		memcpy(specificData.vertices[i].position, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	///Normals
	cursor = normalPosition;
	for (uint i = 0; i < normalSize / 3; ++i)
	{
		memcpy(specificData.vertices[i].normal, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	///TextCords
	cursor = textCordsPosition;
	for (uint i = 0; i < textCordsSize / 2; ++i)
	{
		memcpy(specificData.vertices[i].texCoord, cursor, sizeof(float) * 2);
		cursor += 2;
	}

	ResourceData data;
	data.name = "Default Plane";

	plane = App->res->CreateResource(ResourceTypes::MeshResource, data, &specificData, PLANE_UUID)->GetUuid();
}

void ModuleInternalResHandler::CreateCube()
{
	ResourceMeshData specificData;

	specificData.verticesSize = 8;
	specificData.vertices = new Vertex[specificData.verticesSize];

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

	specificData.indicesSize = 36;
	specificData.indices = new uint[specificData.indicesSize]
	{
		0, 1, 3, 3, 1, 2,
		1, 5, 2, 2, 5, 6,
		5, 4, 6, 6, 4, 7,
		4, 0, 7, 7, 0, 3,
		3, 2, 7, 7, 2, 6,
		4, 5, 0, 0, 5, 1
	};

	// Vertices
	/// Position
	float* cursor = verticesPosition;
	for (uint i = 0; i < specificData.verticesSize; ++i)
	{
		memcpy(specificData.vertices[i].position, cursor, sizeof(float) * 3);
		cursor += 3;
	}

	ResourceData data;
	data.name = "Default Cube";

	cube = App->res->CreateResource(ResourceTypes::MeshResource, data, &specificData, CUBE_UUID)->GetUuid();
}

void ModuleInternalResHandler::CreateDefaultShaderProgram(const char* vShader, const char* fShader, ShaderProgramTypes type)
{
	ResourceData vertexData;
	ResourceShaderObjectData vertexShaderData;
	vertexData.name = "Default vertex object";
	vertexShaderData.shaderObjectType = ShaderObjectTypes::VertexType;
	vertexShaderData.SetSource(vShader, strlen(vShader));
	ResourceShaderObject* vObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &vertexShaderData);
	if (!vObj->Compile())
		vObj->isValid = false;

	ResourceData fragmentData;
	ResourceShaderObjectData fragmentShaderData;
	fragmentData.name = "Default fragment object";
	fragmentShaderData.shaderObjectType = ShaderObjectTypes::FragmentType;
	fragmentShaderData.SetSource(fShader, strlen(fShader));
	ResourceShaderObject* fObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &fragmentShaderData);
	if (!fObj->Compile())
		fObj->isValid = false;

	ResourceData shaderData;
	ResourceShaderProgramData programShaderData;
	shaderData.name = "Default shader program";
	programShaderData.shaderObjects.push_back(vObj);
	programShaderData.shaderObjects.push_back(fObj);
	programShaderData.shaderProgramType = type;
	ResourceShaderProgram* prog = nullptr;
	if(type == ShaderProgramTypes::Standard)
		prog = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData, DEFAULT_SHADER_PROGRAM_UUID);
	else if (type == ShaderProgramTypes::Particles)
		prog = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData, DEFAULT_SHADER_PROGRAM_PARTICLE_UUID);

	if (!prog->Link())
		prog->isValid = false;
	if (type == ShaderProgramTypes::Standard)
		defaultShaderProgram = prog->GetUuid();
	else if (type == ShaderProgramTypes::Particles)
		defaultParticleShaderProgram = prog->GetUuid();

}

void ModuleInternalResHandler::CreateCubemapShaderProgram()
{
	// CUBEMAP_SHADER_PROGRAM_UUID
}

void ModuleInternalResHandler::CreateUIShaderProgram()
{
	ResourceData vertexData;
	ResourceShaderObjectData vertexShaderData;
	vertexData.name = "UI vertex object";
	vertexShaderData.shaderObjectType = ShaderObjectTypes::VertexType;
	vertexShaderData.SetSource(uivShader, strlen(uivShader));
	ResourceShaderObject* vObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &vertexShaderData);
	if (vObj->Compile())
		vObj->isValid = false;
	UIVertexShaderObject = vObj->shaderObject;

	ResourceData fragmentData;
	ResourceShaderObjectData fragmentShaderData;
	fragmentData.name = "UI fragment object";
	fragmentShaderData.shaderObjectType = ShaderObjectTypes::FragmentType;
	fragmentShaderData.SetSource(uifShader, strlen(uifShader));
	ResourceShaderObject* fObj = (ResourceShaderObject*)App->res->CreateResource(ResourceTypes::ShaderObjectResource, vertexData, &fragmentShaderData);
	if (fObj->Compile())
		fObj->isValid = false;
	UIFragmentShaderObject = fObj->shaderObject;

	ResourceData shaderData;
	ResourceShaderProgramData programShaderData;
	shaderData.name = "UI shader program";
	programShaderData.shaderObjects.push_back(vObj);
	programShaderData.shaderObjects.push_back(fObj);
	ResourceShaderProgram* pShader = (ResourceShaderProgram*)App->res->CreateResource(ResourceTypes::ShaderProgramResource, shaderData, &programShaderData);
	if (pShader->Link())
		pShader->isValid = false;
	UIShaderProgram = pShader->shaderProgram;
}

void ModuleInternalResHandler::CreateDefaultMaterial()
{
	ResourceData data;
	ResourceMaterialData materialData;
	data.name = "Default material";
	materialData.shaderUuid = DEFAULT_SHADER_PROGRAM_UUID;
	((ResourceShaderProgram*)App->res->GetResource(materialData.shaderUuid))->GetUniforms(materialData.uniforms);
	for (uint i = 0; i < materialData.uniforms.size(); ++i)
	{
		Uniform& uniform = materialData.uniforms[i];
		switch (uniform.common.type)
		{
		case Uniforms_Values::Sampler2U_value:
		{
			if (strcmp(uniform.common.name, "material.albedo") == 0)
				uniform.sampler2DU.value.id = App->materialImporter->GetDefaultTexture();
		}
		break;
		}
	}

	defaultMaterial = App->res->CreateResource(ResourceTypes::MaterialResource, data, &materialData, DEFAULT_MATERIAL_UUID)->GetUuid();
}
