#include "ResourceTexture.h"

#include "Application.h"
#include "ModuleGOs.h"
#include "MaterialImporter.h"

#include "glew\include\GL\glew.h"

ResourceTexture::ResourceTexture(ResourceType type, uint uuid) : Resource(type, uuid) {}

ResourceTexture::~ResourceTexture() 
{
	App->GOs->InvalidateResource(this);
}

bool ResourceTexture::LoadInMemory()
{
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file.data()) + 1, file.data()); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	TextureImportSettings* textureImportSettings = new TextureImportSettings();
	App->materialImporter->GetTextureImportSettingsFromMeta(metaFile, textureImportSettings);

	bool ret = App->materialImporter->Load(exportedFile.data(), this, textureImportSettings);

	if (!ret)
		return ret;

	return ret;
}

bool ResourceTexture::UnloadFromMemory()
{
	glDeleteTextures(1, (GLuint*)&id);

	id = 0;
	width = 0;
	height = 0;

	return true;
}
