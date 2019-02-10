#ifndef __BONE_IMPORTER_H__
#define __BONE_IMPORTER_H__

#include "Importer.h"
#include "Globals.h"

class aiBone;
class ResourceBone;

struct BoneImportSettings : public ImportSettings
{
	std::string metaFile;
};

class BoneImporter : public Importer
{
public:

	BoneImporter();
	~BoneImporter();

	bool Import(const char* file_path, std::string& output_file); // TODO SOLVE THIS

	bool Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const;
	bool Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings, const char* metaFile) const;

	uint GenerateResourceFromFile(const char* file_path, uint uid_to_force = 0u);

	uint Import(mutable aiBone* new_bone, mutable uint mesh,mutable std::string& output) const;

	bool SaveBone(const ResourceBone* bone, std::string& output) const;

};

#endif // __BONE_IMPORTER_H__