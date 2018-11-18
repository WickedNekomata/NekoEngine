#ifndef __MODULE_FILE_SYSTEM_H__
#define __MODULE_FILE_SYSTEM_H__

#include "Module.h"
#include "PerfTimer.h"

#include <string>
#include <map>

#define DIR_LIBRARY "Library"
#define DIR_LIBRARY_MESHES "Library/Meshes"
#define DIR_LIBRARY_MATERIALS "Library/Materials"
#define DIR_ASSETS "Assets"
#define DIR_ASSETS_SCENES "Assets/Scenes"

#define IS_SCENE(extension) strcmp(extension, EXTENSION_SCENE) == 0
#define IS_META(extension) strcmp(extension, EXTENSION_META) == 0

enum FileType
{
	NoType,

	MeshFile,
	TextureFile,

	SceneFile,

	MetaFile
};

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(bool start_enabled = true);
	~ModuleFileSystem();
	bool Start();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	bool CreateDir(const char* dirName) const;
	bool AddPath(const char* newDir, const char* mountPoint = nullptr);
	bool DeleteFileOrDir(const char* path);

	const char* GetBasePath() const;
	const char* GetReadPaths() const;
	const char* GetWritePath() const;
	const char** GetFilesFromDir(const char* dir) const;

	void RecursiveGetFilesFromDir(const char* dir, std::string& path, std::map<std::string, uint>& outputFiles);

	bool IsDirectory(const char* file) const;
	bool Exists(const char* file) const;
	bool RecursiveExists(const char* fileName, const char* dir, std::string& path);

	int GetLastModificationTime(const char* file) const;

	void GetFileName(const char* file, std::string& fileName, bool extension = false) const;
	void GetExtension(const char* file, std::string& extension) const;
	void GetPath(const char* file, std::string& path) const;

	uint Copy(const char* file, const char* dir, std::string& outputFile) const;

	uint SaveInLibrary(char* buffer, uint size, FileType fileType, std::string& outputFile) const;
	uint Save(const char* file, char* buffer, uint size, bool append = false) const;

	uint Load(const char* file, char** buffer) const;

	// ----- Resource Manager -----

	bool AddMeta(const char* metaFile, uint lastModTime);
	bool DeleteMeta(const char* metaFile);

	std::map<std::string, uint> GetFilesInAssets() const;

	void CheckAssets();

private:

	std::map<std::string, uint> metas;
	std::map<std::string, uint> filesInAssets;
};

#endif