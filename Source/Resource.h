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

	void SetName(const char* name);
	const char* GetName() const;

	uint GetUUID() const;
	ResourceType GetType() const;

	bool IsInMemory() const;
	virtual int LoadMemory();
	virtual int UnloadMemory();

	int CountReferences() const;

private:

	virtual bool LoadInMemory() = 0;
	virtual bool UnloadFromMemory() = 0;

public:

	std::string file;
	std::string exportedFile;

	bool isValid = true;

protected:

	const char* name = nullptr;
	ResourceType type = ResourceType::NoResourceType;
	uint UUID = 0;
	int count = 0;
};


#endif