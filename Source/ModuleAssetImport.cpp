#include "ModuleAssetImport.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

ModuleAssetImport::ModuleAssetImport(bool start_enabled)
{
}

ModuleAssetImport::~ModuleAssetImport()
{
}

bool ModuleAssetImport::Init(JSON_Object * jObject)
{
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);
	return true;
}

bool ModuleAssetImport::CleanUp()
{
	aiDetachAllLogStreams();
	return true;
}
