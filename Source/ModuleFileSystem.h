#ifndef __MODULE_FILE_SYSTEM_H__
#define __MODULE_FILE_SYSTEM_H__

#include "Module.h"

enum FileType
{
	MeshFile,
	TextureFile
};

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(bool start_enabled = true);
	virtual ~ModuleFileSystem();

	bool Init(JSON_Object* jObject);

	bool CleanUp();

	bool CreateDir(const char* dirName) const;
	bool AddPath(const char* newDir, const char* mountPoint = nullptr);

	const char* GetBasePath() const;
	const char* GetReadPaths() const;
	const char* GetWritePath() const;

	uint SaveUnique(const char* fileName, const void* buffer, uint size, FileType fileType) const;
	uint Save(const char* file, const void* buffer, uint size, bool append = false) const;	// file means PHYSFS path + file
	uint Load(const char* file, char** buffer) const;	// file means PHYSFS path + file
};

#endif