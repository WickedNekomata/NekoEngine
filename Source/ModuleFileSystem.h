#ifndef __MODULE_FILE_SYSTEM_H__
#define __MODULE_FILE_SYSTEM_H__

#include "Module.h"
#include "PerfTimer.h"

#include <vector>
#include <string>
#include <map>

#define DIR_LIBRARY "Library"
#define DIR_LIBRARY_MESHES "Library/Meshes"
#define DIR_LIBRARY_MATERIALS "Library/Materials"
#define DIR_ASSETS "Assets"
#define DIR_ASSETS_SCENES "Assets/Scenes"
#define DIR_ASSETS_SHADERS "Assets/Shaders"
#define DIR_ASSETS_SHADERS_OBJECTS "Assets/Shaders/Objects"
#define DIR_ASSETS_SHADERS_PROGRAMS "Assets/Shaders/Programs"

#define IS_SCENE(extension) strcmp(extension, EXTENSION_SCENE) == 0
#define IS_META(extension) strcmp(extension, EXTENSION_META) == 0

class Resource;
struct ImportSettings;

enum FileType
{
	NoFileType,

	MeshFile,
	TextureFile,

	SceneFile,

	VertexShaderObjectFile,
	FragmentShaderObjectFile,
	ShaderProgramFile,

	MetaFile
};

struct File
{
	std::string name;
	std::string path;
	bool isDirectory = false;
	std::vector<File*> children;
};

struct AssetsFile : public File
{
	uint lastModTime = 0;

	const ImportSettings* importSettings = nullptr; // meshes, textures
	const Resource* resource = nullptr; // shaders

	std::map<std::string, uint> UUIDs; // meshes, textures
};

struct LibraryFile : public File
{
	const Resource* resource = nullptr;
};

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(bool start_enabled = true);
	~ModuleFileSystem();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	bool CreateDir(const char* dirName) const;
	bool AddPath(const char* newDir, const char* mountPoint = nullptr) const;
	bool DeleteFileOrDir(const char* path) const;

	const char* GetBasePath() const;
	const char* GetReadPaths() const;
	const char* GetWritePath() const;
	const char** GetFilesFromDir(const char* dir) const;

	void RecursiveGetFilesFromAssets(AssetsFile* assetsFile, std::map<std::string, uint>& assetsFiles) const;
	void RecursiveGetFilesFromLibrary(LibraryFile* libraryFile) const;
	AssetsFile* GetRootAssetsFile() const;
	LibraryFile* GetRootLibraryFile() const;

	int GetLastModificationTime(const char* file) const;

	bool IsDirectory(const char* file) const;
	bool Exists(const char* file) const;
	bool RecursiveExists(const char* fileName, const char* dir, std::string& path) const;

	void GetFileName(const char* file, std::string& fileName, bool extension = false) const;
	void GetExtension(const char* file, std::string& extension) const;
	void GetPath(const char* file, std::string& path) const;

	uint Copy(const char* file, const char* dir, std::string& outputFile) const;

	uint SaveInGame(char* buffer, uint size, FileType fileType, std::string& outputFile, bool overwrite = false) const;
	uint Save(const char* file, char* buffer, uint size, bool append = false) const;

	uint Load(const char* file, char** buffer) const;

	bool AddMeta(const char* metaFile, uint lastModTime);
	bool DeleteMeta(const char* metaFile);

	void CheckFilesInAssets() const;

private:

	std::map<std::string, uint> metas;
	std::map<std::string, uint> assetsFiles;

	AssetsFile* rootAssetsFile = nullptr;
	LibraryFile* rootLibraryFile = nullptr;
};

#endif