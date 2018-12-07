#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "Globals.h"

#include "ResourceTypes.h"

#include <string>

struct ImportSettings;

class Resource
{
public:
	
	Resource(ResourceType type, uint uuid);
	virtual ~Resource();

	uint GetUUID() const;
	ResourceType GetType() const;
	const char* GetFile() const;
	const char* GetExportedFile() const;

	bool IsInMemory() const;
	virtual uint LoadMemory();
	virtual uint UnloadMemory();

	uint CountReferences() const;

private:

	virtual bool LoadInMemory() = 0;
	virtual bool UnloadFromMemory() = 0;

public:

	std::string file;
	std::string exportedFile;

	bool isValid = true;

protected:

	ResourceType type = ResourceType::NoResourceType;
	uint UUID = 0;
	uint count = 0;
};


#endif