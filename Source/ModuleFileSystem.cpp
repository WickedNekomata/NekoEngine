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

		AddPath("./Library/", "Library");

		if (PHYSFS_setWriteDir("Library") == 0)
			CONSOLE_LOG("Could not set Write Dir. ERROR: %s", PHYSFS_getLastError());
	}
}

ModuleFileSystem::~ModuleFileSystem() {}

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
	static char paths[BUF_SIZE];
	paths[0] = '\0'; // null-terminated byte string

	static char tmp_string[BUF_SIZE];

	char** path;
	for (path = PHYSFS_getSearchPath(); *path != nullptr; ++path)
	{
		sprintf_s(tmp_string, BUF_SIZE, "%s", *path);
		strcat_s(paths, BUF_SIZE, tmp_string);
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
		CONSOLE_LOG("FILE SYSTEM: Successfully created the directory '%s'");
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Couldn't create the directory '%s'. ERROR: %s", dirName, PHYSFS_getLastError());

	return ret;
}

uint ModuleFileSystem::SaveUnique(const char* fileName, const void* buffer, uint size, FileType fileType) const
{
	char file[BUF_SIZE];

	switch (fileType)
	{
	case FileType::MeshFile:
		sprintf_s(file, BUF_SIZE, "Library/Meshes/%s.fbx", fileName);
		break;
	case FileType::TextureFile:
		sprintf_s(file, BUF_SIZE, "Library/Materials/%s.dds", fileName);
		break;
	}

	if (Save(file, buffer, size))
		return size;
	else
		return size;
}

uint ModuleFileSystem::Save(const char* file, const void* buffer, uint size, bool append) const
{
	uint objCount = 0;

	bool exists = PHYSFS_exists(file);

	PHYSFS_file* filehandle = nullptr;
	if (append)
		filehandle = PHYSFS_openAppend(file);
	else
		filehandle = PHYSFS_openWrite(file);

	if (filehandle != nullptr)
	{
		objCount = PHYSFS_write(filehandle, buffer, 1, size);
	
		if (objCount == size)
		{
			if (exists)
			{
				if (append)
				{
					CONSOLE_LOG("FILE SYSTEM: Append %u bytes to file '%s'", objCount, file);
				}
				else
					CONSOLE_LOG("FILE SYSTEM: File '%s' overwritten with %u bytes", file, objCount);
			}			
			else
				CONSOLE_LOG("FILE SYSTEM: New file '%s' created with %u bytes", file, objCount);
		}
		else
			CONSOLE_LOG("FILE SYSTEM: Could not write to file '%s'. ERROR: %s", file, PHYSFS_getLastError());

		if (PHYSFS_close(filehandle) == 0)
			CONSOLE_LOG("FILE SYSTEM: Could not close file %s. ERROR: %s", PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Could not open file '%s' to write. ERROR: %s", file, PHYSFS_getLastError());

	return objCount;
}

uint ModuleFileSystem::Load(const char* file, char** buffer) const
{
	uint objCount = 0;

	bool exists = PHYSFS_exists(file);

	if (exists)
	{
		PHYSFS_file* filehandle = PHYSFS_openRead(file);

		if (filehandle != nullptr)
		{
			PHYSFS_sint64 size = PHYSFS_fileLength(filehandle);

			if (size > 0)
			{
				*buffer = new char[size];
				objCount = PHYSFS_read(filehandle, *buffer, 1, size);
			
				if (objCount == size)
				{
					CONSOLE_LOG("FILE SYSTEM: Read %u bytes from file '%s'", objCount, file);
				}
				else
				{
					RELEASE(buffer);
					CONSOLE_LOG("FILE SYSTEM: Could not read from file '%s'. ERROR: %s", file, PHYSFS_getLastError());
				}

				if (PHYSFS_close(filehandle) == 0)
					CONSOLE_LOG("FILE SYSTEM: Could not close file %s. ERROR: %s", PHYSFS_getLastError());
			}
		}
		else
			CONSOLE_LOG("FILE SYSTEM: Could not open file '%s' to read. ERROR: %s", file, PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG("FILE SYSTEM: Could not load file '%s' to read because it doesn't exist");

	return objCount;
}