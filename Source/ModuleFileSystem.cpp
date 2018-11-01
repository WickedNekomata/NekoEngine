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

ModuleFileSystem::~ModuleFileSystem() 
{}

bool ModuleFileSystem::Init(JSON_Object* jObject)
{
	CONSOLE_LOG("Loading File System");

	return true;
}

bool ModuleFileSystem::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG("Freeing File System subsystem");
	PHYSFS_deinit();

	return ret;
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
	}

	outputFileName = fileName;

	ret = Save(filePath, buffer, size);

	return ret;
}

uint ModuleFileSystem::Save(const char* filePath, char* buffer, uint size, bool append) const
{
	uint objCount = 0;

	const char* fileName = GetFileNameFromPath(filePath);

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
					CONSOLE_LOG("FILE SYSTEM: Append %u bytes to file '%s'", objCount, fileName);
				}
				else
					CONSOLE_LOG("FILE SYSTEM: File '%s' overwritten with %u bytes", fileName, objCount);
			}			
			else
				CONSOLE_LOG("FILE SYSTEM: New file '%s' created with %u bytes", fileName, objCount);
		}
		else
			CONSOLE_LOG("FILE SYSTEM: Could not write to file '%s'. ERROR: %s", fileName, PHYSFS_getLastError());

		if (PHYSFS_close(filehandle) == 0)
			CONSOLE_LOG("FILE SYSTEM: Could not close file '%s'. ERROR: %s", fileName, PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Could not open file '%s' to write. ERROR: %s", fileName, PHYSFS_getLastError());

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

	const char* fileName = GetFileNameFromPath(filePath);

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
				objCount = PHYSFS_read(filehandle, *buffer, 1, size);
			
				if (objCount == size)
				{
					CONSOLE_LOG("FILE SYSTEM: Read %u bytes from file '%s'", objCount, fileName);
				}
				else
				{
					RELEASE(buffer);
					CONSOLE_LOG("FILE SYSTEM: Could not read from file '%s'. ERROR: %s", fileName, PHYSFS_getLastError());
				}

				if (PHYSFS_close(filehandle) == 0)
					CONSOLE_LOG("FILE SYSTEM: Could not close file '%s'. ERROR: %s", fileName, PHYSFS_getLastError());
			}
		}
		else
			CONSOLE_LOG("FILE SYSTEM: Could not open file '%s' to read. ERROR: %s", fileName, PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Could not load file '%s' to read because it doesn't exist", fileName);

	return objCount;
}

bool ModuleFileSystem::Exists(const char* filePath) const
{
	return PHYSFS_exists(filePath);
}

bool ModuleFileSystem::ExistsInAssets(const char* fileNameWithExtension, FileType fileType, std::string& outputFilePath) const
{
	uint ret = 0;

	char filePath[DEFAULT_BUF_SIZE];

	switch (fileType)
	{
	case FileType::MeshFile:
		outputFilePath = "Assets/Meshes/";
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "%s%s", outputFilePath.data(), fileNameWithExtension);
		break;
	case FileType::TextureFile:
		outputFilePath = "Assets/Textures/";
		sprintf_s(filePath, DEFAULT_BUF_SIZE, "%s%s", outputFilePath.data(), fileNameWithExtension);
		break;
	}

	ret = Exists(filePath);

	return ret;
}

// TODO CHECK NEW ALWAYS DELETED
const char* ModuleFileSystem::GetFileNameFromPath(const char* path) const
{
	std::string newPath = path;
	std::string name = newPath;
	name = name.substr(name.find_last_of("\\") + 1, name.size());
	name = name.substr(name.find_last_of("//") + 1, name.size());
	name = name.substr(0, name.find_last_of("."));

	const char* result = new char[name.size() + 1];
	strcpy_s((char*)result, name.size() + 1, name.data());

	return result;
}