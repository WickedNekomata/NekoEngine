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
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded Model '%s'", name.data());
		ret = Import(buffer, size, outputFileName);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SCENE IMPORTER: Could not load Model '%s'", name.data());

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

		// 1. Import the scene
		const aiNode* rootNode = scene->mRootNode;

		GameObject* dummy = new GameObject("Dummy", nullptr);
		GameObject* rootGameObject = new GameObject(rootNode->mName.data, dummy); // Root game object will never be a transformation

		RecursivelyImportNodes(scene, rootNode, rootGameObject, nullptr);
		aiReleaseImport(scene);

		// 2. Serialize the imported scene
		App->GOs->SerializeFromNode(dummy, outputFileName);

		dummy->DestroyChildren();
		RELEASE(dummy);
	}

	return ret;
}

void SceneImporter::RecursivelyImportNodes(const aiScene* scene, const aiNode* node, const GameObject* parent, const GameObject* transformation)
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

		std::string outputFileName = std::to_string(gameObject->meshRenderer->res);

		if (App->filesystem->SaveInLibrary(data, size, FileType::MeshFile, outputFileName) > 0)
		{
			CONSOLE_LOG("SCENE IMPORTER: Successfully saved Mesh '%s' to own format", gameObject->GetName());
		}
		else
			CONSOLE_LOG("SCENE IMPORTER: Could not save Mesh '%s' to own format", gameObject->GetName());
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
	/*
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
	*/
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

bool SceneImporter::GetMeshesUUIDsFromJson(char* fileName, std::list<uint>& uuids)
{
	char* buffer;
	uint size = App->filesystem->LoadFromLibrary(fileName, &buffer, FileType::SceneFile);
	if (size > 0)
	{
		CONSOLE_LOG("Scene Serialization: Successfully loaded Scene '%s' (own format)", fileName);
	}
	else
	{
		CONSOLE_LOG("Scene Serialization: Could not load Scene '%s' (own format)", fileName);
		return false;
	}

	JSON_Value* root_value;
	JSON_Array* gameObjectsArray;
	JSON_Object* gObject;

	/* parsing json and validating output */
	root_value = json_parse_string(buffer);
	if (json_value_get_type(root_value) != JSONArray)
		return false;

	gameObjectsArray = json_value_get_array(root_value);

	for (int i = 0; i < json_array_get_count(gameObjectsArray); i++) {
		gObject = json_array_get_object(gameObjectsArray, i);

		JSON_Array* jsonComponents = json_object_get_array(gObject, "jsonComponents");
		JSON_Object* cObject;

		for (int i = 0; i < json_array_get_count(jsonComponents); i++) {
			cObject = json_array_get_object(jsonComponents, i);
			if ((ComponentType)(int)json_object_get_number(cObject, "Type") == ComponentType::Mesh_Component)
				uuids.push_back(json_object_get_number(cObject, "ResourceMesh"));
		}
	}

	RELEASE_ARRAY(buffer);
	json_value_free(root_value);

	return true;
}

bool SceneImporter::Load(const char* exportedFileName, ResourceMesh* outputMesh)
{
	bool ret = false;

	char* buffer;
	uint size = App->filesystem->LoadFromLibrary(exportedFileName, &buffer, FileType::MeshFile);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded Mesh '%s' (own format)", exportedFileName);
		ret = Load(buffer, size, outputMesh);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("SCENE IMPORTER: Could not load Mesh '%s' (own format)", exportedFileName);

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