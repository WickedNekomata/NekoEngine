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

	virtual void OnEditor();

	uint GetUUID() const;
	const char* GetFile() const;
	const char* GetExportedFile() const;
	bool IsLoadedToMemory() const;
	uint LoadToMemory();
	uint UnloadMemory();
	uint CountReferences() const;

private:

	virtual void OnUniqueEditor() = 0;

	virtual bool LoadInMemory() = 0;
	virtual bool UnloadFromMemory() = 0;

public:

	std::string file;
	std::string exportedFile;

protected:

	uint UUID;
	ResourceType type;
	ImportSettings* settings = nullptr;
	uint count = 0u;
};


#endif