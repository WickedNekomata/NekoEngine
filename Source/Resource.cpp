#include "Resource.h"

#include "Application.h"
#include "ModuleGOs.h"

#include <assert.h> 

Resource::Resource(ResourceType type, uint uuid) : type(type), UUID(uuid) {}

Resource::~Resource() 
{
	App->GOs->InvalidateResource(this);
}

// Get UUID of the current resource.
uint Resource::GetUUID() const
{
	return UUID;
}

// Get file of the current resource.
const char* Resource::GetFile() const
{
	return file.data();
}

// Get the exported file of the current resource.
const char* Resource::GetExportedFile() const
{
	return exportedFile.data();
}

// Returns true if the current resource is already loaded into vram.
bool Resource::IsInMemory() const
{
	return count > 0;
}

// Increase number of references and returns it. In case of 0 references also load into memory.
uint Resource::LoadToMemory()
{
	if (!IsInMemory())
		LoadInMemory();

	return count++;
}

// Decrease number of references and returns it. In case of 0 references also unload from memory.
uint Resource::UnloadMemory()
{
	assert(count > 0 && "Calls to load and unload of resource not equivalent");

	if (count <= 1)
		UnloadFromMemory();

	return count--;
}

// Get the number of references.
uint Resource::CountReferences() const
{
	return count;
}

ResourceType Resource::GetType() const
{
	return type;
}