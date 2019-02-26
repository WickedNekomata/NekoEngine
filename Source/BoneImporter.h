#ifndef __BONE_IMPORTER_H__
#define __BONE_IMPORTER_H__

#include "Globals.h"

#include <string>

struct aiBone;
class ResourceBone;
class ResourceBoneData;
class GameObject;

class BoneImporter
{
public:

	BoneImporter();
	~BoneImporter();

	uint GenerateResourceFromFile(mutable const char* file_path,mutable uint uid_to_force = 0u);

	uint Import(mutable aiBone* new_bone,mutable uint mesh,mutable std::string& output, mutable GameObject* go) const;

	bool SaveBone(mutable const ResourceBoneData* bone,  mutable std::string& output) const;

};

#endif // __BONE_IMPORTER_H__