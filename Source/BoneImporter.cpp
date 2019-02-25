#include "BoneImporter.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "Assimp/include/mesh.h"

#include "ModuleResourceManager.h"
#include "ResourceBone.h"
#include "ComponentBone.h"
#include "GameObject.h"
#include <cstdio>

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

	go->cmp_bone->res = App->GenerateRandomNumber();

	std::string outputFile = std::to_string(go->cmp_bone->res);

	ResourceData data;
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
	

	if(SaveBone(data,res_data, outputFile, true /* TODO_G: WHAT? */))
		DEPRECATED_LOG("Saved bone correctly in path: [%s]", output.c_str())
	else
		DEPRECATED_LOG("Error saving bone in path: [%s]", output.c_str());

	return go->cmp_bone->res;
}

Resource* BoneImporter::GenerateResourceFromFile(mutable const char * file_path, mutable uint uid_to_force)
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

	ResourceData data;
	ResourceBoneData bone_data;

	char* cursor = buffer;
	
	// Load mesh UID
	uint bytes = sizeof(uint);
	memcpy(&bone_data.mesh_uid, cursor, bytes);

	// Load offset matrix
	cursor += bytes;
	bytes = sizeof(bone_data.offset_matrix);
	memcpy(bone_data.offset_matrix.v, cursor, bytes);

	// Load num_weigths
	cursor += bytes;
	bytes = sizeof(bone_data.bone_weights_size);
	memcpy(&bone_data.bone_weights_size, cursor, bytes);

	// Allocate mem for indices and weights
	bone_data.bone_weights_indices = new uint[bone_data.bone_weights_size];
	bone_data.bone_weights = new float[bone_data.bone_weights_size];

	// Read indices
	cursor += bytes;
	bytes = sizeof(uint) * bone_data.bone_weights_size;
	memcpy(bone_data.bone_weights_indices, cursor, bytes);

	// Read weigths
	cursor += bytes;
	bytes = sizeof(float) * bone_data.bone_weights_size;
	memcpy(bone_data.bone_weights, cursor, bytes);

	RELEASE_ARRAY(buffer);

	ResourceBone* resource = (ResourceBone*)App->res->CreateResource(ResourceTypes::BoneResource, data, &bone_data, uid_to_force);

	return resource;
}

bool BoneImporter::SaveBone(mutable ResourceData& res_data, mutable ResourceBoneData& bone_data,mutable std::string& outputFile,mutable bool overwrite) const
{
	bool ret = false;

	// TODO: uhm ...
	if (overwrite)
		outputFile = res_data.file;
	else
		outputFile = res_data.name;

	// Format: mesh UID + 16 float matrix + num_weigths uint + indices uint * num_weight + weight float * num_weights
	uint size = sizeof(bone_data.mesh_uid);
	size += sizeof(bone_data.offset_matrix);
	size += sizeof(bone_data.bone_weights_size);
	size += sizeof(uint) * bone_data.bone_weights_size;
	size += sizeof(float) * bone_data.bone_weights_size;

	// allocate mem
	char* data = new char[size];
	char* cursor = data;

	// store mesh UID
	uint bytes = sizeof(bone_data.mesh_uid);
	memcpy(cursor, &bone_data.mesh_uid, bytes);

	// store offset matrix
	cursor += bytes;
	bytes = sizeof(bone_data.offset_matrix);
	memcpy(cursor, &bone_data.offset_matrix.v, bytes);

	// store num_weights
	cursor += bytes;
	bytes = sizeof(bone_data.bone_weights_size);
	memcpy(cursor, &bone_data.bone_weights_size, bytes);

	// store indices
	cursor += bytes;
	bytes = sizeof(uint) * bone_data.bone_weights_size;
	memcpy(cursor, bone_data.bone_weights_indices, bytes);

	// store weights
	cursor += bytes;
	bytes = sizeof(float) * bone_data.bone_weights_size;
	memcpy(cursor, bone_data.bone_weights, bytes);

	if (App->fs->SaveInGame((char*)data, size, FileTypes::BoneFile, outputFile) > 0)
		ret = true;

	// Deleting useless data
	RELEASE_ARRAY(data);

	return ret;
}