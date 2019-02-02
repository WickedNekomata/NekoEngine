#include "ModuleFileSystem.h"

#include "Application.h"
#include "Globals.h"

#include "Importer.h"
#include "SceneImporter.h"
#include "MaterialImporter.h"
#include "ShaderImporter.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ModuleScene.h"

#include "physfs\include\physfs.h"
#include "Brofiler\Brofiler.h"

#include <assert.h>

#pragma comment(lib, "physfs/libx86/physfs.lib")

ModuleFileSystem::ModuleFileSystem(bool start_enabled) : Module(start_enabled)
{
	name = "FileSystem";

	if (PHYSFS_init(nullptr) == 0)
	{
		const char* error = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
		DEPRECATED_LOG("Could not initialize PHYSFS. Error: %s", error);
	}

	AddPath(".");
#ifndef GAMEMODE
	AddPath("./Assets/", "Assets");
#endif
	AddPath("./Settings/", "Settings");

	//Internal Directory: Engine stuff
	AddPath("./internal.f", "Internal");

	if (PHYSFS_setWriteDir(".") == 0)
		DEPRECATED_LOG("Could not set Write Dir. ERROR: %s", PHYSFS_getLastError());

#ifndef GAMEMODE
	CreateDir(DIR_ASSETS_SCENES);
	CreateDir(DIR_ASSETS_SHADERS);
	CreateDir(DIR_ASSETS_SHADERS_OBJECTS);
	CreateDir(DIR_ASSETS_SHADERS_PROGRAMS);
	CreateDir(DIR_ASSETS_SCRIPTS);
#endif
	if (CreateDir(DIR_LIBRARY))
	{
		AddPath("./Library/", "Library");

		CreateDir(DIR_LIBRARY_MESHES);
		CreateDir(DIR_LIBRARY_MATERIALS);
		CreateDir(DIR_LIBRARY_SCRIPTS);
	}
}

ModuleFileSystem::~ModuleFileSystem() {}

bool ModuleFileSystem::CleanUp()
{
	DEPRECATED_LOG("Freeing File System subsystem");
	PHYSFS_deinit();

	return true;
}

void ModuleFileSystem::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::RefreshAssets:

		if (App->scene->selectedObject != CurrentSelection::SelectedType::gameObject)
			SELECT(NULL);

		// Read the current files in Assets
		RELEASE(rootAssetsFile);
		assetsFiles.clear();
		rootAssetsFile = new AssetsFile();
		rootAssetsFile->name = rootAssetsFile->path = DIR_ASSETS;
		rootAssetsFile->isDirectory = true;
		RecursiveGetFilesFromAssets(rootAssetsFile, assetsFiles);

		// Check the read files against the metas
		CheckFilesInAssets();

		break;

	case System_Event_Type::RefreshFiles:

		// Read the current files in Assets
		RELEASE(rootAssetsFile);
		assetsFiles.clear();
		rootAssetsFile = new AssetsFile();
		rootAssetsFile->name = rootAssetsFile->path = DIR_ASSETS;
		rootAssetsFile->isDirectory = true;
		RecursiveGetFilesFromAssets(rootAssetsFile, assetsFiles);

		// Read the current files in Library
		RELEASE(rootLibraryFile);
		rootLibraryFile = new LibraryFile();
		rootLibraryFile->name = rootLibraryFile->path = DIR_LIBRARY;
		rootLibraryFile->isDirectory = true;
		RecursiveGetFilesFromLibrary(rootLibraryFile);

		break;
	}
}

bool ModuleFileSystem::CreateDir(const char* dirName) const
{
	bool ret = true;

	ret = PHYSFS_mkdir(dirName) != 0;

	if (ret)
	{
		DEPRECATED_LOG("FILE SYSTEM: Successfully created the directory '%s'", dirName);
	}
	else
		DEPRECATED_LOG("FILE SYSTEM: Couldn't create the directory '%s'. ERROR: %s", dirName, PHYSFS_getLastError());

	return ret;
}

bool ModuleFileSystem::AddPath(const char* newDir, const char* mountPoint) const
{
	bool ret = false;

	if (PHYSFS_mount(newDir, mountPoint, 1) != 0)
		ret = true;
	else
		DEPRECATED_LOG("FILE SYSTEM: Error while adding a path or zip '%s': %s", newDir, PHYSFS_getLastError());
		
	return ret;
}

bool ModuleFileSystem::DeleteFileOrDir(const char* path) const
{
	bool ret = false;

	if (PHYSFS_delete(path) != 0)
		ret = true;
	else
		DEPRECATED_LOG("FILE SYSTEM: Error while deleting a file or directory '%s': %s", path, PHYSFS_getLastError());

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

void ModuleFileSystem::RecursiveGetFilesFromAssets(AssetsFile* assetsFile, std::map<std::string, int>& assetsFiles) const
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);
#endif
	assert(assetsFile != nullptr);

	std::string path = assetsFile->path;

	const char** files = GetFilesFromDir(path.data());
	const char** it;

	path.append("/");

	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		AssetsFile* file = new AssetsFile();
		file->path = path.data();
		GetFileName(file->path.data(), file->name);

		if (IsDirectory(path.data()))
		{
			file->isDirectory = true;
			RecursiveGetFilesFromAssets(file, assetsFiles);
		}
		else
		{
			std::string extension;
			GetExtension(*it, extension);

			file->lastModTime = GetLastModificationTime(path.data());

			// Search for the meta associated to the file
			char metaFile[DEFAULT_BUF_SIZE];
			strcpy_s(metaFile, strlen(path.data()) + 1, path.data()); // file
			strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

			ResourceType type = ModuleResourceManager::GetResourceTypeByExtension(extension.data());
			switch (type)
			{
			case ResourceType::MeshResource:
			{
				// Read the import settings
				MeshImportSettings* importSettings = new MeshImportSettings();
				App->sceneImporter->GetMeshImportSettingsFromMeta(metaFile, importSettings);
				file->importSettings = importSettings;

				// Read the UUIDs of the meshes
				std::list<uint> UUIDs;
				App->sceneImporter->GetMeshesUUIDsFromMeta(metaFile, UUIDs);

				for (std::list<uint>::const_iterator it = UUIDs.begin(); it != UUIDs.end(); ++it)
				{
					const Resource* resource = App->res->GetResource(*it);
					file->UUIDs[resource->GetName()] = *it;
				}
			}
			break;
			case ResourceType::TextureResource:
			{
				// Read the import settings
				TextureImportSettings* importSettings = new TextureImportSettings();
				App->materialImporter->GetTextureImportSettingsFromMeta(metaFile, importSettings);
				file->importSettings = importSettings;

				// Read the UUID of the texture
				uint UUID = 0;
				App->materialImporter->GetTextureUUIDFromMeta(metaFile, UUID);
				file->UUIDs[file->name.data()] = UUID;
			}
			break;
			case ResourceType::ShaderObjectResource:
			case ResourceType::ShaderProgramResource:
			{
				// Read the UUID of the shader
				uint UUID = 0;
				App->shaderImporter->GetShaderUUIDFromMeta(metaFile, UUID);
				file->resource = App->res->GetResource(UUID);
			}
			break;
			}

			assetsFiles[file->path] = file->lastModTime;
		}

		assetsFile->children.push_back(file);

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}
}

void ModuleFileSystem::RecursiveGetFilesFromLibrary(LibraryFile* libraryFile) const
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);
#endif

	assert(libraryFile != nullptr);

	std::string path = libraryFile->path;

	const char** files = GetFilesFromDir(path.data());
	const char** it;

	path.append("/");

	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		LibraryFile* file = new LibraryFile();
		file->path = path.data();
		GetFileName(file->path.data(), file->name);
		bool invalid = false;

		if (IsDirectory(path.data()))
		{
			if (strcmp(path.data(), DIR_LIBRARY_SHADERS) != 0)
			{
				file->isDirectory = true;
				RecursiveGetFilesFromLibrary(file);
			}
			else
				invalid = true;
		}
		else
		{
			uint UUID = strtoul(*it, NULL, 0);
			file->resource = App->res->GetResource(UUID);

			if (file->resource == nullptr)
				invalid = true;
		}

		if (!invalid)
			libraryFile->children.push_back(file);
		else
			RELEASE(file);

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}
}

AssetsFile* ModuleFileSystem::GetRootAssetsFile() const
{
	return rootAssetsFile;
}

LibraryFile* ModuleFileSystem::GetRootLibraryFile() const
{
	return rootLibraryFile;
}

bool ModuleFileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file);
}

bool ModuleFileSystem::Exists(std::string file) const
{
	return PHYSFS_exists(file.data());
}

bool ModuleFileSystem::RecursiveExists(const char* fileName, const char* dir, std::string& path) const
{
	if (dir == nullptr)
	{
		assert(dir != nullptr);
		return false;
	}

	path.append("/");

	std::string file = path;
	file.append(fileName);

	bool exists = Exists(file.data());

	if (exists)
	{
		path.append(fileName);
		return exists;
	}

	const char** files = GetFilesFromDir(path.data());
	const char** it;

	for (it = files; *it != nullptr; ++it)
	{
		path.append(*it);

		if (IsDirectory(path.data()))
		{
			exists = RecursiveExists(fileName, *it, path);

			if (exists)
				return exists;
		}

		uint found = path.rfind(*it);
		if (found != std::string::npos)
			path = path.substr(0, found);
	}

	return exists;
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

void ModuleFileSystem::GetMetaExtension(const char* file, std::string& extension) const
{
	std::string ex = file;
	extension = ex;

	uint foundMeta = extension.find_last_of(".");
	if (foundMeta != std::string::npos)
	{
		ex = ex.substr(foundMeta);
		extension = extension.substr(0, foundMeta);
	}
	uint foundExtension = extension.find_last_of(".");
	if (foundExtension != std::string::npos)
		extension = extension.substr(foundExtension);
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
				DEPRECATED_LOG("FILE SYSTEM: Successfully copied file '%s' in dir '%s'", file, dir);
			}
			else
				DEPRECATED_LOG("FILE SYSTEM: Could not copy file '%s' in dir '%s'", file, dir);
		}
		else
			DEPRECATED_LOG("FILE SYSTEM: Could not read from file '%s'", file);

		RELEASE_ARRAY(buffer);
		fclose(filehandle);
	}
	else
		DEPRECATED_LOG("FILE SYSTEM: Could not open file '%s' to read", file);

	return size;
}

uint ModuleFileSystem::SaveInGame(char* buffer, uint size, FileType fileType, std::string& outputFile, bool overwrite) const
{
	uint ret = 0;

	if (!overwrite)
	{
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
		case FileType::VertexShaderObjectFile:
			outputFile.insert(0, DIR_ASSETS_SHADERS_OBJECTS);
			outputFile.insert(strlen(DIR_ASSETS_SHADERS_OBJECTS), "/");
			outputFile.append(EXTENSION_VERTEX_SHADER_OBJECT);
			break;
		case FileType::FragmentShaderObjectFile:
			outputFile.insert(0, DIR_ASSETS_SHADERS_OBJECTS);
			outputFile.insert(strlen(DIR_ASSETS_SHADERS_OBJECTS), "/");
			outputFile.append(EXTENSION_FRAGMENT_SHADER_OBJECT);
			break;
		case FileType::ShaderProgramFile:
			outputFile.insert(0, DIR_ASSETS_SHADERS_PROGRAMS);
			outputFile.insert(strlen(DIR_ASSETS_SHADERS_PROGRAMS), "/");
			outputFile.append(EXTENSION_SHADER_PROGRAM);
			break;
		}
	}

	ret = Save(outputFile.data(), buffer, size);

	return ret;
}

uint ModuleFileSystem::Save(std::string file, char* buffer, uint size, bool append) const
{
	uint objCount = 0;

	std::string fileName;
	GetFileName(file.data(), fileName, true);

	bool exists = Exists(file.data());

	PHYSFS_file* filehandle = nullptr;
	if (append)
		filehandle = PHYSFS_openAppend(file.data());
	else
		filehandle = PHYSFS_openWrite(file.data());

	if (filehandle != nullptr)
	{
		objCount = PHYSFS_writeBytes(filehandle, (const void*)buffer, size);
	
		if (objCount == size)
		{
			if (exists)
			{
				if (append)
				{
					DEPRECATED_LOG("FILE SYSTEM: Append %u bytes to file '%s'", objCount, fileName.data());
				}
				else
					DEPRECATED_LOG("FILE SYSTEM: File '%s' overwritten with %u bytes", fileName.data(), objCount);
			}			
			else
				DEPRECATED_LOG("FILE SYSTEM: New file '%s' created with %u bytes", fileName.data(), objCount);
		}
		else
			DEPRECATED_LOG("FILE SYSTEM: Could not write to file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());

		if (PHYSFS_close(filehandle) == 0)
			DEPRECATED_LOG("FILE SYSTEM: Could not close file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
	}
	else
		DEPRECATED_LOG("FILE SYSTEM: Could not open file '%s' to write. ERROR: %s", fileName.data(), PHYSFS_getLastError());

	return objCount;
}

uint ModuleFileSystem::Load(std::string file, char** buffer) const
{
	uint objCount = 0;

	std::string fileName;
	GetFileName(file.data(), fileName, true);

	bool exists = Exists(file.data());

	if (exists)
	{
		PHYSFS_file* filehandle = PHYSFS_openRead(file.data());

		if (filehandle != nullptr)
		{
			PHYSFS_sint64 size = PHYSFS_fileLength(filehandle);

			if (size > 0)
			{
				*buffer = new char[size];
				objCount = PHYSFS_readBytes(filehandle, *buffer, size);
			
				if (objCount == size)
				{
					//CONSOLE_LOG("FILE SYSTEM: Read %u bytes from file '%s'", objCount, fileName.data());
				}
				else
				{
					RELEASE(buffer);
					DEPRECATED_LOG("FILE SYSTEM: Could not read from file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
				}

				if (PHYSFS_close(filehandle) == 0)
					DEPRECATED_LOG("FILE SYSTEM: Could not close file '%s'. ERROR: %s", fileName.data(), PHYSFS_getLastError());
			}
		}
		else
			DEPRECATED_LOG("FILE SYSTEM: Could not open file '%s' to read. ERROR: %s", fileName.data(), PHYSFS_getLastError());
	}
	else
		DEPRECATED_LOG("FILE SYSTEM: Could not load file '%s' to read because it doesn't exist", fileName.data());

	return objCount;
}

bool ModuleFileSystem::AddMeta(const char* metaFile, int lastModTime)
{
	if (metaFile == nullptr)
	{
		assert(metaFile != nullptr);
		return false;
	}

	DEPRECATED_LOG("FILE SYSTEM: Successfully added/modified the meta '%s' in/from the metas map", metaFile);
	metas[metaFile] = lastModTime;

	return true;
}

bool ModuleFileSystem::DeleteMeta(const char* metaFile)
{
	bool ret = false;

	if (metaFile == nullptr)
	{
		assert(metaFile != nullptr);
		return ret;
	}

	if (metas.find(metaFile) != metas.end())
	{
		DEPRECATED_LOG("FILE SYSTEM: Successfully removed the meta '%s' from the metas map", metaFile);
		metas.erase(metaFile);
		ret = true;
	}
	else
		DEPRECATED_LOG("FILE SYSTEM: Meta '%s' was not found in the metas map and therefore could not be removed", metaFile);

	return ret;
}

void ModuleFileSystem::CheckFilesInAssets() const
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);
#endif

	// NOTE: Files in Library are not expected to be removed by the user

	for (std::map<std::string, int>::const_iterator it = metas.begin(); it != metas.end(); ++it)
	{
		// Each meta is expected to have an associated file in Assets that creates a resource

		// Path of the file in Assets associated to the meta
		std::string assetsFile = it->first;
		uint found = assetsFile.find_last_of(".");
		if (found != std::string::npos)
			assetsFile = assetsFile.substr(0, found);

		// CASE 1. Original file has been removed
		// + Original file has been renamed from outside the editor (we have no way to detect it)
		// + Original file has been moved to another folder from outside the editor (we have no way to detect it)
		if (assetsFiles.find(assetsFile.data()) == assetsFiles.end())
		{
			System_Event newEvent;
			newEvent.fileEvent.metaFile = new char[DEFAULT_BUF_SIZE];
			strcpy_s((char*)newEvent.fileEvent.metaFile, DEFAULT_BUF_SIZE, it->first.data());
			newEvent.type = System_Event_Type::FileRemoved;
			App->PushSystemEvent(newEvent);
		}
		// CASE 2. Meta has been removed
		else if (assetsFiles.find(it->first.data()) == assetsFiles.end())
		{
			System_Event newEvent;
			newEvent.fileEvent.metaFile = new char[DEFAULT_BUF_SIZE];
			strcpy_s((char*)newEvent.fileEvent.metaFile, DEFAULT_BUF_SIZE, it->first.data());
			newEvent.type = System_Event_Type::MetaRemoved;
			App->PushSystemEvent(newEvent);
		}
		// CASE 3. Original file has been overwritten
		else if (assetsFiles.find(assetsFile.data())->second != it->second)
		{
			System_Event newEvent;
			newEvent.fileEvent.metaFile = new char[DEFAULT_BUF_SIZE];
			strcpy_s((char*)newEvent.fileEvent.metaFile, DEFAULT_BUF_SIZE, it->first.data());
			newEvent.type = System_Event_Type::FileOverwritten;
			App->PushSystemEvent(newEvent);
		}
	}

	for (std::map<std::string, int>::const_iterator it = assetsFiles.begin(); it != assetsFiles.end(); ++it)
	{
		std::string extension;
		GetExtension(it->first.data(), extension);

		if (ModuleResourceManager::GetResourceTypeByExtension(extension.data()) != ResourceType::NoResourceType)
		{
			// Each file in Assets that creates a resource is expected to have an associated meta

			// Path of the meta associated to the file in Assets
			std::string meta = it->first;
			meta.append(EXTENSION_META);

			// CASE 4. A new file has been added
			if (metas.find(meta.data()) == metas.end())
			{
				System_Event newEvent;
				newEvent.fileEvent.file = new char[DEFAULT_BUF_SIZE];
				strcpy_s((char*)newEvent.fileEvent.file, DEFAULT_BUF_SIZE, it->first.data());
				newEvent.type = System_Event_Type::NewFile;
				App->PushSystemEvent(newEvent);
			}
		}
	}
}

std::string ModuleFileSystem::getAppPath()
{
	std::string baseDir = PHYSFS_getBaseDir();

	PHYSFS_unmount(".");

	AddPath((char*)baseDir.data(), "");

	if (Exists("physfs.dll"))
	{
		PHYSFS_unmount(baseDir.data());
		PHYSFS_mount(".", "", 0);
		return PHYSFS_getBaseDir();
	}

	else
	{
		PHYSFS_unmount(baseDir.data());

		for (int i = 0; i < 2; ++i)
		{
			baseDir = baseDir.substr(0, baseDir.find_last_of("\\"));
		}

		baseDir += "\\Game\\";

		AddPath((char*)baseDir.data(), "");

		std::string moretemp = baseDir + "physfs.dll";

		if (Exists("physfs.dll"))
		{
			PHYSFS_unmount(baseDir.data());
			PHYSFS_mount(".", "", 0);
			return baseDir;
		}

		PHYSFS_unmount(baseDir.data());
	}

	PHYSFS_unmount(baseDir.data());

	PHYSFS_mount(".", "", 0);

	return "";
}

bool ModuleFileSystem::MoveFileInto(const std::string & file, const std::string& newLocation)
{
	char* buffer;
	int size = Load(file, &buffer);
	if (size <= 0) 
	{
		CONSOLE_LOG("Couldn't move the file", LogTypes::Error);
		return false;
	}

	if (Save(newLocation, buffer, size) <= 0)
	{
		CONSOLE_LOG("Couldn't move the file", LogTypes::Error);
		delete[] buffer;
		return false;
	}

	if (!deleteFile(file))
	{
		CONSOLE_LOG("Couldn't move the file", LogTypes::Error);
		delete[] buffer;
		return false;
	}

	CONSOLE_LOG("File %s moved succesfully to %s.", LogTypes::Normal, file.data(), newLocation.data());

	delete[] buffer;
	return true;
}

bool ModuleFileSystem::CopyDirectoryAndContentsInto(const std::string& origin, const std::string& destination, bool keepRoot)
{
	Directory originDir = RecursiveGetFilesFromDir((char*)origin.data());

	for (int i = 0; i < originDir.files.size(); ++i)
	{
		std::string file = originDir.files[i].name;

		char* buffer;
		int size;

		size = Load(origin + "/" + file, &buffer);
		if (size <= 0)
			return false;

		std::string destinationWithRoot = destination + "/" + originDir.name + "/" + file;

		std::string destinationWithoutRoot = destinationWithRoot.at(0) == '/' ? destinationWithRoot.substr(1) : destinationWithRoot;
		destinationWithoutRoot = destinationWithoutRoot.substr(destinationWithoutRoot.find_first_of("/") + 1);

		std::string realDestination = keepRoot ? destinationWithRoot : destinationWithoutRoot;

		if (Save(realDestination, buffer, size) <= 0)
		{
			delete[] buffer;
			return false;
		}

		delete[] buffer;
	}

	for (int i = 0; i < originDir.directories.size(); ++i)
	{
		bool success = CopyDirectoryAndContentsInto(originDir.directories[i].fullPath, destination + "/" + originDir.name, keepRoot);
		if (!success)
			return false;
	}

	return true;
}

Directory ModuleFileSystem::RecursiveGetFilesFromDir(char* dir) const
{
	Directory ret;
	ret.fullPath = dir;
	std::string dirstr(dir);
	std::string name;
	int pos = dirstr.find_last_of("/");
	if (pos != std::string::npos)
	{
		name = dirstr.substr(pos + 1, std::string::npos);
	}
	else
	{
		name = dirstr;
	}

	ret.name = name;

	char** files = PHYSFS_enumerateFiles(dir);
	for (int i = 0; files[i] != nullptr; ++i)
	{
		std::string fulldir(dir + std::string("/") + std::string(files[i]));

		//First, check if PHYSFS can recognize this file as a real file or a directory. If a UNKNOWN_TYPE is thrown, then search the extension and use it to detect. This may fail sometimes.
		PHYSFS_Stat stats;
		PHYSFS_stat(fulldir.data(), &stats);

		if (stats.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_OTHER)
		{
			//Here use the extension as recognition-method.
			CONSOLE_LOG("Physfs could not recognize if \"%s\" is a file or a directory. Using the extension as recognition-method. Weird behaviors may happen.", LogTypes::Warning, files[i]);

			std::string file(files[i]);
			if (file.find(".") == std::string::npos) //It's a directory, have not extension
			{
				stats.filetype = PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY;
				CONSOLE_LOG("File \"%s\" was recognized as a directory", LogTypes::Warning, files[i]);
			}
			else
			{
				stats.filetype = PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR;
				CONSOLE_LOG("File \"%s\" was recognized as a regular file", LogTypes::Warning, files[i]);
			}
		}

		if (stats.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY)
		{
			Directory child = RecursiveGetFilesFromDir((char*)fulldir.data());
			child.fullPath = fulldir;
			ret.directories.push_back(child);
		}
		else if (stats.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR)
		{
			File file;
			file.lastModTime = stats.modtime; //Save the last modification time in order to know when a file has changed
			file.name = files[i];
			ret.files.push_back(file);
		}

	}
	PHYSFS_freeList(files);

	return ret;
}

bool ModuleFileSystem::deleteFile(const std::string& filePath) const
{
	return PHYSFS_delete(filePath.c_str()) != 0;
}

bool ModuleFileSystem::deleteFiles(const std::string& root, const std::string& extension) const
{
	Directory directory = RecursiveGetFilesFromDir((char*)root.c_str());

	for (int i = 0; i < directory.files.size(); ++i)
	{
		std::string fileExt; GetExtension(directory.files[i].name.data(), fileExt);

		if (fileExt == extension)
			PHYSFS_delete(std::string(directory.fullPath + "/" + directory.files[i].name).c_str());
	}

	for (int i = 0; i < directory.directories.size(); ++i)
	{
		deleteFiles(directory.directories[i].fullPath, extension);
	}

	return true;
}
