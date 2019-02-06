#include "Resource.h"

#include "Application.h"
#include "ModuleGOs.h"

#include <assert.h> 

Resource::Resource(ResourceType type, uint uuid) : type(type), UUID(uuid) 
{
	name = new char[DEFAULT_BUF_SIZE];
}

Resource::~Resource() 
{
	RELEASE_ARRAY(name);
}

void Resource::SetName(const char* name)
{
	strcpy_s((char*)this->name, DEFAULT_BUF_SIZE, name);
}

const char* Resource::GetName() const
{
	return name;
}

// Get UUID of the current resource.
uint Resource::GetUUID() const
{
	return UUID;
}

// Get type of the current resource.
ResourceType Resource::GetType() const
{
	return type;
}

// Returns true if the current resource is already loaded into vram.
bool Resource::IsInMemory() const
{
	return count > 0;
}

// Increase number of references and returns it. In case of 0 references also load into memory.
int Resource::LoadMemory()
{
	bool result = true;

	if (!IsInMemory())
		result = LoadInMemory();

	return result ? count++ : count;
}

// Decrease number of references and returns it. In case of 0 references also unload from memory.
int Resource::UnloadMemory()
{
	assert(count > 0 && "Calls to load and unload of resource not equivalent");

	bool result = true;

	if (count <= 1)
		result = UnloadFromMemory();

	assert(result && "Resource could not be unloaded from memory");

	return result ? count-- : count;
}

// Get the number of references.
int Resource::CountReferences() const
{
	return count;
}