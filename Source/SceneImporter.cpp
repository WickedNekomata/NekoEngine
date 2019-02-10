#include "GameMode.h"

#include "SceneImporter.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleGOs.h"
#include "ModuleTimeManager.h"
#include "ModuleResourceManager.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentBone.h"
#include "ComponentTypes.h"
#include "MaterialImporter.h"
#include "Resource.h"
#include "ResourceMesh.h"

#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"
#include "Assimp\include\version.h"

#pragma comment (lib, "Assimp\\libx86\\assimp-vc140-mt.lib")

#define T_PRE_ROT "PreRotation"
#define T_ROT "Rotation"
#define T_POST_ROT "PostRotation"
#define T_TRANSLATION "Translation"
#define T_SCALING "Scaling"

void myCallback(const char* msg, char* userData)
{
	DEPRECATED_LOG("%s", msg);
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

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(importFile) + 1, importFile); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	char* buffer;
	uint size = App->fs->Load(importFile, &buffer);
	if (size > 0)
	{
		DEPRECATED_LOG("SCENE IMPORTER: Successfully loaded Model '%s'", outputFile.data());
		ret = Import(buffer, size, outputFile, importSettings, metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
		DEPRECATED_LOG("SCENE IMPORTER: Could not load Model '%s'", outputFile.data());

	return ret;
}

bool SceneImporter::Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings, const char* metaFile) const
{
	bool ret = false;

	if (buffer == nullptr || size <= 0 || importSettings == nullptr)
	{
		assert(buffer != nullptr && size > 0 && importSettings != nullptr);
		return ret;
	}

	MeshImportSettings* meshImportSettings = (MeshImportSettings*)importSettings;

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

		std::list<uint> UUIDs;
		if (metaFile != nullptr && App->fs->Exists(metaFile))
			GetMeshesUUIDsFromMeta(metaFile, UUIDs);

		RecursivelyImportNodes(scene, rootNode, rootGameObject, nullptr, UUIDs);

		RecursiveProcessBones(scene, scene->mRootNode);

		aiReleaseImport(scene);

		// 2. Serialize the imported scene
		App->GOs->SerializeFromNode(dummy, outputFile);

		dummy->RecursiveForceAllResources(0);
		dummy->DestroyChildren();
		RELEASE(dummy);
	}

	return ret;
}

void SceneImporter::RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation, std::list<uint>& UUIDs) const
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
			if (UUIDs.size() > 0)
			{
				gameObject->meshRenderer->res = UUIDs.front();
				UUIDs.remove(UUIDs.front());
			}
			else				
				gameObject->meshRenderer->res = App->GenerateRandomNumber();


			GLfloat* vertices = nullptr;
			uint verticesSize = 0;

			uint* indices = nullptr;
			uint indicesSize = 0;

			GLfloat* normals = nullptr;
			uint normalsSize = 0;

			GLfloat* tangents = nullptr;
			uint tangentsSize = 0;

			GLfloat* bitangents = nullptr;
			uint bitangentsSize = 0;

			GLubyte* colors = nullptr;
			uint colorsSize = 0;

			GLfloat* texCoords = nullptr;
			uint texCoordsSize = 0;

			// Unique vertices
			verticesSize = nodeMesh->mNumVertices;
			vertices = new GLfloat[verticesSize * 3];
			memcpy(vertices, nodeMesh->mVertices, sizeof(GLfloat) * verticesSize * 3);

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
						DEPRECATED_LOG("WARNING, geometry face with != 3 indices!");
					}
					else
						memcpy(&indices[j * 3], nodeMesh->mFaces[j].mIndices, 3 * sizeof(uint));
				}
			}

			relations[node] = gameObject;

			// Bones / Animations stuff
			if (nodeMesh->HasBones())
			{
				int num = nodeMesh->mNumBones;
				for (int k = 0; k < num; ++k)
				{
					if (!root_bone)
						root_bone = gameObject;

					ComponentMesh* mesh_cmp = (ComponentMesh*)gameObject->GetComponentByType(ComponentTypes::MeshComponent);
					
					bones[nodeMesh->mBones[k]->mName.C_Str()] = nodeMesh->mBones[k];
					mesh_bone[nodeMesh->mBones[k]] = mesh_cmp->res;
				}
			}

			// Normals
			if (nodeMesh->HasNormals())
			{
				normalsSize = verticesSize;
				normals = new GLfloat[normalsSize * 3];
				memcpy(normals, nodeMesh->mNormals, sizeof(GLfloat) * normalsSize * 3);
			}
			
			// Tangents and Bitangents
			if (nodeMesh->HasTangentsAndBitangents())
			{
				tangentsSize = verticesSize;
				tangents = new GLfloat[tangentsSize * 3];
				memcpy(tangents, nodeMesh->mTangents, sizeof(GLfloat) * tangentsSize * 3);
				
				bitangentsSize = verticesSize;
				bitangents = new GLfloat[bitangentsSize * 3];
				memcpy(bitangents, nodeMesh->mBitangents, sizeof(GLfloat) * bitangentsSize * 3);
			}
			
			// Color
			if (nodeMesh->HasVertexColors(0))
			{
				colorsSize = verticesSize;
				colors = new GLubyte[colorsSize * 4];
				memcpy(colors, nodeMesh->mColors, sizeof(GLubyte) * colorsSize * 4);
			}

			// Texture coords
			if (nodeMesh->HasTextureCoords(0))
			{
				texCoordsSize = verticesSize;
				texCoords = new GLfloat[texCoordsSize * 2];

				for (uint j = 0; j < verticesSize; ++j)
				{
					memcpy(&texCoords[j * 2], &nodeMesh->mTextureCoords[0][j].x, sizeof(GLfloat));
					memcpy(&texCoords[(j * 2) + 1], &nodeMesh->mTextureCoords[0][j].y, sizeof(GLfloat));
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
							uint UUID = 0;
							std::list<uint> UUIDs;
							if (!App->res->FindResourcesByFile(outputFile.data(), UUIDs))
								// If the texture is not a resource yet, import it
								UUID = App->res->ImportFile(outputFile.data());
							else
								UUID = UUIDs.front();

							if (UUID > 0)
								gameObject->materialRenderer->res[0].res = UUID;
						}
					}
				}
			}

			// Vertices + Normals + Colors + Texture Coords + Indices
			uint ranges[7] = { verticesSize, normalsSize, tangentsSize, bitangentsSize, colorsSize, texCoordsSize, indicesSize };

			uint size = sizeof(ranges) +
				sizeof(GLfloat) * verticesSize * 3 +
				sizeof(GLfloat) * normalsSize * 3 +
				sizeof(GLfloat) * tangentsSize * 3 +
				sizeof(GLfloat) * bitangentsSize * 3 +
				sizeof(GLubyte) * colorsSize * 4 +
				sizeof(GLfloat) * texCoordsSize * 2 +
				sizeof(uint) * indicesSize;

			char* data = new char[size];
			char* cursor = data;

			// 1. Store ranges
			uint bytes = sizeof(ranges);
			memcpy(cursor, ranges, bytes);

			cursor += bytes;

			// 2. Store vertices
			bytes = sizeof(GLfloat) * verticesSize * 3;
			memcpy(cursor, vertices, bytes);

			cursor += bytes;

			// 3. Store normals
			if (normalsSize > 0)
			{
				bytes = sizeof(GLfloat) * normalsSize * 3;
				memcpy(cursor, normals, bytes);

				cursor += bytes;
			}
			
			// 4. Store tangents
			if (tangentsSize > 0)
			{
				bytes = sizeof(GLfloat) * tangentsSize * 3;
				memcpy(cursor, tangents, bytes);

				cursor += bytes;
			}

			// 5. Store bitangents
			if (bitangentsSize > 0)
			{
				bytes = sizeof(GLfloat) * bitangentsSize * 3;
				memcpy(cursor, bitangents, bytes);

				cursor += bytes;
			}
			
			// 6. Store colors
			if (colorsSize > 0)
			{
				bytes = sizeof(GLubyte) * colorsSize * 4;
				memcpy(cursor, colors, bytes);

				cursor += bytes;
			}

			// 7. Store texture coords
			if (texCoordsSize > 0)
			{
				bytes = sizeof(GLfloat) * texCoordsSize * 2;
				memcpy(cursor, texCoords, bytes);

				cursor += bytes;
			}

			// 8. Store indices
			bytes = sizeof(GLuint) * indicesSize;
			memcpy(cursor, indices, bytes);

			std::string outputFileName = std::to_string(gameObject->meshRenderer->res);

			if (App->fs->SaveInGame(data, size, FileType::MeshFile, outputFileName) > 0)
			{
				DEPRECATED_LOG("SCENE IMPORTER: Successfully saved Mesh '%s' to own format", gameObject->GetName());
			}
			else
				DEPRECATED_LOG("SCENE IMPORTER: Could not save Mesh '%s' to own format", gameObject->GetName());

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
			RecursivelyImportNodes(scene, node->mChildren[i], parent, gameObject, UUIDs);
		else
			// Else, the current game object becomes the new parent for the next game object
			RecursivelyImportNodes(scene, node->mChildren[i], gameObject, nullptr, UUIDs);
	}
}

bool SceneImporter::GenerateMeta(std::list<Resource*> resources, std::string& outputMetaFile, const MeshImportSettings* meshImportSettings) const
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
	json_object_set_number(rootObject, "Time Created", lastModTime);

	JSON_Value* meshesArrayValue = json_value_init_array();
	JSON_Array* meshesArray = json_value_get_array(meshesArrayValue);
	for (std::list<Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		json_array_append_number(meshesArray, (*it)->GetUUID());
	json_object_set_value(rootObject, "Meshes", meshesArrayValue);

	JSON_Value* sceneImporterValue = json_value_init_object();
	JSON_Object* sceneImporterObject = json_value_get_object(sceneImporterValue);
	json_object_set_value(rootObject, "Scene Importer", sceneImporterValue);

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
		DEPRECATED_LOG("SCENE IMPORTER: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		DEPRECATED_LOG("SCENE IMPORTER: Could not save meta '%s'", outputMetaFile.data());
		return false;
	}

	RELEASE_ARRAY(buf);
	json_value_free(rootValue);

	return true;
}

bool SceneImporter::SetMeshUUIDsToMeta(const char* metaFile, std::list<uint> UUIDs) const
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
		//CONSOLE_LOG("SCENE IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		DEPRECATED_LOG("SCENE IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	JSON_Value* meshesArrayValue = json_value_init_array();
	JSON_Array* meshesArray = json_value_get_array(meshesArrayValue);
	for (std::list<uint>::const_iterator it = UUIDs.begin(); it != UUIDs.end(); ++it)
		json_array_append_number(meshesArray, *it);
	json_object_set_value(rootObject, "Meshes", meshesArrayValue);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);

	RELEASE_ARRAY(buffer);

	char* newBuffer = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, newBuffer, sizeBuf);

	size = App->fs->Save(metaFile, newBuffer, sizeBuf);
	if (size > 0)
	{
		DEPRECATED_LOG("SCENE IMPORTER: Successfully saved meta '%s' and set its UUIDs", metaFile);
	}
	else
	{
		DEPRECATED_LOG("SCENE IMPORTER: Could not save meta '%s' nor set its UUIDs", metaFile);
		return false;
	}

	RELEASE_ARRAY(newBuffer);
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
		//CONSOLE_LOG("SCENE IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		DEPRECATED_LOG("SCENE IMPORTER: Could not load meta '%s'", metaFile);
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
		//CONSOLE_LOG("SCENE IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		DEPRECATED_LOG("SCENE IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	JSON_Object* sceneImporterObject = json_object_get_object(rootObject, "Scene Importer");

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

	RELEASE_ARRAY(buffer);

	char* newBuffer = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, newBuffer, sizeBuf);

	size = App->fs->Save(metaFile, newBuffer, sizeBuf);
	if (size > 0)
	{
		DEPRECATED_LOG("SCENE IMPORTER: Successfully saved meta '%s' and set its mesh import settings", metaFile);
	}
	else
	{
		DEPRECATED_LOG("SCENE IMPORTER: Could not save meta '%s' nor set its mesh import settings", metaFile);
		return false;
	}

	RELEASE_ARRAY(newBuffer);
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
		//CONSOLE_LOG("SCENE IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		DEPRECATED_LOG("SCENE IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	meshImportSettings->metaFile = metaFile;

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	JSON_Object* sceneImporterObject = json_object_get_object(rootObject, "Scene Importer");
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

bool SceneImporter::Load(const char* exportedFile, ResourceMesh* outputMesh) const
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
		DEPRECATED_LOG("SCENE IMPORTER: Successfully loaded Mesh '%s' (own format)", exportedFile);
		ret = Load(buffer, size, outputMesh);
		RELEASE_ARRAY(buffer);
	}
	else
		DEPRECATED_LOG("SCENE IMPORTER: Could not load Mesh '%s' (own format)", exportedFile);

	return ret;
}

bool SceneImporter::Load(const void* buffer, uint size, ResourceMesh* outputMesh) const
{
	if (buffer == nullptr || size <= 0 || outputMesh == nullptr)
	{
		assert(buffer != nullptr && size > 0 && outputMesh != nullptr);
		return false;
	}

	char* cursor = (char*)buffer;

	// Vertices + Normals + tangents + bitangents + Colors + Texture Coords + Indices
	uint ranges[7];

	// 1. Load ranges
	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);

	cursor += bytes;

	outputMesh->verticesSize = ranges[0];
	outputMesh->indicesSize = ranges[6];
	uint normalsSize = ranges[1];
	uint tangentsSize = ranges[2];
	uint bitangentsSize = ranges[3];
	uint colorsSize = ranges[4];
	uint texCoordsSize = ranges[5];

	char* normalsCursor = cursor + ranges[0] * sizeof(GLfloat) * 3;
	char* tangentsCursor = normalsCursor + ranges[1] * sizeof(GLfloat) * 3;
	char* bitangentsCursor = tangentsCursor + ranges[2] * sizeof(GLfloat) * 3;
	char* colorCursor = bitangentsCursor + ranges[3] * sizeof(GLfloat) * 3;
	char* texCoordsCursor = colorCursor + ranges[4] * sizeof(GLubyte) * 4;

	outputMesh->vertices = new Vertex[outputMesh->verticesSize];

	for (uint i = 0; i < outputMesh->verticesSize; ++i)
	{
		// 2. Load vertices
		bytes = sizeof(GLfloat) * 3;
		memcpy(outputMesh->vertices[i].position, cursor, bytes);

		cursor += bytes;

		// 3. Load normals
		if (normalsSize > 0)
		{
			bytes = sizeof(GLfloat) * 3;
			memcpy(outputMesh->vertices[i].normal, normalsCursor, bytes);

			normalsCursor += bytes;
		}

		// 4. Load tangents
		if (tangentsSize > 0)
		{
			bytes = sizeof(GLfloat) * 3;
			memcpy(outputMesh->vertices[i].tangent, tangentsCursor, bytes);

			tangentsCursor += bytes;
		}

		// 5. Load bitangents
		if (bitangentsSize > 0)
		{
			bytes = sizeof(GLfloat) * 3;
			memcpy(outputMesh->vertices[i].bitangent, bitangentsCursor, bytes);

			bitangentsCursor += bytes;
		}

		// 6. Load colors
		if (colorsSize > 0)
		{
			bytes = sizeof(GLubyte) * 4;
			memcpy(outputMesh->vertices[i].color, colorCursor, bytes);

			colorCursor += bytes;
		}

		// 7. Load texture coords
		if (texCoordsSize > 0)
		{
			bytes = sizeof(GLfloat) * 2;
			memcpy(outputMesh->vertices[i].texCoord, texCoordsCursor, bytes);

			texCoordsCursor += bytes;
		}
	}

	// 8. Load indices
	cursor = texCoordsCursor;

	bytes = sizeof(GLuint) * outputMesh->indicesSize;
	outputMesh->indices = new GLuint[outputMesh->indicesSize];
	memcpy(outputMesh->indices, cursor, bytes);

	DEPRECATED_LOG("SCENE IMPORTER: New mesh loaded with: %u vertices and %u indices", outputMesh->verticesSize, outputMesh->indicesSize);

	return true;
}

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
	ResourceMesh::GenerateVBO(VBO, vertices, verticesSize);
	ResourceMesh::GenerateIBO(defaultPlaneIBO, indices, defaultPlaneIndicesSize);
	ResourceMesh::GenerateVAO(defaultPlaneVAO, VBO);
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

void SceneImporter::GetDefaultPlane(uint& defaultPlaneVAO, uint& defaultPlaneIBO, uint& defaultPlaneIndicesSize) const
{
	defaultPlaneVAO = this->defaultPlaneVAO;
	defaultPlaneIBO = this->defaultPlaneIBO;
	defaultPlaneIndicesSize = this->defaultPlaneIndicesSize;
}

void SceneImporter::RecursiveProcessBones(const aiScene * scene, const aiNode * node) const
{
	std::map<std::string, aiBone*>::iterator it = bones.find(node->mName.C_Str());

	if (it != bones.end())
	{
		aiBone* bone = it->second;

		GameObject* go = relations[node];
		/*ComponentBone* comp_bone = (ComponentBone*)go->CreateComponent(Component::component_type::COMPONENT_BONE);

		std::string output;
		UID bone_uid = App->resources->bone_importer->Import(bone, mesh_bone[bone], output);

		if (go->GetParent() == nullptr ||
			(go->GetParent() && !go->GetParent()->FindComponentByType(Component::component_type::COMPONENT_BONE)))
			bone_root_uid = go->GetUUID();


		comp_bone->SetResource(bone_uid);
		imported_bones[node->mName.C_Str()] = bone_uid;
		TR_LOG("->-> Added Bone component and created bone resource");*/
	}

	for (uint i = 0; i < node->mNumChildren; ++i)
		RecursiveProcessBones(scene, node->mChildren[i]);
}