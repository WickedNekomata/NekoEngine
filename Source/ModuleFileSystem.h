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
#define DIR_LIBRARY_BONES "Library/Bones"
#define DIR_LIBRARY_ANIMATIONS "Library/Animations"
#define DIR_LIBRARY_SHADERS "Library/Shaders"
#define DIR_LIBRARY_SCRIPTS "Library/Scripts"
#define DIR_ASSETS "Assets"
#define DIR_ASSETS_SCENES "Assets/Scenes"
#define DIR_ASSETS_SHADERS "Assets/Shaders"
#define DIR_ASSETS_SHADERS_OBJECTS "Assets/Shaders/Objects"
#define DIR_ASSETS_SHADERS_PROGRAMS "Assets/Shaders/Programs"
#define DIR_ASSETS_PREFAB "Assets/Prefabs"
#define DIR_ASSETS_SCRIPTS "Assets/Scripts"
#define DIR_ASSETS_ANIMATIONS "Assets/Animations"
#define DIR_ASSETS_MESHES "Assets/Meshes"
#define DIR_ASSETS_TEXTURES "Assets/Textures"
#define DIR_ASSETS_MATERIALS "Assets/Materials"

#define IS_SCENE(extension) strcmp(extension, EXTENSION_SCENE) == 0
#define IS_META(extension) strcmp(extension, EXTENSION_META) == 0

class Resource;

enum FileTypes
{
	NoFileType,

	MeshFile,
	TextureFile,

	BoneFile,
	AnimationFile,

	PrefabFile,

	SceneFile,

	VertexShaderObjectFile,
	FragmentShaderObjectFile,
	GeometryShaderObjectFile,
	ShaderProgramFile,

	MaterialFile,

	MetaFile
};

struct File
{
	//FileName + extension, not full path
	std::string name;

	int64_t lastModTime = 0.0f;

	uint bytesToSerialize() const
	{
		return sizeof(uint) + name.size() + sizeof(int64_t);
	}

	void Serialize(char*& cursor) const
	{
		uint bytes = sizeof(uint);
		uint nameLenght = name.size();
		memcpy(cursor, &nameLenght, bytes);
		cursor += bytes;

		bytes = nameLenght;
		memcpy(cursor, name.c_str(), bytes);
		cursor += bytes;

		bytes = sizeof(int64_t);
		memcpy(cursor, &lastModTime, bytes);
		cursor += bytes;
	}

	void DeSerialize(char*& cursor)
	{
		uint bytes = sizeof(uint);
		uint nameLenght;
		memcpy(&nameLenght, cursor, bytes);
		cursor += bytes;


		name.resize(nameLenght);
		bytes = nameLenght;
		memcpy((char*)name.c_str(), cursor, bytes);
		cursor += bytes;

		bytes = sizeof(int64_t);
		memcpy(&lastModTime, cursor, bytes);
		cursor += bytes;
	}

	bool operator == (File other)
	{
		return (name == other.name) && (lastModTime == other.lastModTime);
	}
	bool operator != (File other)
	{
		return !(*(this) == other);
	}
};

struct Directory
{
	std::string fullPath;
	std::string name;
	std::vector<File> files;
	std::vector<Directory> directories;

	~Directory()
	{
		name.clear();
		files.clear();
		directories.clear();
	}

	void getFullPaths(std::vector<std::string>& fullpaths) const
	{
		for (int i = 0; i < files.size(); ++i)
			fullpaths.push_back(fullPath + "/" + files[i].name);

		for (int i = 0; i < directories.size(); ++i)
			directories[i].getFullPaths(fullpaths);
	}

	void getFiles(std::vector<File>& _files) const
	{
		for (int i = 0; i < files.size(); ++i)
			_files.push_back(files[i]);

		for (int i = 0; i < directories.size(); ++i)
			directories[i].getFiles(_files);
	}

	/*void Serialize(char*& cursor) const
	//{
	//	uint fullPathLength = fullPath.size();
	//	uint bytes = sizeof(uint);
	//	memcpy(cursor, &fullPathLength, bytes);
	//	cursor += bytes;
	//	bytes = fullPathLength;
	//	memcpy(cursor, fullPath.c_str(), bytes);
	//	cursor += bytes;
	//	bytes = sizeof(uint);
	//	uint nameLenght = name.size();
	//	memcpy(cursor, &nameLenght, bytes);
	//	cursor += bytes;
	//	bytes = nameLenght;
	//	memcpy(cursor, name.c_str(), bytes);
	//	cursor += bytes;
	//	uint numFiles = files.size(), numDirs = directories.size();
	//	bytes = sizeof(uint);
	//	memcpy(cursor, &numFiles, bytes);
	//	cursor += bytes;
	//	memcpy(cursor, &numDirs, bytes);
	//	cursor += bytes;
	//	for (int i = 0; i < files.size(); ++i)
	//	{
	//		files[i].Serialize(cursor);
	//	}
	//	for (int i = 0; i < directories.size(); ++i)
	//	{
	//		directories[i].Serialize(cursor);
	//	}
	//}
	//void DeSerialize(char*& cursor)
	//{
	//	uint bytes = sizeof(uint);
	//	uint fullPathLenght;
	//	memcpy(&fullPathLenght, cursor, bytes);
	//	cursor += bytes;
	//	fullPath.resize(fullPathLenght);
	//	memcpy((char*)fullPath.c_str(), cursor, fullPathLenght);
	//	cursor += fullPathLenght;
	//	bytes = sizeof(uint);
	//	uint nameLenght;
	//	memcpy(&nameLenght, cursor, bytes);
	//	cursor += bytes;
	//	name.resize(nameLenght);
	//	bytes = nameLenght;
	//	memcpy((char*)name.c_str(), cursor, bytes);
	//	cursor += bytes;
	//	uint numFiles;
	//	bytes = sizeof(uint);
	//	memcpy(&numFiles, cursor, bytes);
	//	cursor += bytes;
	//	uint numDirs;
	//	memcpy(&numDirs, cursor, bytes);
	//	cursor += bytes;
	//	files.resize(numFiles);
	//	directories.resize(numDirs);
	//	for (int i = 0; i < files.size(); ++i)
	//	{
	//		files[i].DeSerialize(cursor);
	//	}
	//	for (int i = 0; i < directories.size(); ++i)
	//	{
	//		directories[i].DeSerialize(cursor);
	//	}
	//}
	//uint bytesToSerialize() const
	//{
	//	uint ret =
	//		sizeof(uint) + fullPath.size() +	//Fullpath lenght + fullpath
	//		sizeof(uint) + name.size() +		//Name lenght + Name
	//		sizeof(uint) * 2;					//Num files / directories
	//	for (int i = 0; i < files.size(); ++i)
	//	{
	//		ret += files[i].bytesToSerialize();
	//	}
	//	for (int i = 0; i < directories.size(); ++i)
	//	{
	//		ret += directories[i].bytesToSerialize();
	//	}
	//	return ret;
	}*/

	bool operator == (Directory other)
	{
		bool ret = true;

		if (name != other.name || files.size() != other.files.size() || directories.size() != other.directories.size())
			ret = false;

		if (ret)
		{
			for (int i = 0; i < files.size() && ret; ++i)
			{
				if (files[i] != other.files[i])
					ret = false;
			}

			if (ret)
			{
				for (int i = 0; i < directories.size() && ret; ++i)
				{
					if (directories[i] != other.directories[i])
						ret = false;
				}
			}
		}
		return ret;
	}

	bool operator != (Directory other)
	{
		return !(*this == other);
	}
};

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(bool start_enabled = true);
	~ModuleFileSystem();

	update_status PreUpdate() override;
	bool Start();
	bool CleanUp();

	void OnSystemEvent(System_Event event);

	bool CreateDir(const char* dirName) const;
	bool AddPath(const char* newDir, const char* mountPoint = nullptr) const;
	bool DeleteFileOrDir(const char* path) const;

	const char* GetBasePath() const;
	const char* GetReadPaths() const;
	const char* GetWritePath() const;
	const char** GetFilesFromDir(const char* dir) const;

	int64_t GetLastModificationTime(const char* file) const;

	bool IsDirectory(const char* file) const;
	bool Exists(std::string file) const;
	bool RecursiveExists(const char* fileName, const char* dir, std::string& path) const;

	void GetFileName(const char* file, std::string& fileName, bool extension = false) const;
	void GetExtension(const char* file, std::string& extension) const;
	void GetMetaExtension(const char* file, std::string& extension) const;
	void GetFileFromMeta(const char* metaFile, std::string& file) const;
	void GetPath(const char* file, std::string& path, bool bar = true) const;

	uint Copy(const char* file, const char* dir, std::string& outputFile) const;

	uint SaveInGame(char* buffer, uint size, FileTypes fileType, std::string& outputFile, bool overwrite = false) const;
	uint Save(std::string file, char* buffer, uint size, bool append = false) const;

	uint Load(std::string file, char** buffer) const;

	std::string getAppPath();
	void UpdateAssetsDir();
	bool MoveFileInto(const std::string& file, const std::string& newLocation);
	bool CopyDirectoryAndContentsInto(const std::string& origin, const std::string& destination, bool keepRoot = true);
	Directory RecursiveGetFilesFromDir(char* dir) const;
	bool deleteFile(const std::string& filePath) const;
	bool deleteFiles(const std::string& rootDirectory, const std::string& extension) const;
	void SendEvents(const Directory& newAssetsDir);
	void ImportFilesEvents(const Directory& newAssetsDir, std::vector<std::string>& lateEvents = std::vector<std::string>());
	void ForceReImport(const Directory& assetsDir);

	void BeginTempException(std::string directory);
	void EndTempException();

public:
	Directory rootAssets;

private:
	float updateAssetsRate = 1.0f;	
	std::string tempException;

};

#endif