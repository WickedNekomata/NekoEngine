#include "ModuleAssetImport.h"
#include "ModuleRenderer3D.h"

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
struct Mesh
{
	int num_vertices;
	float* vertices;

	uint* indices;
	int num_indices;
};
bool ModuleAssetImport::LoadFBXfromFile(const char* path)
{
	bool ret = true;

	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene != nullptr)
	{	
		for (int i = 0; i < scene->mNumMeshes; ++i)
		{			
			Mesh mesh;

			mesh.num_vertices = scene->mMeshes[i]->mNumVertices;
			mesh.vertices = new float[mesh.num_vertices * 3];
			memcpy(mesh.vertices, scene->mMeshes[i]->mVertices, sizeof(float) * mesh.num_vertices * 3);
			CONSOLE_LOG("New mesh with %d vertices");

			if (scene->mMeshes[i]->HasFaces())
			{
				mesh.num_indices = scene->mMeshes[i]->mNumFaces * 3;
				mesh.indices = new uint[mesh.num_indices];
				for (uint i = 0; i < scene->mMeshes[i]->mNumFaces; ++i)
				{
					if (scene->mMeshes[i]->mFaces[i].mNumIndices != 3)
					{
						CONSOLE_LOG("WARNING, geometry face with != 3 indices!");
					}
					else
					{
						memcpy(&mesh.indices[i * 3], scene->mMeshes[i]->mFaces[i].mIndices, 3 * sizeof(uint));
					}
				}
			}
			// renderer->addMesh(mesh);
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

bool ModuleAssetImport::LoadFBXfromMemory(const char * buffer, unsigned int bufferSize)
{
	bool ret = true;

	const aiScene* scene = aiImportFileFromMemory(buffer, bufferSize, aiProcessPreset_TargetRealtime_MaxQuality, "");

	if (scene != nullptr)
	{
		for (int i = 0; i < scene->mNumMeshes; ++i)
		{
			Mesh mesh;

			mesh.num_vertices = scene->mMeshes[i]->mNumVertices;
			mesh.vertices = new float[mesh.num_vertices * 3];
			memcpy(mesh.vertices, scene->mMeshes[i]->mVertices, sizeof(float) * mesh.num_vertices * 3);
			CONSOLE_LOG("New mesh with %d vertices");

			if (scene->mMeshes[i]->HasFaces())
			{
				mesh.num_indices = scene->mMeshes[i]->mNumFaces * 3;
				mesh.indices = new uint[mesh.num_indices];
				for (uint i = 0; i < scene->mMeshes[i]->mNumFaces; ++i)
				{
					if (scene->mMeshes[i]->mFaces[i].mNumIndices != 3)
					{
						CONSOLE_LOG("WARNING, geometry face with != 3 indices!");
					}
					else
					{
						memcpy(&mesh.indices[i * 3], scene->mMeshes[i]->mFaces[i].mIndices, 3 * sizeof(uint));
					}
				}
			}
			// renderer->addMesh(mesh);
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
