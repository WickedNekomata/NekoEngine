#ifndef __MODULE_FILE_SYSTEM_H__
#define __MODULE_FILE_SYSTEM_H__

#include "Module.h"

enum FileType
{
	MeshFile,
	TextureFile,
	SceneFile
};

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(bool start_enabled = true);
	~ModuleFileSystem();

	bool CleanUp();

	bool CreateDir(const char* dirName) const;
	bool AddPath(const char* newDir, const char* mountPoint = nullptr);

	const char* GetBasePath() const;
	const char* GetReadPaths() const;
	const char* GetWritePath() const;

	const char** GetFilesFromDir(const char* dir) const;

	uint SaveInLibrary(char* buffer, uint size, FileType fileType, std::string& outputFileName, uint ID = 0) const;
	uint Save(const char* filePath, char* buffer, uint size, bool append = false) const;

	uint LoadFromLibrary(const char* fileName, char** buffer, FileType fileType) const;
	uint Load(const char* filePath, char** buffer) const;

	bool Exists(const char* filePath) const;
	bool ExistsInAssets(const char* fileNameWithExtension, FileType fileType, std::string& outputFilePath) const;
	bool IsDirectory(const char* file) const;

	const char* GetFileNameFromPath(const char* path) const;
};

#endif