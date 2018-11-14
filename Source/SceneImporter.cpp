#include "GameMode.h"

#include "SceneImporter.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleGOs.h"
#include "ModuleTimeManager.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentTypes.h"
#include "MaterialImporter.h"
#include "Resource.h"
#include "ResourceMesh.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"
#include "Assimp/include/version.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

#define T_PRE_ROT "PreRotation"
#define T_ROT "Rotation"
#define T_POST_ROT "PostRotation"
#define T_TRANSLATION "Translation"
#define T_SCALING "Scaling"

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

bool SceneImporter::Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const
{
	bool ret = false;

	if (importFile == nullptr || importSettings == nullptr)
	{
		assert(importFile != nullptr && importSettings != nullptr);
		return ret;
	}

	std::string importFileName;
	App->fs->GetFileName(importFile, importFileName);
	outputFile = importFileName.data();

	char* buffer;
	uint size = App->fs->Load(importFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded Model '%s'", outputFile.data());
		ret = Import(buffer, size, outputFile, importSettings);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SCENE IMPORTER: Could not load Model '%s'", outputFile.data());

	return ret;
}

bool SceneImporter::Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings) const
{
	bool ret = false;

	if (buffer == nullptr || size <= 0 || importSettings == nullptr)
	{
		assert(buffer != nullptr && size > 0 && importSettings != nullptr);
		return ret;
	}

	MeshImportSettings* meshImportSettings = (MeshImportSettings*)importSettings;
	//TODO FILE SCALE
	uint postProcessingFlags = 0;

	switch (meshImportSettings->postProcessConfiguration)
	{
	case MeshImportSettings::MeshPostProcessConfiguration::TARGET_REALTIME_FAST:
		postProcessingFlags |= aiProcessPreset_TargetRealtime_Fast;
		break;
	case MeshImportSettings::MeshPostProcessConfiguration::TARGET_REALTIME_QUALITY:
		postProcessingFlags |= aiProcessPreset_TargetRealtime_Quality;
		break;
	case MeshImportSettings::MeshPostProcessConfiguration::TARGET_REALTIME_MAX_QUALITY:
		postProcessingFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
		break;
	case MeshImportSettings::MeshPostProcessConfiguration::CUSTOM:
		if (meshImportSettings->calcTangentSpace)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_CalcTangentSpace;
		if (meshImportSettings->genNormals)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_GenNormals;
		else if (meshImportSettings->genSmoothNormals)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
		if (meshImportSettings->joinIdenticalVertices)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;
		if (meshImportSettings->triangulate)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_Triangulate;
		if (meshImportSettings->genUVCoords)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_GenUVCoords;
		if (meshImportSettings->sortByPType)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_SortByPType;
		if (meshImportSettings->improveCacheLocality)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_ImproveCacheLocality;
		if (meshImportSettings->limitBoneWeights)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_LimitBoneWeights;
		if (meshImportSettings->removeRedundantMaterials)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_RemoveRedundantMaterials;
		if (meshImportSettings->splitLargeMeshes)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_SplitLargeMeshes;
		if (meshImportSettings->findDegenerates)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_FindDegenerates;
		if (meshImportSettings->findInvalidData)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_FindInvalidData;
		if (meshImportSettings->findInstances)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_FindInstances;
		if (meshImportSettings->validateDataStructure)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_ValidateDataStructure;
		if (meshImportSettings->optimizeMeshes)
			postProcessingFlags |= aiPostProcessSteps::aiProcess_OptimizeMeshes;
		break;
	}

	const aiScene* scene = aiImportFileFromMemory((const char*)buffer, size, postProcessingFlags, nullptr);

	if (scene != nullptr)
	{
		ret = true;

		// 1. Import the scene
		const aiNode* rootNode = scene->mRootNode;

		GameObject* dummy = new GameObject("Dummy", nullptr);
		GameObject* rootGameObject = new GameObject(rootNode->mName.data, dummy); // Root game object will never be a transformation

		RecursivelyImportNodes(scene, rootNode, rootGameObject, nullptr);
		aiReleaseImport(scene);

		// 2. Serialize the imported scene
		App->GOs->SerializeFromNode(dummy, outputFile);

		dummy->DestroyChildren();
		RELEASE(dummy);
	}

	return ret;
}

void SceneImporter::RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation) const
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
	if (!isTransformation && node->mNumMeshes > 0) // Transformations don't contain meshes
	{
		aiMesh* nodeMesh = scene->mMeshes[node->mMeshes[0]];

		gameObject->AddComponent(ComponentType::Mesh_Component);
		gameObject->meshRenderer->res = App->GenerateRandomNumber();

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
		/*
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
						gameObject->AddComponent(ComponentType::Material_Component);
						gameObject->materialRenderer->textures.push_back(texture);
					}
				}
			}
		}*/

		// Vertices + Indices + Texture Coords
		uint ranges[3] = { verticesSize, indicesSize, textureCoordsSize };

		uint size = sizeof(ranges) +
			sizeof(float) * verticesSize * 3 +
			sizeof(uint) * indicesSize +
			sizeof(float) * textureCoordsSize;

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

		// 3. Store indices
		bytes = sizeof(uint) * indicesSize;
		memcpy(cursor, indices, bytes);

		cursor += bytes;

		// 4. Store texture coords
		bytes = sizeof(float) * textureCoordsSize;
		memcpy(cursor, textureCoords, bytes);

		std::string outputFileName = std::to_string(gameObject->meshRenderer->res);

		if (App->fs->SaveInLibrary(data, size, FileType::MeshFile, outputFileName) > 0)
		{
			CONSOLE_LOG("SCENE IMPORTER: Successfully saved Mesh '%s' to own format", gameObject->GetName());
		}
		else
			CONSOLE_LOG("SCENE IMPORTER: Could not save Mesh '%s' to own format", gameObject->GetName());

		RELEASE_ARRAY(data);
		RELEASE_ARRAY(vertices);
		RELEASE_ARRAY(indices);
		RELEASE_ARRAY(textureCoords);
	}

	for (uint i = 0; i < node->mNumChildren; ++i)
	{
		// If the current game object is a transformation, keep its parent and pass it as the new transformation for the next game object
		if (isTransformation)
			RecursivelyImportNodes(scene, node->mChildren[i], parent, gameObject);
		// Else, the current game object becomes the new parent for the next game object
		else
			RecursivelyImportNodes(scene, node->mChildren[i], gameObject, nullptr);
	}
}

bool SceneImporter::GenerateMeta(std::list<Resource*>& resources, const MeshImportSettings* meshImportSettings, std::string& outputMetaFile) const
{
	if (resources.empty() || meshImportSettings == nullptr)
	{
		assert(!resources.empty() && meshImportSettings != nullptr);
		return false;
	}

	ResourceMesh* meshResource = (ResourceMesh*)resources.front();

	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	// Fill the JSON with data
	int lastModTime = App->fs->GetLastModificationTime(resources.front()->file.data());
	assert(lastModTime != -1);
	json_object_set_number(rootObject, "Time Created", lastModTime);

	JSON_Value* meshesArrayValue = json_value_init_array();
	JSON_Array* meshesArray = json_value_get_array(meshesArrayValue);
	for (std::list<Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		json_array_append_number(meshesArray, (*it)->GetUUID());
	json_object_set_value(rootObject, "Meshes", meshesArrayValue);

	JSON_Value* sceneImporterValue = json_value_init_object();
	JSON_Object* sceneImporterObject = json_value_get_object(sceneImporterValue);
	json_object_set_value(rootObject, "Scene Importer", sceneImporterValue);

	JSON_Value* scaleArrayValue = json_value_init_array();
	JSON_Array* scaleArray = json_value_get_array(scaleArrayValue);
	json_array_append_number(scaleArray, meshImportSettings->scale.x);
	json_array_append_number(scaleArray, meshImportSettings->scale.y);
	json_array_append_number(scaleArray, meshImportSettings->scale.z);
	json_object_set_value(sceneImporterObject, "Scale", scaleArrayValue);

	json_object_set_boolean(sceneImporterObject, "Use File Scale", meshImportSettings->useFileScale);
	json_object_set_number(sceneImporterObject, "Post Process Configuration", meshImportSettings->postProcessConfiguration);
	json_object_set_boolean(sceneImporterObject, "Calculate Tangent Space", meshImportSettings->calcTangentSpace);
	json_object_set_boolean(sceneImporterObject, "Generate Normals", meshImportSettings->genNormals);
	json_object_set_boolean(sceneImporterObject, "Generate Smooth Normals", meshImportSettings->genSmoothNormals);
	json_object_set_boolean(sceneImporterObject, "Join Identical Vertices", meshImportSettings->joinIdenticalVertices);
	json_object_set_boolean(sceneImporterObject, "Triangulate", meshImportSettings->triangulate);
	json_object_set_boolean(sceneImporterObject, "Generate UV Coordinates", meshImportSettings->genUVCoords);
	json_object_set_boolean(sceneImporterObject, "Sort By Primitive Type", meshImportSettings->sortByPType);
	json_object_set_boolean(sceneImporterObject, "Improve Cache Locality", meshImportSettings->improveCacheLocality);
	json_object_set_boolean(sceneImporterObject, "Limit Bone Weights", meshImportSettings->limitBoneWeights);
	json_object_set_boolean(sceneImporterObject, "Remove Redundant Materials", meshImportSettings->removeRedundantMaterials);
	json_object_set_boolean(sceneImporterObject, "Split Large Meshes", meshImportSettings->splitLargeMeshes);
	json_object_set_boolean(sceneImporterObject, "Find Degenerates", meshImportSettings->findDegenerates);
	json_object_set_boolean(sceneImporterObject, "Find Invalid Data", meshImportSettings->findInvalidData);
	json_object_set_boolean(sceneImporterObject, "Find Instances", meshImportSettings->findInstances);
	json_object_set_boolean(sceneImporterObject, "Validate Data Structure", meshImportSettings->validateDataStructure);
	json_object_set_boolean(sceneImporterObject, "Optimize Meshes", meshImportSettings->optimizeMeshes);

	// Build the path of the meta file
	outputMetaFile.append(meshResource->file.data());
	outputMetaFile.append(EXTENSION_META);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);

	uint size = App->fs->Save(outputMetaFile.data(), buf, sizeBuf);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG("SCENE IMPORTER: Could not save meta '%s'", outputMetaFile.data());
		return false;
	}

	RELEASE_ARRAY(buf);
	json_value_free(rootValue);

	return true;
}

bool SceneImporter::SetMeshImportSettingsToMeta(const char* metaFile, const MeshImportSettings* meshImportSettings) const
{
	if (metaFile == nullptr || meshImportSettings == nullptr)
	{
		assert(metaFile != nullptr && meshImportSettings != nullptr);
		return false;
	}

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("SCENE IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	JSON_Object* sceneImporterObject = json_object_get_object(rootObject, "Scene Importer");

	JSON_Array* scaleArray = json_object_get_array(rootObject, "Scale");
	json_array_append_number(scaleArray, meshImportSettings->scale.x);
	json_array_append_number(scaleArray, meshImportSettings->scale.y);
	json_array_append_number(scaleArray, meshImportSettings->scale.z);

	json_object_set_boolean(sceneImporterObject, "Use File Scale", meshImportSettings->useFileScale);
	json_object_set_number(sceneImporterObject, "Post Process Configuration", meshImportSettings->postProcessConfiguration);
	json_object_set_boolean(sceneImporterObject, "Calculate Tangent Space", meshImportSettings->calcTangentSpace);
	json_object_set_boolean(sceneImporterObject, "Generate Normals", meshImportSettings->genNormals);
	json_object_set_boolean(sceneImporterObject, "Generate Smooth Normals", meshImportSettings->genSmoothNormals);
	json_object_set_boolean(sceneImporterObject, "Join Identical Vertices", meshImportSettings->joinIdenticalVertices);
	json_object_set_boolean(sceneImporterObject, "Triangulate", meshImportSettings->triangulate);
	json_object_set_boolean(sceneImporterObject, "Generate UV Coordinates", meshImportSettings->genUVCoords);
	json_object_set_boolean(sceneImporterObject, "Sort By Primitive Type", meshImportSettings->sortByPType);
	json_object_set_boolean(sceneImporterObject, "Improve Cache Locality", meshImportSettings->improveCacheLocality);
	json_object_set_boolean(sceneImporterObject, "Limit Bone Weights", meshImportSettings->limitBoneWeights);
	json_object_set_boolean(sceneImporterObject, "Remove Redundant Materials", meshImportSettings->removeRedundantMaterials);
	json_object_set_boolean(sceneImporterObject, "Split Large Meshes", meshImportSettings->splitLargeMeshes);
	json_object_set_boolean(sceneImporterObject, "Find Degenerates", meshImportSettings->findDegenerates);
	json_object_set_boolean(sceneImporterObject, "Find Invalid Data", meshImportSettings->findInvalidData);
	json_object_set_boolean(sceneImporterObject, "Find Instances", meshImportSettings->findInstances);
	json_object_set_boolean(sceneImporterObject, "Validate Data Structure", meshImportSettings->validateDataStructure);
	json_object_set_boolean(sceneImporterObject, "Optimize Meshes", meshImportSettings->optimizeMeshes);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);
	json_serialize_to_buffer_pretty(rootValue, buffer, sizeBuf);

	size = App->fs->Save(metaFile, buffer, sizeBuf);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully saved meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("SCENE IMPORTER: Could not save meta '%s'", metaFile);
		return false;
	}

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}

bool SceneImporter::GetMeshesUUIDsFromMeta(const char* metaFile, std::list<uint>& UUIDs) const
{
	if (metaFile == nullptr)
	{
		assert(metaFile != nullptr);
		return false;
	}

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("SCENE IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	JSON_Array* meshesArray = json_object_get_array(rootObject, "Meshes");
	uint meshesArraySize = json_array_get_count(meshesArray);
	for (uint i = 0; i < meshesArraySize; i++)
		UUIDs.push_back(json_array_get_number(meshesArray, i));

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}

bool SceneImporter::GetMeshImportSettingsFromMeta(const char* metaFile, MeshImportSettings* meshImportSettings) const
{
	if (metaFile == nullptr || meshImportSettings == nullptr)
	{
		assert(metaFile != nullptr && meshImportSettings != nullptr);
		return false;
	}

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("SCENE IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	JSON_Array* meshesArray = json_object_get_array(rootObject, "Meshes");
	meshImportSettings->scale.x = json_array_get_number(meshesArray, 0);
	meshImportSettings->scale.y = json_array_get_number(meshesArray, 1);
	meshImportSettings->scale.z = json_array_get_number(meshesArray, 2);

	JSON_Object* sceneImporterObject = json_object_get_object(rootObject, "Scene Importer");
	meshImportSettings->useFileScale = json_object_get_boolean(sceneImporterObject, "Use File Scale");
	meshImportSettings->postProcessConfiguration = (MeshImportSettings::MeshPostProcessConfiguration)(uint)json_object_get_number(sceneImporterObject, "Post Process Configuration");
	meshImportSettings->calcTangentSpace = json_object_get_boolean(sceneImporterObject, "Calculate Tangent Space");
	meshImportSettings->genNormals = json_object_get_boolean(sceneImporterObject, "Generate Normals");
	meshImportSettings->genSmoothNormals = json_object_get_boolean(sceneImporterObject, "Generate Smooth Normals");
	meshImportSettings->joinIdenticalVertices = json_object_get_boolean(sceneImporterObject, "Join Identical Vertices");
	meshImportSettings->triangulate = json_object_get_boolean(sceneImporterObject, "Triangulate");
	meshImportSettings->genUVCoords = json_object_get_boolean(sceneImporterObject, "Generate UV Coordinates");
	meshImportSettings->sortByPType = json_object_get_boolean(sceneImporterObject, "Sort By Primitive Type");
	meshImportSettings->improveCacheLocality = json_object_get_boolean(sceneImporterObject, "Improve Cache Locality");
	meshImportSettings->limitBoneWeights = json_object_get_boolean(sceneImporterObject, "Limit Bone Weights");
	meshImportSettings->removeRedundantMaterials = json_object_get_boolean(sceneImporterObject, "Remove Redundant Materials");
	meshImportSettings->splitLargeMeshes = json_object_get_boolean(sceneImporterObject, "Split Large Meshes");
	meshImportSettings->findDegenerates = json_object_get_boolean(sceneImporterObject, "Find Degenerates");
	meshImportSettings->findInvalidData = json_object_get_boolean(sceneImporterObject, "Find Invalid Data");
	meshImportSettings->findInstances = json_object_get_boolean(sceneImporterObject, "Find Instances");
	meshImportSettings->validateDataStructure = json_object_get_boolean(sceneImporterObject, "Validate Data Structure");
	meshImportSettings->optimizeMeshes = json_object_get_boolean(sceneImporterObject, "Optimize Meshes");

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}

bool SceneImporter::Load(const char* exportedFile, ResourceMesh* outputMesh)
{
	bool ret = false;

	if (exportedFile == nullptr || outputMesh == nullptr)
	{
		assert(exportedFile != nullptr && outputMesh != nullptr);
		return ret;
	}

	char* buffer;
	uint size = App->fs->Load(exportedFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded Mesh '%s' (own format)", exportedFile);
		ret = Load(buffer, size, outputMesh);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SCENE IMPORTER: Could not load Mesh '%s' (own format)", exportedFile);

	return ret;
}

bool SceneImporter::Load(const void* buffer, uint size, ResourceMesh* outputMesh)
{
	if (buffer == nullptr || size <= 0 || outputMesh == nullptr)
	{
		assert(buffer != nullptr && size > 0 && outputMesh != nullptr);
		return false;
	}

	char* cursor = (char*)buffer;

	// Vertices + Indices + Texture Coords
	uint ranges[3];

	// 1. Load ranges
	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);

	cursor += bytes;

	outputMesh->verticesSize = ranges[0];
	outputMesh->indicesSize = ranges[1];
	outputMesh->textureCoordsSize = ranges[2];

	// 2. Load vertices
	bytes = sizeof(float) * outputMesh->verticesSize * 3;
	outputMesh->vertices = new float[outputMesh->verticesSize * 3];
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

	// TODO: find the texture associated with the material 0 of the mesh

	CONSOLE_LOG("SCENE IMPORTER: New mesh loaded with: %u vertices, %u indices, %u texture coords", outputMesh->verticesSize, outputMesh->indicesSize, outputMesh->textureCoordsSize);

	return true;
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