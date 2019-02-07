#ifndef __BONE_IMPORTER_H__
#define __BONE_IMPORTER_H__

#include "Importer.h"
#include "Globals.h"

class aiBone;
class ResourceBone;

class BoneImporter : public Importer
{
public:

	BoneImporter();
	~BoneImporter();

	bool Import(const char* file_path, std::string& output_file); // TODO SOLVE THIS

	uint GenerateResourceFromFile(const char* file_path, uint uid_to_force = 0u);

	uint Import(const aiBone* new_bone, uint mesh, std::string& output) const;

	bool SaveBone(const ResourceBone* bone, std::string& output) const;

};

#endif // __BONE_IMPORTER_H__