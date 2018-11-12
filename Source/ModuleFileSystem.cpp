#include "ModuleFileSystem.h"

#include "Application.h"
#include "Globals.h"

#include "physfs/include/physfs.h"

#pragma comment(lib, "physfs/libx86/physfs.lib")

ModuleFileSystem::ModuleFileSystem(bool start_enabled) : Module(start_enabled)
{
	name = "FileSystem";

	PHYSFS_init(nullptr);

	AddPath(".");
	AddPath("./Assets/", "Assets");

	if (PHYSFS_setWriteDir(".") == 0)
		CONSOLE_LOG("Could not set Write Dir. ERROR: %s", PHYSFS_getLastError());

	CreateDir(DIR_ASSETS_SCENES);

	// TODO: If the user creates a new folder inside Assets, add the folder as a path!
	AddPath("./Assets/Meshes/", "Meshes");
	AddPath("./Assets/Textures/", "Textures");
	AddPath("./Assets/UI/", "UI");
	AddPath("./Assets/Scenes/", "Scenes");

	if (CreateDir(DIR_LIBRARY))
	{
		AddPath("./Library/", "Library");

		CreateDir(DIR_LIBRARY_MESHES);
		CreateDir(DIR_LIBRARY_MATERIALS);
		AddPath("./Library/Meshes/", "LibMeshes");
		AddPath("./Library/Materials/", "LibMaterials");
	}
	
	// TODO
	//CreateDir("Settings");
}

ModuleFileSystem::~ModuleFileSystem() {}

bool ModuleFileSystem::CleanUp()
{
	CONSOLE_LOG("Freeing File System subsystem");
	PHYSFS_deinit();

	return true;
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

uint ModuleFileSystem::Copy(const char* file, const char* dir, std::string& outputFile) const
{
	uint size = 0;

	if (file == nullptr || dir == nullptr)
	{
		assert(file != nullptr && dir != nullptr);
		return size;
	}

	std::FILE* filehandle;
	fopen_s(&filehandle, file, "rb");
	
	if (filehandle != nullptr)
	{
		fseek(filehandle, 0, SEEK_END);
		size = ftell(filehandle);
		rewind(filehandle);

		char* buffer = new char[size];
		size = fread(buffer, 1, size, filehandle);
		if (size > 0)
		{
			GetFileName(file, outputFile, true);
			outputFile.insert(0, "/");
			outputFile.insert(0, dir);

			size = Save(outputFile.data(), buffer, size);
			if (size > 0)
			{
				CONSOLE_LOG("FILE SYSTEM: Successfully copied file '%s' in dir '%s'", file, dir);
			}
			else
				CONSOLE_LOG("FILE SYSTEM: Could not copy file '%s' in dir '%s'", file, dir);
		}
		else
			CONSOLE_LOG("FILE SYSTEM: Could not read from file '%s'", file);

		RELEASE_ARRAY(buffer);
		fclose(filehandle);
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Could not open file '%s' to read", file);

	return size;
}

uint ModuleFileSystem::SaveInLibrary(char* buffer, uint size, FileType fileType, std::string& outputFile) const
{
	uint ret = 0;

	switch (fileType)
	{
	case FileType::MeshFile:
		outputFile.insert(0, DIR_LIBRARY_MESHES);
		outputFile.insert(strlen(DIR_LIBRARY_MESHES), "/");
		outputFile.append(EXTENSION_MESH);
		break;
	case FileType::TextureFile:
		outputFile.insert(0, DIR_LIBRARY_MATERIALS);
		outputFile.insert(strlen(DIR_LIBRARY_MATERIALS), "/");
		outputFile.append(EXTENSION_TEXTURE);
		break;
	case FileType::SceneFile:
		outputFile.insert(0, DIR_ASSETS_SCENES);
		outputFile.insert(strlen(DIR_ASSETS_SCENES), "/");
		outputFile.append(EXTENSION_SCENE);
		break;
	}

	ret = Save(outputFile.data(), buffer, size);

	return ret;
}

uint ModuleFileSystem::Save(const char* file, char* buffer, uint size, bool append) const
{
	uint objCount = 0;

	std::string fileName;
	GetFileName(file, fileName, true);

	bool exists = Exists(file);

	PHYSFS_file* filehandle = nullptr;
	if (append)
		filehandle = PHYSFS_openAppend(file);
	else
		filehandle = PHYSFS_openWrite(file);

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

uint ModuleFileSystem::Load(const char* file, char** buffer) const
{
	uint objCount = 0;

	std::string fileName;
	GetFileName(file, fileName, true);

	bool exists = Exists(file);

	if (exists)
	{
		PHYSFS_file* filehandle = PHYSFS_openRead(file);

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

bool ModuleFileSystem::Exists(const char* file) const
{
	return PHYSFS_exists(file);
}

void ModuleFileSystem::GetFileName(const char* file, std::string& fileName, bool extension) const
{
	fileName = file;

	uint found = fileName.find_last_of("\\");
	if (found != std::string::npos)
		fileName = fileName.substr(found + 1, fileName.size());

	found = fileName.find_last_of("//");
	if (found != std::string::npos)
		fileName = fileName.substr(found + 1, fileName.size());

	if (!extension)
	{
		found = fileName.find_last_of(".");
		if (found != std::string::npos)
			fileName = fileName.substr(0, found);
	}
}

void ModuleFileSystem::GetExtension(const char* file, std::string& extension) const
{
	extension = file;

	uint found = extension.find_last_of(".");
	if (found != std::string::npos)
		extension = extension.substr(found);
}

void ModuleFileSystem::GetPath(const char* file, std::string& path) const
{
	path = file;

	uint found = path.find_last_of("\\");
	if (found != std::string::npos)
		path = path.substr(0, found + 1);

	found = path.find_last_of("//");
	if (found != std::string::npos)
		path = path.substr(0, found + 1);
}