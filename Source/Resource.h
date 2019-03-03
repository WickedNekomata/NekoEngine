#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "ResourceTypes.h"

#include "Globals.h"

#include <string>

struct ResourceData
{
	std::string file;
	std::string exportedFile;
	std::string name;

	bool internal = false;
};

class Resource
{
public:
	
	Resource(ResourceTypes type, uint uuid, ResourceData data);
	virtual ~Resource();

	virtual void OnPanelAssets() = 0;

	// ----------------------------------------------------------------------------------------------------
	
	virtual bool GenerateLibraryFiles() const { return true; }

	static uint SetLastModTimeToMeta(const char* metaFile, const uint64_t& lastModTime);

	uint IncreaseReferences();
	uint DecreaseReferences();

	// ----------------------------------------------------------------------------------------------------

	ResourceTypes GetType() const;
	uint GetUuid() const;
	uint GetReferencesCount() const;
	bool IsInMemory() const;

	inline ResourceData& GetData() { return data; }
	void SetFile(const char* file);
	const char* GetFile() const;
	void SetExportedFile(const char* exportedFile);
	const char* GetExportedFile() const;
	void SetName(const char* name);
	const char* GetName() const;

	// ----------------------------------------------------------------------------------------------------

private:

	virtual bool LoadInMemory() = 0;
	virtual bool UnloadFromMemory() = 0;

	bool IsLastInMemory() const;

protected:

	ResourceTypes type = ResourceTypes::NoResourceType;
	uint uuid = 0;
	uint count = 0;

	ResourceData data;
};

#endif