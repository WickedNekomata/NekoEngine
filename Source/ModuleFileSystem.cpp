#include "Globals.h"
#include "Application.h"
#include "ModuleFileSystem.h"

#include "physfs/include/physfs.h"
#include "SDL/include/SDL.h"

#pragma comment(lib, "physfs/libx86/physfs.lib")

ModuleFileSystem::ModuleFileSystem(bool start_enabled) : Module(start_enabled)
{
	name = "FileSystem";

	PHYSFS_init(nullptr);

	AddPath(".");
	AddPath("./Assets/", "Assets");
	// TODO: If the user creates a new folder inside Assets, add the folder as a path!
	AddPath("./Assets/Meshes/", "Meshes");
	AddPath("./Assets/Textures/", "Textures");
	AddPath("./Assets/UI/", "UI");

	if (PHYSFS_setWriteDir(".") == 0)
		CONSOLE_LOG("Could not set Write Dir. ERROR: %s", PHYSFS_getLastError());

	if (CreateDir("Library"))
	{
		CreateDir("Library/Meshes");
		CreateDir("Library/Materials");
		CreateDir("Library/Animation");
		CreateDir("Library/Scenes");

		AddPath("./Library/", "Library");
	}
	
	CreateDir("Settings");
}

ModuleFileSystem::~ModuleFileSystem() {}

bool ModuleFileSystem::CleanUp()
{
	CONSOLE_LOG("Freeing File System subsystem");
	PHYSFS_deinit();

	return true;
}

bool ModuleFileSystem::AddPath(const char* newDir, const char* mountPoint)
{
	bool ret = false;

	if (PHYSFS_mount(newDir, mountPoint, 1) != 0)
		ret = true;
	else
		CONSOLE_LOG("File System error while adding a path or zip (%s): %s", newDir, PHYSFS_getLastError());
		
	return ret;
}

const char* ModuleFileSystem::GetBasePath() const 
{
	return PHYSFS_getBaseDir();
}

const char* ModuleFileSystem::GetReadPaths() const
{
	static char paths[MAX_BUF_SIZE];
	paths[0] = '\0'; // null-terminated byte string

	static char tmp_string[MAX_BUF_SIZE];

	char** path;
	for (path = PHYSFS_getSearchPath(); *path != nullptr; ++path)
	{
		sprintf_s(tmp_string, MAX_BUF_SIZE, "%s", *path);
		strcat_s(paths, MAX_BUF_SIZE, tmp_string);
	}

	return paths;
}

const char* ModuleFileSystem::GetWritePath() const 
{
	return PHYSFS_getWriteDir();
}

const char** ModuleFileSystem::GetFilesFromDir(const char* dir) const
{
	return (const char**)PHYSFS_enumerateFiles(dir);
}

bool ModuleFileSystem::CreateDir(const char* dirName) const
{
	bool ret = true;

	ret = PHYSFS_mkdir(dirName) != 0;

	if (ret)
	{
		CONSOLE_LOG("FILE SYSTEM: Successfully created the directory '%s'", dirName);
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Couldn't create the directory '%s'. ERROR: %s", dirName, PHYSFS_getLastError());

	return ret;
}

uint ModuleFileSystem::SaveInLibrary(char* buffer, uint size, FileType fileType, std::string& outputFileName, uint ID) const
{
	uint ret = 0;

	char filePath[DEFAULT_BUF_SIZE];
	char fileName[DEFAULT_BUF_SIZE];

	switch (fileType)
	{
	case FileType::MeshFile:
		if (ID > 0)
			sprintf_s(fileName, DEFAULT_BUF_SIZE, "%s_Mesh%u", outputFileName.data(), ID);
		else
			sprintf_s(fileName, DEFAULT_BUF_SIZE, "%s_Mesh", outputFileName.data());
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "Library/Meshes/%s.%s", fileName, FILE_EXTENSION);
		break;
	case FileType::TextureFile:
		if (ID > 0)
			sprintf_s(fileName, DEFAULT_BUF_SIZE, "%s_Texture%u", outputFileName.data(), ID);
		else
			sprintf_s(fileName, DEFAULT_BUF_SIZE, "%s_Texture", outputFileName.data());
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "Library/Materials/%s.%s", fileName, FILE_EXTENSION);
		break;
	case FileType::SceneFile:
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "Library/Scenes/%s.json", outputFileName.data(), FILE_EXTENSION);
		break;
	}

	outputFileName = fileName;

	ret = Save(filePath, buffer, size);

	return ret;
}

uint ModuleFileSystem::Save(const char* filePath, char* buffer, uint size, bool append) const
{
	uint objCount = 0;

	std::string fileName;
	GetFileName(filePath, fileName);

	bool exists = Exists(filePath);

	PHYSFS_file* filehandle = nullptr;
	if (append)
		filehandle = PHYSFS_openAppend(filePath);
	else
		filehandle = PHYSFS_openWrite(filePath);

	if (filehandle != nullptr)
	{
		objCount = PHYSFS_writeBytes(filehandle, (const void*)buffer, size);
	
		if (objCount == size)
		{
			if (exists)
			{
				if (append)
				{
					CONSOLE_LOG("FILE SYSTEM: Append %u bytes to file '%s'", objCount, fileName.data());
				}
				else
					CONSOLE_LOG("FILE SYSTEM: File '%s' overwritten with %u bytes", fileName.data(), objCount);
			}			
			else
				CONSOLE_LOG("FILE SYSTEM: New file '%s' created with %u bytes", fileName.data(), objCount);
		}
		else
			CONSOLE_LOG("FILE SYSTEM: Could not write to file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());

		if (PHYSFS_close(filehandle) == 0)
			CONSOLE_LOG("FILE SYSTEM: Could not close file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Could not open file '%s' to write. ERROR: %s", fileName.data(), PHYSFS_getLastError());

	return objCount;
}

uint ModuleFileSystem::LoadFromLibrary(const char* fileName, char** buffer, FileType fileType) const
{
	uint ret = 0;

	char filePath[DEFAULT_BUF_SIZE];

	switch (fileType)
	{
	case FileType::MeshFile:
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "Library/Meshes/%s.%s", fileName, FILE_EXTENSION);
		break;
	case FileType::TextureFile:
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "Library/Materials/%s.%s", fileName, FILE_EXTENSION);
		break;
	}

	ret = Load(filePath, buffer);
	
	return ret;
}

uint ModuleFileSystem::Load(const char* filePath, char** buffer) const
{
	uint objCount = 0;

	std::string fileName;
	GetFileName(filePath, fileName);

	bool exists = Exists(filePath);

	if (exists)
	{
		PHYSFS_file* filehandle = PHYSFS_openRead(filePath);

		if (filehandle != nullptr)
		{
			PHYSFS_sint64 size = PHYSFS_fileLength(filehandle);

			if (size > 0)
			{
				*buffer = new char[size];
				objCount = PHYSFS_readBytes(filehandle, *buffer, size);
			
				if (objCount == size)
				{
					CONSOLE_LOG("FILE SYSTEM: Read %u bytes from file '%s'", objCount, fileName.data());
				}
				else
				{
					RELEASE(buffer);
					CONSOLE_LOG("FILE SYSTEM: Could not read from file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
				}

				if (PHYSFS_close(filehandle) == 0)
					CONSOLE_LOG("FILE SYSTEM: Could not close file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
			}
		}
		else
			CONSOLE_LOG("FILE SYSTEM: Could not open file '%s' to read. ERROR: %s", fileName.data(), PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Could not load file '%s' to read because it doesn't exist", fileName.data());

	return objCount;
}

bool ModuleFileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file);
}

bool ModuleFileSystem::Exists(const char* filePath) const
{
	return PHYSFS_exists(filePath);
}

bool ModuleFileSystem::ExistsInAssets(const char* fileName, FileType fileType, std::string& outputFilePath) const
{
	uint ret = 0;

	char filePath[DEFAULT_BUF_SIZE];

	switch (fileType)
	{
	case FileType::MeshFile:
		outputFilePath = "Assets/Meshes/";
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "%s%s", outputFilePath.data(), fileName);
		break;
	case FileType::TextureFile:
		outputFilePath = "Assets/Textures/";
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "%s%s", outputFilePath.data(), fileName);
		break;
	}

	ret = Exists(filePath);

	return ret;
}

bool ModuleFileSystem::RecursiveFindNewFileInAssets(const char* dir, std::string& newFileInAssets) const
{
	bool ret = false;

	newFileInAssets.append(dir);
	newFileInAssets.append("/");

	const char** files = App->filesystem->GetFilesFromDir(dir);
	const char** it;

	for (it = files; *it != nullptr; ++it)
	{
		if (App->filesystem->IsDirectory(*it))
			ret = RecursiveFindNewFileInAssets(*it, newFileInAssets);
		else
		{
			// Search for the meta associated to the file
			char metaFile[DEFAULT_BUF_SIZE];
			strcpy_s(metaFile, strlen(*it) + 1, *it);

			static const char extension[] = ".meta";
			strcat_s(metaFile, strlen(metaFile) + strlen(extension) + 1, extension);

			// If the meta doesn't exist, then the file is new
			if (!Exists(metaFile))
			{
				CONSOLE_LOG("FILE SYSTEM: There is a new file '%s' in %s", *it, newFileInAssets.data());
				newFileInAssets.append(*it);
				ret = true;
			}
		}

		if (ret)
			break;
	}

	return ret;
}

void ModuleFileSystem::GetFileName(const char* file, std::string& fileName) const
{
	fileName = file;
	fileName = fileName.substr(fileName.find_last_of("\\") + 1, fileName.size());
	fileName = fileName.substr(fileName.find_last_of("//") + 1, fileName.size());
	fileName = fileName.substr(0, fileName.find_last_of("."));
}

void ModuleFileSystem::GetExtension(const char* file, std::string& extension) const
{
	extension = file;
	extension = extension.substr(extension.find_last_of("."));
}

void ModuleFileSystem::GetPath(const char* file, std::string& path) const
{
	path = file;
	path = path.substr(0, path.find_last_of("\\") + 1);
	path = path.substr(0, path.find_last_of("//") + 1);
}