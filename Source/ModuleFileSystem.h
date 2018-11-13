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

#define MAX_ASSETS_CHECK_TIME 2.0f // seconds
#define MAX_ASSETS_SEARCH_TIME 8.0 // ms

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
	bool Start();
	update_status Update() override;
	bool CleanUp();

	bool CreateDir(const char* dirName) const;
	bool AddPath(const char* newDir, const char* mountPoint = nullptr);

	const char* GetBasePath() const;
	const char* GetReadPaths() const;
	const char* GetWritePath() const;
	const char** GetFilesFromDir(const char* dir) const;

	void RecursiveGetFilesFromDir(const char* dir, std::string& path, std::map<std::string, uint>& files);

	bool IsDirectory(const char* file) const;
	bool Exists(const char* file) const;

	int GetLastModificationTime(const char* file) const;

	void GetFileName(const char* file, std::string& fileName, bool extension = false) const;
	void GetExtension(const char* file, std::string& extension) const;
	void GetPath(const char* file, std::string& path) const;

	uint Copy(const char* file, const char* dir, std::string& outputFile) const;

	uint SaveInLibrary(char* buffer, uint size, FileType fileType, std::string& outputFile) const;
	uint Save(const char* file, char* buffer, uint size, bool append = false) const;

	uint Load(const char* file, char** buffer) const;

	void CheckAssetsAgainst(std::map<std::string, uint> newFilesInAssets);

private:

	std::map<std::string, uint> filesInAssets;

	float assetsCheckTime = 1.0f; // seconds
	PerfTimer assetsCheckTimer;

	PerfTimer assetsSearchTimer;
};

#endif