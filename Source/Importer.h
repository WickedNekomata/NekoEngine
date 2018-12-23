#ifndef __IMPORTER_H__
#define __IMPORTER_H__

#include "GameMode.h"

#include "Globals.h"

#include <string>

struct ImportSettings {};

class Importer
{
public:

	Importer() {}
	virtual ~Importer() {}

	virtual bool Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const = 0;
	virtual bool Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings) const { return true; }

	static bool SetLastModificationTimeToMeta(const char* metaFile, int& lastModTime);
	static bool GetLastModificationTimeFromMeta(const char* metaFile, int& lastModTime);
};

#endif