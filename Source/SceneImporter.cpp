#include "GameMode.h"

#include "SceneImporter.h"

#include "Application.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentTypes.h"
#include "MaterialImporter.h"
#include "Resource.h"
#include "ResourceMesh.h"

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

	std::string name;

	char fullImportPath[DEFAULT_BUF_SIZE];
	strcpy_s(fullImportPath, strlen(importPath) + 1, importPath);

	if (importFileName != nullptr)
	{
		name = importFileName;

		// Build the import path
		strcat_s(fullImportPath, strlen(fullImportPath) + strlen(importFileName) + 1, importFileName);
	}
	else
		App->filesystem->GetFileName(importPath, name);

	outputFileName = name.data();

	char* buffer;
	uint size = App->filesystem->Load(fullImportPath, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded model %s (original format)", name.data());
		ret = Import(buffer, size, outputFileName);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SCENE IMPORTER: Could not load model %s (original format)", name.data());

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
		RecursivelyImportNodes(scene, rootNode, App->scene->root, nullptr, outputFileName);

		aiReleaseImport(scene);
	}

	return ret;
}

void SceneImporter::RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parentGO, GameObject* transformationGO, std::string& outputFileName)
{
	/*
	std::string name = node->mName.data;

	bool isTransformation =
		(name.find("PreRotation") != std::string::npos
			|| name.find("Rotation") != std::string::npos
			|| name.find("PostRotation") != std::string::npos
			|| name.find("Translation") != std::string::npos
			|| name.find("Scaling") != std::string::npos);

	GameObject* go = transformationGO;

	if (go == nullptr)
		go = App->GOs->CreateGameObject((char*)name.data(), (GameObject*)parentGO);
	else if (!isTransformation)
		go->SetName((char*)name.data());

	// Transform
	aiVector3D position;
	aiVector3D scale;
	aiQuaternion rotation;
	node->mTransformation.Decompose(scale, rotation, position);

	math::float3 newPosition = { position.x, position.y, position.z };
	math::Quat newRotation = { rotation.x, rotation.y, rotation.z, rotation.w };
	math::float3 newScale = { scale.x, scale.y, scale.z };

	if (transformationGO != nullptr)
	{
		go->transform->position = transformationGO->transform->position + newPosition;
		go->transform->rotation = transformationGO->transform->rotation * newRotation;
		go->transform->scale = { transformationGO->transform->scale.x * newScale.x, transformationGO->transform->scale.y * newScale.y, transformationGO->transform->scale.z * newScale.z };
	}
	else
	{
		go->transform->position = newPosition;
		go->transform->rotation = newRotation;
		go->transform->scale = newScale;
	}
	*/
	bool isTransformation = false;
	GameObject* go = new GameObject("Name", (GameObject*)parentGO);

	// Meshes
	if (!isTransformation && node->mNumMeshes > 0)
	{
		aiMesh* nodeMesh = scene->mMeshes[node->mMeshes[0]];

		go->AddComponent(ComponentType::Mesh_Component);

		float* vertices = nullptr;
		uint verticesSize = 0;
		uint verticesID = 0;

		uint* indices = nullptr;
		uint indicesID = 0;
		uint indicesSize = 0;

		float* textureCoords = nullptr;
		uint textureCoordsID = 0;
		uint textureCoordsSize = 0;

		// Unique vertices
		verticesSize = nodeMesh->mNumVertices;
		vertices = new float[verticesSize * 3];
		memcpy(vertices, (float*)nodeMesh->mVertices, sizeof(float) * verticesSize * 3);

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
					CONSOLE_LOG("WARNING, geometry face with != 3 indices!");
				}
				else
					memcpy(&indices[j * 3], nodeMesh->mFaces[j].mIndices, 3 * sizeof(uint));
			}
		}

		// Texture coords
		// TODO: multitexturing
		if (nodeMesh->HasTextureCoords(0))
		{
			textureCoordsSize = verticesSize * 2;
			textureCoords = new float[textureCoordsSize];

			for (uint j = 0; j < verticesSize; ++j)
			{
				memcpy(&textureCoords[j * 2], &nodeMesh->mTextureCoords[0][j].x, sizeof(float));
				memcpy(&textureCoords[(j * 2) + 1], &nodeMesh->mTextureCoords[0][j].y, sizeof(float));
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

		uint ranges[3] = { verticesSize, indicesSize, textureCoordsSize };

		uint size = sizeof(ranges) +
			sizeof(float) * verticesSize +
			sizeof(uint) * indicesSize +
			sizeof(float) * textureCoordsSize;

		char* data = new char[size];
		char* cursor = data;

		// 1. Store ranges
		uint bytes = sizeof(ranges);
		memcpy(cursor, ranges, bytes);

		cursor += bytes;

		// 3. Store vertices
		bytes = sizeof(float) * verticesSize;
		memcpy(cursor, vertices, bytes);

		cursor += bytes;

		// 4. Store indices
		bytes = sizeof(uint) * indicesSize;
		memcpy(cursor, indices, bytes);

		cursor += bytes;

		// 5. Store texture coords
		bytes = sizeof(float) * textureCoordsSize;
		memcpy(cursor, textureCoords, bytes);

		if (App->filesystem->SaveInLibrary(data, size, FileType::MeshFile, outputFileName, node->mMeshes[0]) > 0)
		{
			CONSOLE_LOG("SCENE IMPORTER: Successfully saved mesh %s to own format", go->GetName());
		}
		else
			CONSOLE_LOG("SCENE IMPORTER: Could not save mesh %s to own format", go->GetName());
	}

	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		//if (isTransformation)
			//RecursivelyImportNodes(scene, node->mChildren[i], parentGO, go, outputFileName);
		//else
			RecursivelyImportNodes(scene, node->mChildren[i], go, nullptr, outputFileName);
	}
}

void SceneImporter::GenerateMeta(Resource* resource)
{
	ResourceMesh* resourceMesh = (ResourceMesh*)resource;

	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	// Fill the JSON with data
	json_object_set_number(rootObject, "UUID", resourceMesh->GetUUID());
	json_object_set_number(rootObject, "Time Created", App->timeManager->GetRealTime());

	JSON_Value* newValue = json_value_init_object();
	JSON_Object* objModule = json_value_get_object(newValue);
	json_object_set_value(rootObject, "Scene Importer", newValue);
	
	json_object_set_number(objModule, "Configuration", defaultImportSettings.configuration);
	json_object_set_number(objModule, "Calculate Tangent Space", defaultImportSettings.calcTangentSpace);
	json_object_set_number(objModule, "Generate Normals", defaultImportSettings.genNormals);
	json_object_set_number(objModule, "Generate Smooth Normals", defaultImportSettings.genSmoothNormals);
	json_object_set_number(objModule, "Join Identical Vertices", defaultImportSettings.joinIdenticalVertices);
	json_object_set_number(objModule, "Triangulate", defaultImportSettings.triangulate);
	json_object_set_number(objModule, "Generate UV Coordinates", defaultImportSettings.genUVCoords);
	json_object_set_number(objModule, "Sort By Primitive Type", defaultImportSettings.sortByPType);
	json_object_set_number(objModule, "Improve Cache Locality", defaultImportSettings.improveCacheLocality);
	json_object_set_number(objModule, "Limit Bone Weights", defaultImportSettings.limitBoneWeights);
	json_object_set_number(objModule, "Remove Redundant Materials", defaultImportSettings.removeRedundantMaterials);
	json_object_set_number(objModule, "Split Large Meshes", defaultImportSettings.splitLargeMeshes);
	json_object_set_number(objModule, "Find Degenerates", defaultImportSettings.findDegenerates);
	json_object_set_number(objModule, "Find Invalid Data", defaultImportSettings.findInvalidData);
	json_object_set_number(objModule, "Find Instances", defaultImportSettings.findInstances);
	json_object_set_number(objModule, "Validate Data Structure", defaultImportSettings.validateDataStructure);
	json_object_set_number(objModule, "Optimize Meshes", defaultImportSettings.optimizeMeshes);

	char path[DEFAULT_BUF_SIZE];
	strcpy_s(path, strlen(resource->file.data()) + 1, resource->file.data());

	// Build the path
	static const char extension[] = ".meta";
	strcat_s(path, strlen(path) + strlen(extension) + 1, extension);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);
	App->filesystem->Save(path, buf, sizeBuf);
	delete[] buf;
	json_value_free(rootValue);
}

bool SceneImporter::Load(const char* exportedFileName, ResourceMesh* outputMesh)
{
	bool ret = false;

	char* buffer;
	uint size = App->filesystem->LoadFromLibrary(exportedFileName, &buffer, FileType::MeshFile);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded mesh %s (own format)", exportedFileName);
		ret = Load(buffer, size, outputMesh);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SCENE IMPORTER: Could not load mesh %s (own format)", exportedFileName);

	return ret;
}

bool SceneImporter::Load(const void* buffer, uint size, ResourceMesh* outputMesh)
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