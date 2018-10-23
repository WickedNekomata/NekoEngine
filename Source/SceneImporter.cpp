#include "SceneImporter.h"

#include "Application.h"

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

bool SceneImporter::Import(const char* importFile, const char* importPath, const char* outputFile)
{
	bool ret = false;

	if (importPath == nullptr)
		return ret;

	char importFilePath[DEFAULT_BUF_SIZE];
	char outputFileName[DEFAULT_BUF_SIZE];

	strcpy_s(importFilePath, strlen(importPath) + 1, importPath);

	// If importFile == nullptr, it means that importPath already contains the name of the file to be imported
	// If outputFile == nullptr, use the name of importFile (or, if importFile == nullptr, find its name from importPath)
	if (outputFile != nullptr)
		strcpy_s(outputFileName, strlen(outputFile) + 1, outputFile);
	else if (importFile != nullptr)
	{
		strcat_s(importFilePath, strlen(importFile) + 1, importFile);
		strcpy_s(outputFileName, strlen(importFile) + 1, importFile);
	}
	else
	{
		const char* importFileName = App->filesystem->GetFileNameFromPath(importPath);
		strcpy_s(outputFileName, strlen(importFileName) + 1, importFileName);
	}

	char* buffer;
	uint size = App->filesystem->Load(importFilePath, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded mesh(es) %s (original format)", importFile);
		ret = Import(buffer, size, outputFile);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SCENE IMPORTER: Could not load mesh(es) %s (original format)", importFile);

	return ret;
}

bool SceneImporter::Import(const void* buffer, uint size, const char* outputFile)
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
		for (uint i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[i];

			// Mesh Name
			const char* meshName = mesh->mName.C_Str();
			uint meshNameSize = strlen(meshName) + 1;

			// Unique vertices
			uint verticesSize = mesh->mNumVertices;
			float* vertices = (float*)mesh->mVertices;

			// Indices
			uint indicesSize = 0;
			uint* indices = nullptr;
			if (mesh->HasFaces())
			{
				uint facesSize = mesh->mNumFaces;
				indicesSize = facesSize * 3;
				indices = new uint[indicesSize];

				for (uint j = 0; j < facesSize; ++j)
				{
					if (mesh->mFaces[j].mNumIndices != 3)
					{
						CONSOLE_LOG("WARNING, geometry face with != 3 indices!");
					}
					else
						memcpy(&indices[j * 3], mesh->mFaces[j].mIndices, 3 * sizeof(uint));
				}
			}

			// Texture coords
			// TODO: multitexturing
			uint textureCoordsSize = 0;
			float* textureCoords = nullptr;
			if (mesh->HasTextureCoords(0))
			{
				textureCoordsSize = verticesSize * 2;
				textureCoords = new float[textureCoordsSize];

				for (uint j = 0; j < verticesSize; ++j)
				{
					memcpy(&textureCoords[j * 2], &mesh->mTextureCoords[0][j].x, sizeof(float));
					memcpy(&textureCoords[(j * 2) + 1], &mesh->mTextureCoords[0][j].y, sizeof(float));
				}
			}

			// Material 0 (Texture Name)
			const char* textureName = nullptr;
			uint textureNameSize = 0;
			if (scene->mMaterials[0] != nullptr)
			{
				aiString texName;
				scene->mMaterials[0]->GetTexture(aiTextureType_DIFFUSE, 0, &texName);
				std::string realTexName = texName.data;
				realTexName = realTexName.substr((realTexName.find_last_of("\\") + 1), realTexName.size());

				textureName = realTexName.data();
				textureNameSize = realTexName.size();
			}

			// Normals
			/*
			if (scene->mMeshes[i]->HasNormals())
			{
				mesh->normals = new float[mesh->verticesSize * 3];
				memcpy(mesh->normals, scene->mMeshes[i]->mNormals, sizeof(float) * mesh->verticesSize * 3);
				CONSOLE_LOG("Mesh vertices normals loaded");
			}
			*/

			// Mesh Name + Vertices + Indices + Texture Coords + Texture Name
			uint ranges[5] = { meshNameSize, verticesSize, indicesSize, textureCoordsSize, textureNameSize };

			uint size = sizeof(ranges) + sizeof(char) * meshNameSize + sizeof(float) * verticesSize + sizeof(uint) * indicesSize + sizeof(float) * textureCoordsSize + sizeof(char) * textureNameSize;

			char* data = new char[size];
			char* cursor = data;

			// 1. Store ranges
			uint bytes = sizeof(ranges);
			memcpy(cursor, ranges, bytes);

			cursor += bytes;

			// 2. Store mesh name
			bytes = sizeof(float) * meshNameSize;
			memcpy(cursor, meshName, bytes);

			// 3. Store vertices
			bytes = sizeof(float) * verticesSize;
			memcpy(cursor, vertices, bytes);

			// 4. Store indices
			bytes = sizeof(uint) * indicesSize;
			memcpy(cursor, indices, bytes);

			// 5. Store texture coords
			bytes = sizeof(float) * textureCoordsSize;
			memcpy(cursor, textureCoords, bytes);

			// 6. Store texture name
			bytes = sizeof(float) * textureNameSize;
			memcpy(cursor, textureName, bytes);

			if (App->filesystem->SaveInLibrary(outputFile, data, size, FileType::MeshFile) > 0)
			{
				CONSOLE_LOG("SCENE IMPORTER: Successfully saved mesh %s to own format", meshName);
				ret = true;
			}
			else
				CONSOLE_LOG("SCENE IMPORTER: Could not save mesh %s to own format", meshName);

			RELEASE_ARRAY(indices);
			RELEASE_ARRAY(textureCoords);

			// Transform
			/*
			if (scene->mRootNode != nullptr)
			{
				aiVector3D position;
				aiVector3D scale;
				aiQuaternion rotation;

				scene->mRootNode->mTransformation.Decompose(scale, rotation, position);
				mesh->position = { position.x, position.y, position.z };
				mesh->scale = { scale.x, scale.y, scale.z };
				mesh->rotation = { rotation.x, rotation.y, rotation.z, rotation.w };
			}
			*/
		}

		aiReleaseImport(scene);
	}

	return ret;
}

bool SceneImporter::Load(const char* exportedFile, Mesh* outputMesh)
{
	bool ret = false;

	char* buffer;
	uint size = App->filesystem->LoadFromLibrary(exportedFile, &buffer, FileType::MeshFile);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded mesh %s (own format)", exportedFile);
		ret = Load(buffer, size, outputMesh);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load mesh %s (own format)", exportedFile);

	return ret;
}

bool SceneImporter::Load(const void* buffer, uint size, Mesh* outputMesh)
{
	bool ret = false;

	if (buffer == nullptr || size <= 0 || outputMesh == nullptr)
		return ret;

	char* cursor = (char*)buffer;

	// Mesh Name + Vertices + Indices + Texture Coords + Texture Name
	uint ranges[5];
	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);

	cursor += bytes;

	uint meshNameSize = ranges[0];
	outputMesh->verticesSize = ranges[1];
	outputMesh->indicesSize = ranges[2];
	outputMesh->textureCoordsSize = ranges[3];
	uint textureNameSize = ranges[4];

	// 1. Load mesh name
	bytes = sizeof(char) * meshNameSize;
	outputMesh->name = new char[meshNameSize];
	memcpy((char*)outputMesh->name, cursor, bytes);

	// 2. Load vertices
	bytes = sizeof(float) * outputMesh->verticesSize;
	outputMesh->vertices = new float[outputMesh->verticesSize];
	memcpy(outputMesh->vertices, cursor, bytes);

	// 3. Load indices
	bytes = sizeof(uint) * outputMesh->indicesSize;
	outputMesh->indices = new uint[outputMesh->indicesSize];
	memcpy(outputMesh->indices, cursor, bytes);

	// 4. Load texture coords
	bytes = sizeof(float) * outputMesh->textureCoordsSize;
	outputMesh->textureCoords = new float[outputMesh->textureCoordsSize];
	memcpy(outputMesh->textureCoords, cursor, bytes);

	// 5. Load texture name
	bytes = sizeof(char) * textureNameSize;
	const char* textureName = new char[textureNameSize];
	memcpy((char*)textureName, cursor, bytes);

	// TODO: find the texture associated with the material 0 of the mesh

	CONSOLE_LOG("SCENE IMPORTER: New mesh loaded with: %i vertices, %i indices, %i texture coords", outputMesh->verticesSize, outputMesh->indicesSize, outputMesh->textureCoordsSize);
	ret = true;

	RELEASE_ARRAY(textureName);
}

void SceneImporter::InitMeshesFromScene(const aiScene* scene, const char* path) const
{
	for (uint i = 0; i < scene->mNumMeshes; ++i)
	{
		Mesh* mesh = new Mesh();

		// Unique vertices
		mesh->verticesSize = scene->mMeshes[i]->mNumVertices;
		mesh->vertices = new float[mesh->verticesSize * 3];
		memcpy(mesh->vertices, scene->mMeshes[i]->mVertices, sizeof(float) * mesh->verticesSize * 3);
		CONSOLE_LOG("New mesh with %d vertices", mesh->verticesSize);

		// Indices
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
	
		// Texture coords
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
	}	
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

/*

if (importFilePath != nullptr)
				{
					std::string meshPath = importFilePath;
					std::string texturePath = meshPath.substr(0, meshPath.find_last_of("\\") + 1) + realTextureName.data();

					if (!App->tex->LoadImageFromFile(texturePath.data()))
					{
						std::string texturePath = "Assets\\Textures\\Models\\";
						texturePath += realTextureName.data();
						if (!App->tex->LoadImageFromFile(texturePath.data()))
						{
							std::string texturePath = "Assets\\Textures\\";
							texturePath += realTextureName.data();
							if (!App->tex->LoadImageFromFile(texturePath.data()))
							{
								std::string texturePath = "Game\\";
								texturePath += realTextureName.data();
								if (!App->tex->LoadImageFromFile(texturePath.data()))
								{
									CONSOLE_LOG("Impossible to load texture: %s", realTextureName.data());
								}
								else
								{
									CONSOLE_LOG("Loaded correctly texture: %s", realTextureName.data());
								}
							}
							else
							{
								CONSOLE_LOG("Loaded correctly texture: %s", realTextureName.data());
							}
						}
						else
						{
							CONSOLE_LOG("Loaded correctly texture: %s", realTextureName.data());
						}
					}
				}
				else
					CONSOLE_LOG("The texture '%s', used by the material 0, couldn't be found", realTextureName.data());
*/