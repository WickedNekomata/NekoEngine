#include "SceneImporter.h"

#include "Application.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentTypes.h"
#include "MaterialImporter.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"
#include "Assimp/include/version.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

void myCallback(const char* msg, char* userData)
{
	CONSOLE_LOG("%s", msg);
}

SceneImporter::SceneImporter()
{
	struct aiLogStream stream;
	stream.callback = myCallback;
	aiAttachLogStream(&stream);
}

SceneImporter::~SceneImporter()
{
	aiDetachAllLogStreams();
}

bool SceneImporter::Import(const char* importFileName, const char* importPath, std::string& outputFileName)
{
	bool ret = false;

	if (importPath == nullptr)
		return ret;

	char importFilePath[DEFAULT_BUF_SIZE];
	strcpy_s(importFilePath, strlen(importPath) + 1, importPath);
	if (importFileName != nullptr)
		strcat_s(importFilePath, strlen(importFilePath) + strlen(importFileName) + 1, importFileName);
	outputFileName = App->filesystem->GetFileNameFromPath(importFilePath);

	char* buffer;
	uint size = App->filesystem->Load(importFilePath, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded mesh(es) %s (original format)", importFileName);
		ret = Import(buffer, size, outputFileName);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SCENE IMPORTER: Could not load mesh(es) %s (original format)", importFileName);

	return ret;
}

bool SceneImporter::Import(const void* buffer, uint size, std::string& outputFileName)
{
	bool ret = false;

	if (buffer == nullptr || size <= 0)
		return ret;

	uint postProcessingFlags = 0;
	postProcessingFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_Triangulate;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;

	const aiScene* scene = aiImportFileFromMemory((const char*)buffer, size, postProcessingFlags, nullptr);

	if (scene != nullptr)
	{
		ret = true;

		const aiNode* rootNode = scene->mRootNode;
		RecursivelyImportNodes(scene, rootNode, App->scene->root, outputFileName);

		aiReleaseImport(scene);
	}

	return ret;
}

void SceneImporter::RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parentGO, std::string& outputFileName)
{
	GameObject* go = App->GOs->CreateGameObject((char*)node->mName.data, (GameObject*)parentGO);

	// Transform
	aiVector3D position;
	aiVector3D scale;
	aiQuaternion rotation;
	node->mTransformation.Decompose(scale, rotation, position);

	go->transform->position = { position.x, position.y, position.z };
	go->transform->rotation = { rotation.x, rotation.y, rotation.z, rotation.w };
	go->transform->scale = { scale.x, scale.y, scale.z };

	// Meshes
	if (node->mNumMeshes > 0)
	{
		aiMesh* nodeMesh = scene->mMeshes[node->mMeshes[0]];

		go->AddComponent(ComponentType::Mesh_Component);
		Mesh* goMesh = go->meshRenderer->mesh;

		// Unique vertices
		goMesh->verticesSize = nodeMesh->mNumVertices;
		goMesh->vertices = new float[goMesh->verticesSize * 3];
		memcpy(goMesh->vertices, (float*)nodeMesh->mVertices, sizeof(float) * goMesh->verticesSize * 3);

		// Indices
		if (nodeMesh->HasFaces())
		{
			uint facesSize = nodeMesh->mNumFaces;
			goMesh->indicesSize = facesSize * 3;
			goMesh->indices = new uint[goMesh->indicesSize];

			for (uint j = 0; j < facesSize; ++j)
			{
				if (nodeMesh->mFaces[j].mNumIndices != 3)
				{
					CONSOLE_LOG("WARNING, geometry face with != 3 indices!");
				}
				else
					memcpy(&goMesh->indices[j * 3], nodeMesh->mFaces[j].mIndices, 3 * sizeof(uint));
			}
		}

		// Texture coords
		// TODO: multitexturing
		if (nodeMesh->HasTextureCoords(0))
		{
			goMesh->textureCoordsSize = goMesh->verticesSize * 2;
			goMesh->textureCoords = new float[goMesh->textureCoordsSize];

			for (uint j = 0; j < goMesh->verticesSize; ++j)
			{
				memcpy(&goMesh->textureCoords[j * 2], &nodeMesh->mTextureCoords[0][j].x, sizeof(float));
				memcpy(&goMesh->textureCoords[(j * 2) + 1], &nodeMesh->mTextureCoords[0][j].y, sizeof(float));
			}
		}

		// Material
		if (scene->mMaterials[nodeMesh->mMaterialIndex] != nullptr) // TODO CHECK IF NULL
		{
			aiString textureName;
			scene->mMaterials[nodeMesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);

			std::string filePath;
			if (App->filesystem->ExistsInAssets(textureName.data, FileType::TextureFile, filePath))
			{
				std::string outputFileName;			
				if (App->materialImporter->Import(textureName.data, filePath.data(), outputFileName))
				{
					Texture* texture = new Texture();
					if (App->materialImporter->Load(outputFileName.data(), texture))
					{
						go->AddComponent(ComponentType::Material_Component);
						go->materialRenderer->textures.push_back(texture);
					}
				}			
			}		
		}

		goMesh->Init();
		go->RecursiveRecalculateBoundingBoxes();

		uint ranges[3] = { goMesh->verticesSize, goMesh->indicesSize, goMesh->textureCoordsSize };

		uint size = sizeof(ranges) +
					sizeof(float) * goMesh->verticesSize +
					sizeof(uint) * goMesh->indicesSize +
					sizeof(float) * goMesh->textureCoordsSize;

		char* data = new char[size];
		char* cursor = data;

		// 1. Store ranges
		uint bytes = sizeof(ranges);
		memcpy(cursor, ranges, bytes);

		cursor += bytes;

		// 3. Store vertices
		bytes = sizeof(float) * goMesh->verticesSize;
		memcpy(cursor, goMesh->vertices, bytes);

		cursor += bytes;

		// 4. Store indices
		bytes = sizeof(uint) * goMesh->indicesSize;
		memcpy(cursor, goMesh->indices, bytes);

		cursor += bytes;

		// 5. Store texture coords
		bytes = sizeof(float) * goMesh->textureCoordsSize;
		memcpy(cursor, goMesh->textureCoords, bytes);

		if (App->filesystem->SaveInLibrary(data, size, FileType::MeshFile, outputFileName, node->mMeshes[0]) > 0)
		{
			CONSOLE_LOG("SCENE IMPORTER: Successfully saved mesh %s to own format", go->GetName());
		}
		else
			CONSOLE_LOG("SCENE IMPORTER: Could not save mesh %s to own format", go->GetName());
	}

	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		RecursivelyImportNodes(scene, node->mChildren[i], go, outputFileName);
	}
}

bool SceneImporter::Load(const char* exportedFileName, Mesh* outputMesh)
{
	bool ret = false;

	char* buffer;
	uint size = App->filesystem->LoadFromLibrary(exportedFileName, &buffer, FileType::MeshFile);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded mesh %s (own format)", exportedFileName);
		ret = Load(buffer, size, outputMesh);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load mesh %s (own format)", exportedFileName);

	return ret;
}

bool SceneImporter::Load(const void* buffer, uint size, Mesh* outputMesh)
{
	bool ret = false;

	if (buffer == nullptr || size <= 0 || outputMesh == nullptr)
		return ret;

	char* cursor = (char*)buffer;

	// Mesh Name + Vertices + Indices + Texture Coords + Texture Name
	uint ranges[3];
	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);

	cursor += bytes;

	//uint meshNameSize = ranges[0];
	outputMesh->verticesSize = ranges[0];
	outputMesh->indicesSize = ranges[1];
	outputMesh->textureCoordsSize = ranges[2];
	//uint textureNameSize = ranges[4];

	// 1. Load mesh name
	//bytes = sizeof(char) * meshNameSize;
	//outputMesh->name = new char[meshNameSize];
	//memcpy((char*)outputMesh->name, cursor, bytes);

	// 2. Load vertices
	bytes = sizeof(float) * outputMesh->verticesSize;
	outputMesh->vertices = new float[outputMesh->verticesSize];
	memcpy(outputMesh->vertices, cursor, bytes);

	cursor += bytes;

	// 3. Load indices
	bytes = sizeof(uint) * outputMesh->indicesSize;
	outputMesh->indices = new uint[outputMesh->indicesSize];
	memcpy(outputMesh->indices, cursor, bytes);

	cursor += bytes;

	// 4. Load texture coords
	bytes = sizeof(float) * outputMesh->textureCoordsSize;
	outputMesh->textureCoords = new float[outputMesh->textureCoordsSize];
	memcpy(outputMesh->textureCoords, cursor, bytes);

	// 5. Load texture name
	//bytes = sizeof(char) * textureNameSize;
	//const char* textureName = new char[textureNameSize];
	//memcpy((char*)textureName, cursor, bytes);

	// TODO: find the texture associated with the material 0 of the mesh

	CONSOLE_LOG("SCENE IMPORTER: New mesh loaded with: %i vertices, %i indices, %i texture coords", outputMesh->verticesSize, outputMesh->indicesSize, outputMesh->textureCoordsSize);
	ret = true;
}

uint SceneImporter::GetAssimpMajorVersion() const
{
	return aiGetVersionMajor();
}

uint SceneImporter::GetAssimpMinorVersion() const
{
	return aiGetVersionMinor();
}

uint SceneImporter::GetAssimpRevisionVersion() const
{
	return aiGetVersionRevision();
}

// Mesh --------------------------------------------------
void Mesh::Init()
{
	// Generate vertices buffer
	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize * 3, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generate indices buffer
	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Generate texture coords
	glGenBuffers(1, (GLuint*)&textureCoordsID);
	glBindBuffer(GL_ARRAY_BUFFER, textureCoordsID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize * 2, textureCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, (GLuint*)&verticesID);
	glDeleteBuffers(1, (GLuint*)&indicesID);
	glDeleteBuffers(1, (GLuint*)&textureCoordsID);

	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(indices);
	RELEASE_ARRAY(textureCoords);
}