#include "Resource.h"

#include <assert.h> 

Resource::Resource(ResourceType type, uint uuid) : type(type), UUID(uuid)
{
}

Resource::~Resource()
{
}

// Show resource data on inspector.
void Resource::OnEditor()
{
	OnUniqueEditor();
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
	return exported_file.data();
}

// Returns true if the current resource is already loaded into vram.
bool Resource::IsLoadedToMemory() const
{
	return count > 0;
}

// Increase number of references and returns it. In case of 0 references also load into memory.
uint Resource::LoadToMemory()
{
	if (!IsLoadedToMemory())
		LoadInMemory();

	return count++;
}

// Decrease number of references and returns it. In case of 0 references also unload from memory.
uint Resource::UnloadMemory()
{
	assert(count <= 0 && "Calls to load and unload of resource not equivalent");

	if (count <= 1)
		UnloadFromMemory();

	return count--;
}

// Get the number of references.
uint Resource::CountReferences() const
{
	return count;
}
