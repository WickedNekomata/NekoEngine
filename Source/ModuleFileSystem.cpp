#include "ModuleFileSystem.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "Importer.h"
#include "Globals.h"
#include "Importer.h"
#include "ModuleResourceManager.h"

#include "physfs/include/physfs.h"

#include <assert.h>

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

bool ModuleFileSystem::Start()
{
	// Store all metas found in Assets
	std::string path;
	RecursiveGetFilesFromDir(DIR_ASSETS, path, metas, FileType::MetaFile);

	// For each one, replace its modification time for the one written in the meta itself
	std::string extension;
	int lastModTime = 0;

	for (std::map<std::string, uint>::const_iterator it = metas.begin(); it != metas.end(); ++it)
	{
		GetExtension(it->first.data(), extension);

		if (IS_META(extension.data()))
		{
			lastModTime = it->second;
			Importer::GetLastModificationTimeFromMeta(it->first.data(), lastModTime);
			assert(lastModTime != -1);
			metas[it->first] = lastModTime;
		}
	}

	assetsCheckTimer.Start();

	return true;
}

update_status ModuleFileSystem::Update()
{
	if (assetsCheckTimer.ReadMs() >= assetsCheckTime * 1000.0f)
	{
		//assetsSearchTimer.Start();

		std::string path;
		std::map<std::string, uint> newFilesInAssets;
		RecursiveGetFilesFromDir(DIR_ASSETS, path, newFilesInAssets);

		CheckAssets(newFilesInAssets);

		assetsCheckTimer.Start();
	}

	return UPDATE_CONTINUE;
}

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

bool ModuleFileSystem::DeleteFileOrDir(const char* path)
{
	bool ret = false;

	if (PHYSFS_delete(path) != 0)
		ret = true;
	else
		CONSOLE_LOG("File System error while deleting a file or directory (%s): %s", path, PHYSFS_getLastError());

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

void ModuleFileSystem::RecursiveGetFilesFromDir(const char* dir, std::string& path, std::map<std::string, uint>& files, FileType fileType)
{
	if (dir == nullptr)
	{
		assert(dir != nullptr);
		return;
	}

	path.append(dir);
	path.append("/");

	const char** currentFiles = App->fs->GetFilesFromDir(dir);
	const char** it;

	for (it = currentFiles; *it != nullptr; ++it)
	{
		if (App->fs->IsDirectory(*it))
		{
			RecursiveGetFilesFromDir(*it, path, files, fileType);

			uint found = path.rfind(*it);
			if (found != std::string::npos)
				path = path.substr(0, found);
		}
		else
		{
			std::string extension;
			GetExtension(*it, extension);

			switch (fileType)
			{
			case FileType::MeshFile:	
				if (ModuleResourceManager::GetResourceTypeByExtension(extension.data()) != ResourceType::Mesh_Resource)
					continue;
				break;
			case FileType::TextureFile:
				if (ModuleResourceManager::GetResourceTypeByExtension(extension.data()) != ResourceType::Texture_Resource)
					continue;
				break;
			case FileType::ResourceFile:
			{
				ResourceType type = ModuleResourceManager::GetResourceTypeByExtension(extension.data());
				if (type != ResourceType::Mesh_Resource && type != ResourceType::Texture_Resource)
					continue;
			}
				break;
			case FileType::SceneFile:
				if (!IS_SCENE(extension.data()))
					continue;
				break;
			case FileType::MetaFile:
				if (!IS_META(extension.data()))
					continue;
				break;
			}

			std::string file = path;
			file.append(*it);

			int lastModTime = GetLastModificationTime(file.data());
			assert(lastModTime != -1);
			files[file] = lastModTime;
		}
	}
}

bool ModuleFileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file);
}

bool ModuleFileSystem::Exists(const char* file) const
{
	return PHYSFS_exists(file);
}

int ModuleFileSystem::GetLastModificationTime(const char* file) const
{
	return PHYSFS_getLastModTime(file);
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

void ModuleFileSystem::CheckAssets(std::map<std::string, uint> newFilesInAssets)
{
	// NOTE: Files in library are not expected to be removed by the user

	for (std::map<std::string, uint>::const_iterator it = metas.begin(); it != metas.end(); ++it)
	{
		// Each meta is expected to have an associated file in Assets that creates a resource

		// Path of the file in Assets associated to the meta
		std::string fileInAssets = it->first;
		uint found = fileInAssets.find_last_of(".");
		if (found != std::string::npos)
			fileInAssets = fileInAssets.substr(0, found);

		// CASE 1. Original file has been removed
		// + Original file has been renamed from outside the editor (we have no way to detect it)
		// + Original file has been moved to another folder from outside the editor (we have no way to detect it)
		if (newFilesInAssets.find(fileInAssets.data()) == newFilesInAssets.end())
		{
			System_Event newEvent;
			newEvent.fileEvent.file = fileInAssets.data();
			newEvent.fileEvent.metaFile = it->first.data();
			newEvent.type = System_Event_Type::FileRemoved;
			App->PushSystemEvent(newEvent);
			// TODO remove the meta from the vector
		}
		// CASE 2. Meta has been removed
		else if (newFilesInAssets.find(it->first.data()) == newFilesInAssets.end())
		{
			System_Event newEvent;
			newEvent.fileEvent.file = it->first.data();
			newEvent.type = System_Event_Type::MetaRemoved;
			App->PushSystemEvent(newEvent);
			// TODO remove the meta from the vector and then add the new meta when generating the file
		}
		// CASE 3. Original file has been overwritten
		else if (newFilesInAssets.find(fileInAssets.data())->second != it->second)
		{
			System_Event newEvent;
			newEvent.fileEvent.file = fileInAssets.data();
			newEvent.fileEvent.metaFile = it->first.data();
			newEvent.type = System_Event_Type::FileOverwritten;
			App->PushSystemEvent(newEvent);
		}
	}

	for (std::map<std::string, uint>::const_iterator it = newFilesInAssets.begin(); it != newFilesInAssets.end(); ++it)
	{
		std::string extension;
		GetExtension(it->first.data(), extension);

		if (ModuleResourceManager::GetResourceTypeByExtension(extension.data()) != ResourceType::No_Type_Resource)
		{
			// Each file in Assets that creates a resource is expected to have an associated meta

			// Path of the meta associated to the file in Assets
			std::string meta = it->first;
			meta.append(EXTENSION_META);

			// CASE 4. A new file has been added
			if (metas.find(meta.data()) == metas.end())
			{
				System_Event newEvent;
				newEvent.fileEvent.file = it->first.data();
				newEvent.type = System_Event_Type::NewFile;
				App->PushSystemEvent(newEvent);
				// TODO add the generated meta to the metas map when the file has been created
			}
		}
	}
}

void ModuleFileSystem::SetAssetsCheckTime(float assetsCheckTime)
{
	this->assetsCheckTime = assetsCheckTime;

	if (this->assetsCheckTime > MAX_ASSETS_CHECK_TIME)
		this->assetsCheckTime = MAX_ASSETS_CHECK_TIME;
}

float ModuleFileSystem::GetAssetsCheckTime() const
{
	return assetsCheckTime;
}