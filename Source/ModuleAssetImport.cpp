#include "ModuleAssetImport.h"
#include "ModuleRenderer3D.h"

#include "Application.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

ModuleAssetImport::ModuleAssetImport(bool start_enabled)
{
}

ModuleAssetImport::~ModuleAssetImport()
{
}

bool ModuleAssetImport::Init(JSON_Object * jObject)
{
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	return true;
}

bool ModuleAssetImport::CleanUp()
{
	aiDetachAllLogStreams();
	return true;
}

bool ModuleAssetImport::LoadFBXfromFile(const char* path) const
{
	bool ret = true;

	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene != nullptr)
	{	
		for (int i = 0; i < scene->mNumMeshes; ++i)
		{			
			Mesh* mesh = new Mesh();

			mesh->verticesSize = scene->mMeshes[i]->mNumVertices;
			mesh->vertices = new float[mesh->verticesSize * 3];
			memcpy(mesh->vertices, scene->mMeshes[i]->mVertices, sizeof(float) * mesh->verticesSize * 3);
			CONSOLE_LOG("New mesh with %d vertices");

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
			App->renderer3D->AddMesh(mesh);
		}
		aiReleaseImport(scene);
	}
	else
	{
		CONSOLE_LOG("Error loading scene %s");
		ret = false;
	}

	return ret;
}

bool ModuleAssetImport::LoadFBXfromMemory(const char * buffer, unsigned int bufferSize) const
{
	bool ret = true;

	const aiScene* scene = aiImportFileFromMemory(buffer, bufferSize, aiProcessPreset_TargetRealtime_MaxQuality, "");

	if (scene != nullptr)
	{
		for (int i = 0; i < scene->mNumMeshes; ++i)
		{
			Mesh* mesh = new Mesh();

			mesh->verticesSize = scene->mMeshes[i]->mNumVertices;
			mesh->vertices = new float[mesh->verticesSize * 3];
			memcpy(mesh->vertices, scene->mMeshes[i]->mVertices, sizeof(float) * mesh->verticesSize * 3);
			CONSOLE_LOG("New mesh with %d vertices");

			if (scene->mMeshes[i]->HasFaces())
			{
				mesh->indicesSize = scene->mMeshes[i]->mNumFaces * 3;
				mesh->indices = new uint[mesh->indicesSize];
				for (uint i = 0; i < scene->mMeshes[i]->mNumFaces; ++i)
				{
					if (scene->mMeshes[i]->mFaces[i].mNumIndices != 3)
					{
						CONSOLE_LOG("WARNING, geometry face with != 3 indices!");
					}
					else
					{
						memcpy(&mesh->indices[i * 3], scene->mMeshes[i]->mFaces[i].mIndices, 3 * sizeof(uint));
					}
				}
			}
			App->renderer3D->AddMesh(mesh);
		}
		aiReleaseImport(scene);
	}
	else
	{
		CONSOLE_LOG("Error loading scene %s");
		ret = false;
	}

	return ret;
}
