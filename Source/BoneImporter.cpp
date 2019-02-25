#include "BoneImporter.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "Assimp/include/mesh.h"

#include "ModuleResourceManager.h"
#include "ResourceBone.h"
#include "ComponentBone.h"
#include "GameObject.h"

BoneImporter::BoneImporter()
{
}

BoneImporter::~BoneImporter()
{
}

uint BoneImporter::Import(mutable aiBone* new_bone, mutable uint mesh, mutable std::string& output, mutable GameObject* go) const
{
	bool ret = false;

	if (new_bone == nullptr)
		return ret;

	// TODO_G : Now we dont create resources here uwu

	// Temporary object to make the load/Save process
	go->cmp_bone->res = App->GenerateRandomNumber();
	std::string outputFile = std::to_string(go->cmp_bone->res);
	ResourceBoneData res_data;
	res_data.mesh_uid = mesh;
	res_data.bone_weights_size = new_bone->mNumWeights;
	memcpy(res_data.offset_matrix.v, &new_bone->mOffsetMatrix.a1, sizeof(float) * 16);

	res_data.bone_weights_indices = new uint[res_data.bone_weights_size];
	res_data.bone_weights = new float[res_data.bone_weights_size];

	for (uint k = 0; k < res_data.bone_weights_size; ++k)
	{
		res_data.bone_weights_indices[k] = new_bone->mWeights[k].mVertexId;
		res_data.bone_weights[k] = new_bone->mWeights[k].mWeight;
	}
	//CONTINUE HERE TODO_G
	return 0u;
	/*bone->mesh_uid = mesh;
	bone->bone_weights_size = new_bone->mNumWeights;
	memcpy(bone->offset_matrix.v, &new_bone->mOffsetMatrix.a1, sizeof(float) * 16);

	bone->bone_weights_indices = new uint[bone->bone_weights_size];
	bone->bone_weights = new float[bone->bone_weights_size];

	for (uint k = 0; k < bone->bone_weights_size; ++k)
	{
		bone->bone_weights_indices[k] = new_bone->mWeights[k].mVertexId;
		bone->bone_weights[k] = new_bone->mWeights[k].mWeight;
	}

	if(SaveBone(bone, output))
		DEPRECATED_LOG("Saved bone correctly in path: [%s]", output.c_str())
	else
		DEPRECATED_LOG("Error saving bone in path: [%s]", output.c_str());

	bone->exportedFile = output;

	return bone->GetUUID();*/
}

uint BoneImporter::GenerateResourceFromFile(mutable const char * file_path, mutable uint uid_to_force)
{
	// Reading file
	char* buffer = nullptr;
	App->fs->Load(file_path,&buffer);

	// Checking for errors
	if (buffer == nullptr)
	{
		DEPRECATED_LOG("BoneImporter: Unable to open file...");
		return false;
	}

	return 0u;
	// TODO_G : Now we dont create resources here uwu

	/*char* cursor = buffer;
	ResourceBone* resource = (ResourceBone*)App->res->CreateNewResource(ResourceTypes::BoneResource, uid_to_force);

	// Load mesh UID
	uint bytes = sizeof(uint);
	memcpy(&resource->mesh_uid, cursor, bytes);

	// Load offset matrix
	cursor += bytes;
	bytes = sizeof(resource->offset_matrix);
	memcpy(resource->offset_matrix.v, cursor, bytes);

	// Load num_weigths
	cursor += bytes;
	bytes = sizeof(resource->bone_weights_size);
	memcpy(&resource->bone_weights_size, cursor, bytes);

	// Allocate mem for indices and weights
	resource->bone_weights_indices = new uint[resource->bone_weights_size];
	resource->bone_weights = new float[resource->bone_weights_size];

	// Read indices
	cursor += bytes;
	bytes = sizeof(uint) * resource->bone_weights_size;
	memcpy(resource->bone_weights_indices, cursor, bytes);

	// Read weigths
	cursor += bytes;
	bytes = sizeof(float) * resource->bone_weights_size;
	memcpy(resource->bone_weights, cursor, bytes);

	RELEASE_ARRAY(buffer);

	return resource->GetUUID();*/
}

bool BoneImporter::SaveBone(mutable const ResourceBoneData* bone, mutable std::string& output) const
{
	bool ret = false;

	// Format: mesh UID + 16 float matrix + num_weigths uint + indices uint * num_weight + weight float * num_weights
	uint size = sizeof(bone->mesh_uid);
	size += sizeof(bone->offset_matrix);
	size += sizeof(bone->bone_weights_size);
	size += sizeof(uint) * bone->bone_weights_size;
	size += sizeof(float) * bone->bone_weights_size;

	// allocate mem
	char* data = new char[size];
	char* cursor = data;

	// store mesh UID
	uint bytes = sizeof(bone->mesh_uid);
	memcpy(cursor, &bone->mesh_uid, bytes);

	// store offset matrix
	cursor += bytes;
	bytes = sizeof(bone->offset_matrix);
	memcpy(cursor, &bone->offset_matrix.v, bytes);

	// store num_weights
	cursor += bytes;
	bytes = sizeof(bone->bone_weights_size);
	memcpy(cursor, &bone->bone_weights_size, bytes);

	// store indices
	cursor += bytes;
	bytes = sizeof(uint) * bone->bone_weights_size;
	memcpy(cursor, bone->bone_weights_indices, bytes);

	// store weights
	cursor += bytes;
	bytes = sizeof(float) * bone->bone_weights_size;
	memcpy(cursor, bone->bone_weights, bytes);

	if (App->fs->SaveInGame((char*)data, size, FileTypes::BoneFile, output) > 0)
		ret = true;

	// Deleting useless data
	RELEASE_ARRAY(data);

	return ret;
}