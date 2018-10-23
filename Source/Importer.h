#ifndef __IMPORTER_H__
#define __IMPORTER_H__

#include "Globals.h"

#include <string>

class Importer
{
public:

	Importer() {}
	virtual ~Importer() {}

	virtual bool Import(const char* importFile, const char* importPath, const char* outputFile) = 0;
	virtual bool Import(const void* buffer, uint size, const char* outputFile) = 0;
};

#endif