#include "GameMode.h"

#include "SceneImporter.h"

#include "Application.h"
#include "BoneImporter.h"
#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"

#include "ModuleGOs.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentTransform.h"
#include "ComponentBone.h"
#include "BoneImporter.h"

#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"
#include "Assimp\include\version.h"

#include "glew\include\GL\glew.h"

#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\Quat.h"

#include <assert.h>

#pragma comment (lib, "Assimp\\libx86\\assimp-vc140-mt.lib")

#define T_PRE_ROT "PreRotation"
#define T_ROT "Rotation"
#define T_POST_ROT "PostRotation"
#define T_TRANSLATION "Translation"
#define T_SCALING "Scaling"

void myCallback(const char* msg, char* userData)
{
	CONSOLE_LOG(LogTypes::Normal, "%s", msg);
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

bool SceneImporter::Import(const char* file, std::vector<std::string>& outputFiles, const ResourceMeshImportSettings& importSettings, std::vector<uint>& forcedUuids) const
{
	assert(file != nullptr);

	bool ret = false;

	std::string fileName;
	App->fs->GetFileName(file, fileName);

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: Successfully loaded Model '%s'", fileName.data());
		ret = Import(buffer, size, fileName.data(), outputFiles, importSettings, forcedUuids);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "SCENE IMPORTER: Could not load Model '%s'", fileName.data());

	return ret;
}

bool SceneImporter::Import(const void* buffer, uint size, const char* prefabName, std::vector<std::string>& outputFiles, const ResourceMeshImportSettings& importSettings, std::vector<uint>& forcedUuids) const
{
	assert(buffer != nullptr && size > 0);

	bool ret = false;

	uint postProcessConfigurationFlags = 0;
	switch (importSettings.postProcessConfigurationFlags)
	{
	case ResourceMeshImportSettings::PostProcessConfigurationFlags::TARGET_REALTIME_FAST:
		postProcessConfigurationFlags |= aiProcessPreset_TargetRealtime_Fast;
		break;
	case ResourceMeshImportSettings::PostProcessConfigurationFlags::TARGET_REALTIME_QUALITY:
		postProcessConfigurationFlags |= aiProcessPreset_TargetRealtime_Quality;
		break;
	case ResourceMeshImportSettings::PostProcessConfigurationFlags::TARGET_REALTIME_MAX_QUALITY:
		postProcessConfigurationFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
		break;
	case ResourceMeshImportSettings::PostProcessConfigurationFlags::CUSTOM:
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::CALC_TANGENT_SPACE)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_CalcTangentSpace;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::GEN_NORMALS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_GenNormals;
		else if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::GEN_SMOOTH_NORMALS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::JOIN_IDENTICAL_VERTICES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::TRIANGULATE)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_Triangulate;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::GEN_UV_COORDS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_GenUVCoords;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::SORT_BY_P_TYPE)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_SortByPType;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::IMPROVE_CACHE_LOCALITY)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_ImproveCacheLocality;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::LIMIT_BONE_WEIGHTS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_LimitBoneWeights;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::REMOVE_REDUNDANT_MATERIALS)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_RemoveRedundantMaterials;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::SPLIT_LARGE_MESHES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_SplitLargeMeshes;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::FIND_DEGENERATES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_FindDegenerates;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::FIND_INVALID_DATA)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_FindInvalidData;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::FIND_INSTANCES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_FindInstances;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::VALIDATE_DATA_STRUCTURE)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_ValidateDataStructure;
		if (importSettings.customConfigurationFlags & ResourceMeshImportSettings::CustomConfigurationFlags::OPTIMIZE_MESHES)
			postProcessConfigurationFlags |= aiPostProcessSteps::aiProcess_OptimizeMeshes;
		break;
	}

	const aiScene* scene = aiImportFileFromMemory((const char*)buffer, size, postProcessConfigurationFlags, nullptr);

	if (scene != nullptr)
	{
		ret = true;

		// 1. Import the scene
		const aiNode* rootNode = scene->mRootNode;

		GameObject* dummy = new GameObject("Dummy", nullptr);
		GameObject* rootGameObject = new GameObject(rootNode->mName.data, dummy); // Root game object will never be a transformation

		std::vector<uint> dummyForcedUuids = forcedUuids;
		RecursivelyImportNodes(scene, rootNode, rootGameObject, nullptr, outputFiles, dummyForcedUuids);



		aiReleaseImport(scene);

		// 2. Serialize the imported scene
		// TODO: create prefab
		//App->GOs->SerializeFromNode(dummy, outputFile);

		dummy->RecursiveForceAllResources(0);
		// TODO DESTROY DUMMY/CHILDREN/COMPONENTS
		App->GOs->Kill(dummy);
	}

	return ret;
}

void SceneImporter::RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation, std::vector<std::string>& outputFiles, std::vector<uint>& forcedUuids) const
{
	std::string name = node->mName.data;

	// A game object is a transformation if its name contains a transformation
	bool isTransformation =
		(name.find(T_PRE_ROT) != std::string::npos
			|| name.find(T_ROT) != std::string::npos
			|| name.find(T_POST_ROT) != std::string::npos
			|| name.find(T_TRANSLATION) != std::string::npos
			|| name.find(T_SCALING) != std::string::npos);

	GameObject* gameObject = nullptr;

	// If the node is the first node, then the game object is the parent
	if (node == scene->mRootNode)
		gameObject = (GameObject*)parent;
	// If the previous game object was a transformation, keep the transformation
	else if (transformation != nullptr)
		gameObject = (GameObject*)transformation;
	// If the previous game object wasn't a transformation, create a new game object
	else
		gameObject = new GameObject(name.data(), (GameObject*)parent);
	
	// If the current game object is not a transformation, update its name (just in case the previous one was)
	if (!isTransformation)
		gameObject->SetName(name.data());

	// Transform
	aiVector3D position;
	aiVector3D scale;
	aiQuaternion rotation;
	node->mTransformation.Decompose(scale, rotation, position);

	math::float3 newPosition = { position.x, position.y, position.z };
	math::Quat newRotation = { rotation.x, rotation.y, rotation.z, rotation.w };
	math::float3 newScale = { scale.x, scale.y, scale.z };

	if (transformation != nullptr)
	{
		gameObject->transform->position = transformation->transform->position + newPosition;
		gameObject->transform->rotation = transformation->transform->rotation * newRotation;
		gameObject->transform->scale = { transformation->transform->scale.x * newScale.x, transformation->transform->scale.y * newScale.y, transformation->transform->scale.z * newScale.z };
	}
	else
	{
		gameObject->transform->position = newPosition;
		gameObject->transform->rotation = newRotation;
		gameObject->transform->scale = newScale;
	}

	// Meshes
	if (!isTransformation && node->mNumMeshes > 0) // We assume that transformations don't contain meshes
	{
		aiMesh* nodeMesh = scene->mMeshes[node->mMeshes[0]];

		bool broken = false;
		for (uint i = 0; i < nodeMesh->mNumFaces; ++i)
		{
			broken = nodeMesh->mFaces[i].mNumIndices != 3;

			if (broken)
				break;
		}

		if (!broken)
		{
			gameObject->AddComponent(ComponentTypes::MeshComponent);
			if (forcedUuids.size() > 0)
			{
				gameObject->cmp_mesh->res = forcedUuids.front();
				forcedUuids.erase(forcedUuids.begin());
			}
			else
				gameObject->cmp_mesh->res = App->GenerateRandomNumber();

			float* vertices = nullptr;
			uint verticesSize = 0;

			uint* indices = nullptr;
			uint indicesSize = 0;

			float* normals = nullptr;
			uint normalsSize = 0;

			float* tangents = nullptr;
			uint tangentsSize = 0;

			float* bitangents = nullptr;
			uint bitangentsSize = 0;

			uchar* colors = nullptr;
			uint colorsSize = 0;

			float* texCoords = nullptr;
			uint texCoordsSize = 0;

			// Unique vertices
			verticesSize = nodeMesh->mNumVertices;
			vertices = new float[verticesSize * 3];
			memcpy(vertices, nodeMesh->mVertices, sizeof(float) * verticesSize * 3);

			// Indices
			if (nodeMesh->HasFaces())
			{
				uint facesSize = nodeMesh->mNumFaces;
				indicesSize = facesSize * 3;
				indices = new uint[indicesSize];

				for (uint j = 0; j < facesSize; ++j)
				{
					if (nodeMesh->mFaces[j].mNumIndices != 3)
					{
						CONSOLE_LOG(LogTypes::Warning, "WARNING, geometry face with != 3 indices!");
					}
					else
						memcpy(&indices[j * 3], nodeMesh->mFaces[j].mIndices, 3 * sizeof(uint));
				}
			}

			// Normals
			if (nodeMesh->HasNormals())
			{
				normalsSize = verticesSize;
				normals = new float[normalsSize * 3];
				memcpy(normals, nodeMesh->mNormals, sizeof(float) * normalsSize * 3);
			}
			
			// Tangents and Bitangents
			if (nodeMesh->HasTangentsAndBitangents())
			{
				tangentsSize = verticesSize;
				tangents = new float[tangentsSize * 3];
				memcpy(tangents, nodeMesh->mTangents, sizeof(float) * tangentsSize * 3);
				
				bitangentsSize = verticesSize;
				bitangents = new float[bitangentsSize * 3];
				memcpy(bitangents, nodeMesh->mBitangents, sizeof(float) * bitangentsSize * 3);
			}
			
			// Color
			if (nodeMesh->HasVertexColors(0))
			{
				colorsSize = verticesSize;
				colors = new uchar[colorsSize * 4];
				memcpy(colors, nodeMesh->mColors, sizeof(uchar) * colorsSize * 4);
			}

			// Texture coords
			if (nodeMesh->HasTextureCoords(0))
			{
				texCoordsSize = verticesSize;
				texCoords = new float[texCoordsSize * 2];

				for (uint j = 0; j < verticesSize; ++j)
				{
					memcpy(&texCoords[j * 2], &nodeMesh->mTextureCoords[0][j].x, sizeof(float));
					memcpy(&texCoords[(j * 2) + 1], &nodeMesh->mTextureCoords[0][j].y, sizeof(float));
				}

				// Material
				if (scene->mMaterials[nodeMesh->mMaterialIndex] != nullptr)
				{
					aiString textureName;
					scene->mMaterials[nodeMesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);

					if (textureName.length > 0)
					{
						std::string fileName;
						App->fs->GetFileName(textureName.data, fileName, true);

						// Check if the texture exists in Assets
						std::string outputFile = DIR_ASSETS;
						if (App->fs->RecursiveExists(fileName.data(), DIR_ASSETS, outputFile))
						{
							uint uuid = 0;
							std::vector<uint> uuids;
							if (App->res->GetResourcesUuidsByFile(outputFile.data(), uuids))
								uuid = uuids.front();
							else
								// If the texture is not a resource yet, import it
								uuid = App->res->ImportFile(outputFile.data())->GetUuid();

							assert(uuid > 0);
							gameObject->cmp_material->res[0].res = uuid;
						}
					}
				}
			}

			// Name
			char meshName[DEFAULT_BUF_SIZE];
			strcpy_s(meshName, DEFAULT_BUF_SIZE, name.data());

			// Vertices + Normals + Tangents + Bitangents + Colors + Texture Coords + Indices + Name
			uint ranges[8] = { verticesSize, normalsSize, tangentsSize, bitangentsSize, colorsSize, texCoordsSize, indicesSize, DEFAULT_BUF_SIZE };

			uint size = sizeof(ranges) +
				sizeof(float) * verticesSize * 3 +
				sizeof(float) * normalsSize * 3 +
				sizeof(float) * tangentsSize * 3 +
				sizeof(float) * bitangentsSize * 3 +
				sizeof(uchar) * colorsSize * 4 +
				sizeof(float) * texCoordsSize * 2 +
				sizeof(uint) * indicesSize +
				sizeof(char) * DEFAULT_BUF_SIZE;

			char* data = new char[size];
			char* cursor = data;

			// 1. Store ranges
			uint bytes = sizeof(ranges);
			memcpy(cursor, ranges, bytes);

			cursor += bytes;

			// 2. Store vertices
			bytes = sizeof(float) * verticesSize * 3;
			memcpy(cursor, vertices, bytes);

			cursor += bytes;

			// 3. Store normals
			if (normalsSize > 0)
			{
				bytes = sizeof(float) * normalsSize * 3;
				memcpy(cursor, normals, bytes);

				cursor += bytes;
			}
			
			// 4. Store tangents
			if (tangentsSize > 0)
			{
				bytes = sizeof(float) * tangentsSize * 3;
				memcpy(cursor, tangents, bytes);

				cursor += bytes;
			}

			// 5. Store bitangents
			if (bitangentsSize > 0)
			{
				bytes = sizeof(float) * bitangentsSize * 3;
				memcpy(cursor, bitangents, bytes);

				cursor += bytes;
			}
			
			// 6. Store colors
			if (colorsSize > 0)
			{
				bytes = sizeof(uchar) * colorsSize * 4;
				memcpy(cursor, colors, bytes);

				cursor += bytes;
			}

			// 7. Store texture coords
			if (texCoordsSize > 0)
			{
				bytes = sizeof(float) * texCoordsSize * 2;
				memcpy(cursor, texCoords, bytes);

				cursor += bytes;
			}

			// 8. Store indices
			bytes = sizeof(uint) * indicesSize;
			memcpy(cursor, indices, bytes);

			cursor += bytes;

			// 9. Store name
			bytes = sizeof(char) * DEFAULT_BUF_SIZE;
			memcpy(cursor, meshName, bytes);

			std::string outputFile = std::to_string(gameObject->cmp_mesh->res);
			if (App->fs->SaveInGame(data, size, FileType::MeshFile, outputFile) > 0)
			{
				CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: Successfully saved Mesh '%s' to own format", gameObject->GetName());
				outputFiles.push_back(outputFile);
			}
			else
				CONSOLE_LOG(LogTypes::Error, "SCENE IMPORTER: Could not save Mesh '%s' to own format", gameObject->GetName());

			RELEASE_ARRAY(data);
			RELEASE_ARRAY(vertices);
			RELEASE_ARRAY(indices);
			RELEASE_ARRAY(normals);
			RELEASE_ARRAY(tangents);
			RELEASE_ARRAY(bitangents);
			RELEASE_ARRAY(colors);
			RELEASE_ARRAY(texCoords);
		}
	}

	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		if (isTransformation)
			// If the current game object is a transformation, keep its parent and pass it as the new transformation for the next game object
			RecursivelyImportNodes(scene, node->mChildren[i], parent, gameObject, outputFiles, forcedUuids);
		else
			// Else, the current game object becomes the new parent for the next game object
			RecursivelyImportNodes(scene, node->mChildren[i], gameObject, nullptr, outputFiles, forcedUuids);
	}
}

bool SceneImporter::Load(const char* exportedFile, ResourceData& outputData, ResourceMeshData& outputMeshData) const
{
	assert(exportedFile != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(exportedFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: Successfully loaded Mesh '%s' (own format)", exportedFile);
		ret = Load(buffer, size, outputData, outputMeshData);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: Could not load Mesh '%s' (own format)", exportedFile);

	return ret;
}

bool SceneImporter::Load(const void* buffer, uint size, ResourceData& outputData, ResourceMeshData& outputMeshData) const
{
	assert(buffer != nullptr && size > 0);

	char* cursor = (char*)buffer;

	// Vertices + Normals + Tangents + Bitangents + Colors + Texture Coords + Indices + Name
	uint ranges[8];

	// 1. Load ranges
	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);

	cursor += bytes;

	outputMeshData.verticesSize = ranges[0];
	outputMeshData.indicesSize = ranges[6];
	uint normalsSize = ranges[1];
	uint tangentsSize = ranges[2];
	uint bitangentsSize = ranges[3];
	uint colorsSize = ranges[4];
	uint texCoordsSize = ranges[5];
	uint nameSize = ranges[7];

	char* normalsCursor = cursor + ranges[0] * sizeof(float) * 3;
	char* tangentsCursor = normalsCursor + ranges[1] * sizeof(float) * 3;
	char* bitangentsCursor = tangentsCursor + ranges[2] * sizeof(float) * 3;
	char* colorCursor = bitangentsCursor + ranges[3] * sizeof(float) * 3;
	char* texCoordsCursor = colorCursor + ranges[4] * sizeof(uchar) * 4;

	outputMeshData.vertices = new Vertex[outputMeshData.verticesSize];

	for (uint i = 0; i < outputMeshData.verticesSize; ++i)
	{
		// 2. Load vertices
		bytes = sizeof(float) * 3;
		memcpy(outputMeshData.vertices[i].position, cursor, bytes);

		cursor += bytes;

		// 3. Load normals
		if (normalsSize > 0)
		{
			bytes = sizeof(float) * 3;
			memcpy(outputMeshData.vertices[i].normal, normalsCursor, bytes);

			normalsCursor += bytes;
		}

		// 4. Load tangents
		if (tangentsSize > 0)
		{
			bytes = sizeof(float) * 3;
			memcpy(outputMeshData.vertices[i].tangent, tangentsCursor, bytes);

			tangentsCursor += bytes;
		}

		// 5. Load bitangents
		if (bitangentsSize > 0)
		{
			bytes = sizeof(float) * 3;
			memcpy(outputMeshData.vertices[i].bitangent, bitangentsCursor, bytes);

			bitangentsCursor += bytes;
		}

		// 6. Load colors
		if (colorsSize > 0)
		{
			bytes = sizeof(uchar) * 4;
			memcpy(outputMeshData.vertices[i].color, colorCursor, bytes);

			colorCursor += bytes;
		}

		// 7. Load texture coords
		if (texCoordsSize > 0)
		{
			bytes = sizeof(float) * 2;
			memcpy(outputMeshData.vertices[i].texCoord, texCoordsCursor, bytes);

			texCoordsCursor += bytes;
		}
	}

	// 8. Load indices
	cursor = texCoordsCursor;

	bytes = sizeof(uint) * outputMeshData.indicesSize;
	outputMeshData.indices = new uint[outputMeshData.indicesSize];
	memcpy(outputMeshData.indices, cursor, bytes);

	cursor += bytes;

	// 9. Load name
	bytes = sizeof(char) * nameSize;
	outputData.name.resize(nameSize);
	memcpy(&outputData.name[0], cursor, bytes);

	CONSOLE_LOG(LogTypes::Normal, "SCENE IMPORTER: New mesh loaded with %u vertices and %u indices", outputMeshData.verticesSize, outputMeshData.indicesSize);

	return true;
}

void SceneImporter::GenerateVBO(uint& VBO, Vertex* vertices, uint verticesSize) const
{
	assert(vertices != nullptr && verticesSize > 0);

	// Vertex Buffer Object

	// Generate a VBO
	glGenBuffers(1, &VBO);
	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verticesSize, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneImporter::GenerateIBO(uint& IBO, uint* indices, uint indicesSize) const
{
	assert(indices != nullptr && indicesSize > 0);

	// Index Buffer Object

	// Generate a IBO
	glGenBuffers(1, &IBO);
	// Bind the IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesSize, indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SceneImporter::GenerateVAO(uint& VAO, uint& VBO) const
{
	// Vertex Array Object

	// Generate a VAO
	glGenVertexArrays(1, &VAO);
	// Bind the VAO
	glBindVertexArray(VAO);

	// Bind the VBO 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Set the vertex attributes pointers
	// 1. Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);

	// 2. Normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);

	// 3. Color
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
	glEnableVertexAttribArray(2);

	// 4. Tex coords
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
	glEnableVertexAttribArray(3);

	// 5. Tangents
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));
	glEnableVertexAttribArray(4);

	// 6. Bitangents
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bitangent)));
	glEnableVertexAttribArray(5);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneImporter::DeleteBufferObject(uint& name) const
{
	glDeleteBuffers(1, &name);
}

void SceneImporter::DeleteVertexArrayObject(uint& name) const
{
	glDeleteVertexArrays(1, &name);
}

// ----------------------------------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------------------------------

void SceneImporter::LoadCubemap(uint& VBO, uint& VAO) const
{
	float skyboxVertices[] = {
        
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneImporter::LoadPrimitivePlane()
{
	uint verticesSize = 12;
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

	defaultPlaneIndicesSize = 6;
	uint indices[6]
	{
		// Front
		0, 1, 2, // ABC
		1, 3, 2, // BDC
	};

	// Vertices
	/// Position
	float* cursor = verticesPosition;
	for (uint i = 0; i < verticesSize/3; ++i)
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
	for (uint i = 0; i < textCordsSize/2; ++i)
	{
		memcpy(vertices[i].texCoord, cursor, sizeof(float) * 2);
		cursor += 2;
	}
	// -----

	GLuint VBO = 0;
	GenerateVBO(VBO, vertices, verticesSize);
	GenerateIBO(defaultPlaneIBO, indices, defaultPlaneIndicesSize);
	GenerateVAO(defaultPlaneVAO, VBO);
}

void SceneImporter::GetDefaultPlane(uint& defaultPlaneVAO, uint& defaultPlaneIBO, uint& defaultPlaneIndicesSize) const
{
	defaultPlaneVAO = this->defaultPlaneVAO;
	defaultPlaneIBO = this->defaultPlaneIBO;
	defaultPlaneIndicesSize = this->defaultPlaneIndicesSize;
}

void SceneImporter::RecursiveProcessBones(mutable const aiScene * scene,mutable const aiNode * node)const
{
	std::map<std::string, aiBone*>::iterator it = bones.find(node->mName.C_Str());

	if (it != bones.end())
	{
		aiBone* bone = it->second;

		GameObject* go = relations[node];
		ComponentBone* comp_bone = (ComponentBone*)go->AddComponent(ComponentTypes::BoneComponent);

		std::string output;
		// TODO_G : DONT CREATE THE RESOURCE IN THE IMPORT METHOD
		uint bone_uid = App->boneImporter->Import(bone, mesh_bone[bone], output);
		if (go->GetParent() == nullptr ||
			(go->GetParent() && !go->GetParent()->GetComponent(ComponentTypes::BoneComponent)))
			bone_root_uid = go->GetUUID();


		comp_bone->SetResource(bone_uid);
		imported_bones[node->mName.C_Str()] = bone_uid;
		DEPRECATED_LOG("->-> Added Bone component");
	}

	for (uint i = 0; i < node->mNumChildren; ++i)
		RecursiveProcessBones(scene, node->mChildren[i]);
}
