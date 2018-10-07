#include "ModuleAssetImporter.h"
#include "ModuleRenderer3D.h"

#include "Application.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

ModuleAssetImporter::ModuleAssetImporter(bool start_enabled)
{
}

ModuleAssetImporter::~ModuleAssetImporter()
{
}

bool ModuleAssetImporter::Init(JSON_Object * jObject)
{
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	return true;
}

bool ModuleAssetImporter::CleanUp()
{
	aiDetachAllLogStreams();
	return true;
}

bool ModuleAssetImporter::LoadMeshFromFile(const char* path) const
{
	bool ret = false;

	App->renderer3D->ClearMeshes();
	
	uint postProcessingFlags = 0;
	postProcessingFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_Triangulate;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;

	const aiScene* scene = aiImportFile(path, postProcessingFlags);

	if (scene != nullptr)
	{
		InitFromScene(scene);
		aiReleaseImport(scene);

		ret = true;
	}
	else
		CONSOLE_LOG("Error loading scene %s", path);

	return ret;
}

bool ModuleAssetImporter::LoadMeshFromMemory(const char* buffer, unsigned int& bufferSize) const
{
	bool ret = false;

	App->renderer3D->ClearMeshes();

	const aiScene* scene = aiImportFileFromMemory(buffer, bufferSize, aiProcessPreset_TargetRealtime_MaxQuality, nullptr);

	if (scene != nullptr)
	{
		InitFromScene(scene);
		aiReleaseImport(scene);

		ret = true;
	}

	return ret;
}

bool ModuleAssetImporter::LoadMeshWithPHYSFS(const char* path)
{
	bool ret = false;

	char* buffer;
	uint size;

	if (App->filesystem->OpenRead(path, &buffer, size))
	{
		ret = LoadMeshFromMemory(buffer, size);

		if (!ret)
			CONSOLE_LOG("Error loading scene %s", path);

		delete[] buffer;
	}

	return ret;
}

void ModuleAssetImporter::InitFromScene(const aiScene* scene) const
{
	// Init mesh
	for (uint i = 0; i < scene->mNumMeshes; ++i)
	{
		Mesh* mesh = new Mesh();

		// Vertices
		mesh->verticesSize = scene->mMeshes[i]->mNumVertices;
		mesh->vertices = new float[mesh->verticesSize * 3];
		memcpy(mesh->vertices, scene->mMeshes[i]->mVertices, sizeof(float) * mesh->verticesSize * 3);
		CONSOLE_LOG("New mesh with %d vertices", mesh->verticesSize);

		if (scene->mMeshes[i]->HasFaces())
		{
			// Indices
			mesh->indicesSize = scene->mMeshes[i]->mNumFaces * 3;
			mesh->indices = new uint[mesh->indicesSize];
			
			for (uint j = 0; j < scene->mMeshes[i]->mNumFaces; ++j)
			{
				if (scene->mMeshes[i]->mFaces[j].mNumIndices != 3)
				{
					CONSOLE_LOG("WARNING, geometry face with != 3 indices!");
				}
				else
				{
					memcpy(&mesh->indices[j * 3], scene->mMeshes[i]->mFaces[j].mIndices, 3 * sizeof(uint));
				}
			}
		}

		// Normals
		if (scene->mMeshes[i]->HasNormals())
		{
			mesh->normals = new float[mesh->verticesSize * 3];
			memcpy(mesh->normals, scene->mMeshes[i]->mNormals, sizeof(float) * mesh->verticesSize * 3);
			CONSOLE_LOG("Mesh with normals");
		}

		// Texture coords
		if (scene->mMeshes[i]->HasTextureCoords(0))
		{
			mesh->textureCoords = new float[mesh->verticesSize * 2];
			memcpy(mesh->normals, scene->mMeshes[i]->mTextureCoords[0], sizeof(float) * mesh->verticesSize * 2);
			CONSOLE_LOG("Mesh with texture coords");
		}

		mesh->Init();
		App->renderer3D->AddMesh(mesh);
	}

	// Init materials
	for (uint i = 0; i < scene->mNumMaterials; ++i)
	{
		// TODO
	}
}
