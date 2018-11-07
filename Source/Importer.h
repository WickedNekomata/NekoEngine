#ifndef __IMPORTER_H__
#define __IMPORTER_H__

#include "GameMode.h"

#include "Globals.h"

#include <string>

class Resource;

class Importer
{
public:

	Importer() {}
	virtual ~Importer() {}

	virtual bool Import(const char* importFile, const char* importPath, std::string& outputFileName) = 0;
	virtual bool Import(const void* buffer, uint size, std::string& outputFileName) = 0;
};

#endif