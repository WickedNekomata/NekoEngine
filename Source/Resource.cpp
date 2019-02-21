#include "Resource.h"

#include "Application.h"
#include "ModuleFileSystem.h"

#include <assert.h> 

Resource::Resource(ResourceTypes type, uint uuid, ResourceData data) : type(type), uuid(uuid), data(data) {}

Resource::~Resource() {}

// ----------------------------------------------------------------------------------------------------

uint Resource::SetLastModTimeToMeta(const char* metaFile, const uint64_t& lastModTime)
{
	assert(metaFile != nullptr);

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		char* cursor = (char*)buffer;

		// 1. Store last modification time
		uint bytes = sizeof(int64_t);
		memcpy(cursor, &lastModTime, bytes);

		CONSOLE_LOG(LogTypes::Normal, "Resource: Successfully saved meta '%s'", metaFile);

		uint resultSize = App->fs->Save(metaFile, buffer, size);
		if (resultSize > 0)
		{
			CONSOLE_LOG(LogTypes::Normal, "Resource: Successfully saved meta '%s'", metaFile);
		}
		else
		{
			CONSOLE_LOG(LogTypes::Error, "Resource: Could not save meta '%s'", metaFile);
			return 0;
		}

		RELEASE_ARRAY(buffer);
	}
	else
	{
		CONSOLE_LOG(LogTypes::Error, "Resource: Could not saved meta '%s'", metaFile);
		return false;
	}

	return true;
}

uint Resource::IncreaseReferences()
{
	bool ret = true;

	if (!IsInMemory())
		ret = LoadInMemory();

	if (ret)
	{
		++count;
		CONSOLE_LOG(LogTypes::Normal, "The references of the resource '%s' have been incremented", data.file.data());
	}
	else
		CONSOLE_LOG(LogTypes::Error, "The references of the resource '%s' have not been incremented", data.file.data());

	return count;
}

uint Resource::DecreaseReferences()
{
	assert(IsInMemory());

	bool ret = true;

	if (IsLastInMemory())
		ret = UnloadFromMemory();
	assert(ret);

	--count;

	return count;
}

// ----------------------------------------------------------------------------------------------------

ResourceTypes Resource::GetType() const
{
	return type;
}

uint Resource::GetUuid() const
{
	return uuid;
}

uint Resource::GetReferencesCount() const
{
	return count;
}

bool Resource::IsInMemory() const
{
	return count > 0;
}

// ----------------------------------------------------------------------------------------------------

void Resource::SetFile(const char* file)
{
	data.file = file;
}

const char* Resource::GetFile() const
{
	return data.file.data();
}

void Resource::SetExportedFile(const char* exportedFile)
{
	data.exportedFile = exportedFile;
}

const char* Resource::GetExportedFile() const
{
	return data.exportedFile.data();
}

void Resource::SetName(const char* name)
{
	data.name = name;
}

const char* Resource::GetName() const
{
	return data.name.data();
}

// ----------------------------------------------------------------------------------------------------

bool Resource::IsLastInMemory() const
{
	return count == 1;
}