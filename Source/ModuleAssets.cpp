#include "ModuleAssets.h"
#include "ModuleRenderer3D.h"

#include "Application.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

void myCallback(const char* msg, char* userData)
{
	CONSOLE_LOG("%s", msg);
}

ModuleAssets::ModuleAssets(bool start_enabled)
{
}

ModuleAssets::~ModuleAssets()
{
}

bool ModuleAssets::Init(JSON_Object* jObject)
{
	struct aiLogStream stream;
	stream.callback = myCallback;
	aiAttachLogStream(&stream);

	return true;
}

bool ModuleAssets::CleanUp()
{
	aiDetachAllLogStreams();
	return true;
}

bool ModuleAssets::LoadMeshFromFile(const char* path) const
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

bool ModuleAssets::LoadMeshFromMemory(const char* buffer, unsigned int& bufferSize) const
{
	bool ret = false;

	App->renderer3D->ClearMeshes();

	uint postProcessingFlags = 0;
	postProcessingFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_Triangulate;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
	postProcessingFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;

	const aiScene* scene = aiImportFileFromMemory(buffer, bufferSize, postProcessingFlags, nullptr);

	if (scene != nullptr)
	{
		InitFromScene(scene);
		aiReleaseImport(scene);

		ret = true;
	}

	return ret;
}

bool ModuleAssets::LoadMeshWithPHYSFS(const char* path)
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

void ModuleAssets::InitFromScene(const aiScene* scene) const
{
	// Init mesh
	for (uint i = 0; i < scene->mNumMeshes; ++i)
	{
		Mesh* mesh = new Mesh();

		mesh->verticesSize = scene->mMeshes[i]->mNumVertices;
		mesh->vertices = new float[mesh->verticesSize * 3];
		memcpy(mesh->vertices, scene->mMeshes[i]->mVertices, sizeof(float) * mesh->verticesSize * 3);
		CONSOLE_LOG("New mesh with %d vertices", mesh->verticesSize);

		if (scene->mMeshes[i]->HasFaces())
		{
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

		if (scene->mMeshes[i]->HasNormals())
		{
			mesh->normals = new float[mesh->verticesSize * 3];
			memcpy(mesh->normals, scene->mMeshes[i]->mNormals, sizeof(float) * mesh->verticesSize * 3);
			CONSOLE_LOG("Mesh vertices normals loaded");
		}
	
		if (scene->mMeshes[i]->HasTextureCoords(0))
		{
			mesh->textureCoords = new float[scene->mMeshes[i]->mNumVertices * 2];

			for (uint j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
			{
				memcpy(&mesh->textureCoords[j * 2], &scene->mMeshes[i]->mTextureCoords[0][j].x, sizeof(float));
				memcpy(&mesh->textureCoords[(j * 2) + 1], &scene->mMeshes[i]->mTextureCoords[0][j].y, sizeof(float));
			}
			CONSOLE_LOG("Mesh tex coords at channel 0 loaded");
		}

		mesh->Init();
		App->renderer3D->AddMesh(mesh);
	}
}