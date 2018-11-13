#include "ResourceTexture.h"

#include "Application.h"
#include "MaterialImporter.h"
#include "glew/include/GL/glew.h"

ResourceTexture::ResourceTexture(ResourceType type, uint uuid) : Resource(type, uuid) {}

ResourceTexture::~ResourceTexture() {}

bool ResourceTexture::LoadInMemory()
{
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(file.data()) + 1, file.data()); // file
	const char metaExtension[] = ".meta";
	strcat_s(metaFile, strlen(metaFile) + strlen(metaExtension) + 1, metaExtension); // extension

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

	return true;
}
