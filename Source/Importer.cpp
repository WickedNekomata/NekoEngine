#include "Importer.h"

#include "Application.h"
#include "ModuleFileSystem.h"

#include <assert.h>

bool Importer::GetLastModificationTimeFromMeta(const char* metaFile, int& lastModTime)
{
	if (metaFile == nullptr)
	{
		assert(metaFile != nullptr);
		return false;
	}

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("SCENE IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("SCENE IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	lastModTime = json_object_get_number(rootObject, "Time Created");

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}