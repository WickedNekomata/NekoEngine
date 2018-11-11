#ifndef __MODULE_FILE_SYSTEM_H__
#define __MODULE_FILE_SYSTEM_H__

#include "Module.h"

#define DIR_LIBRARY "Library"
#define DIR_LIBRARY_MESHES "Library/Meshes"
#define DIR_LIBRARY_MATERIALS "Library/Materials"
#define DIR_ASSETS_SCENES "Assets/Scenes"

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

	uint SaveInLibrary(char* buffer, uint size, FileType fileType, std::string& outputFileName) const;
	uint Save(const char* filePath, char* buffer, uint size, bool append = false) const;

	uint LoadFromLibrary(const char* fileName, char** buffer, FileType fileType) const;
	uint Load(const char* filePath, char** buffer) const;

	bool IsDirectory(const char* file) const;
	bool Exists(const char* file) const;

	void GetFileName(const char* file, std::string& fileName) const;
	void GetExtension(const char* file, std::string& extension) const;
	void GetPath(const char* file, std::string& path) const;
};

#endif