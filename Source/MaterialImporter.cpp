#include "MaterialImporter.h"

#include "Application.h"
#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ResourceTexture.h"

#include "glew\include\GL\glew.h"

#include "DevIL\include\il.h"
#include "DevIL\include\ilu.h"
#include "DevIL\include\ilut.h"

#include <assert.h>

#pragma comment (lib, "DevIL\\libx86\\DevIL.lib")
#pragma comment (lib, "DevIL\\libx86\\ILU.lib")
#pragma comment (lib, "DevIL\\libx86\\ILUT.lib")

// Reference: https://open.gl/textures

MaterialImporter::MaterialImporter()
{
	// Check versions
	bool versionCheck = true;
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION)
	{
		DEPRECATED_LOG("MATERIAL IMPORTER: DevIL version is different. Exiting...");
		versionCheck = false;
	}

	if (versionCheck)
	{
		// Initialize all DevIL functionality
		ilInit();
		iluInit();
		ilutInit();
		ilutRenderer(ILUT_OPENGL); // Tell DevIL that we're using OpenGL for our rendering
	}

	skyboxTextures.reserve(6);
}

MaterialImporter::~MaterialImporter() 
{
	glDeleteTextures(1, (GLuint*)&skyboxTexture);

	for (uint i = 0; i < skyboxTextures.size(); ++i)
		glDeleteTextures(1, (GLuint*)&skyboxTextures[i]);
}

bool MaterialImporter::Import(const char* file, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid) const
{
	assert(file != nullptr);

	bool ret = false;

	std::string fileName;
	App->fs->GetFileName(file, fileName);
	outputFile = fileName.data();

	char* buffer;
	uint size = App->fs->Load(file, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "MATERIAL IMPORTER: Successfully loaded Texture '%s' (original format)", outputFile.data());
		ret = Import(buffer, size, outputFile, importSettings, forcedUuid);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "MATERIAL IMPORTER: Could not load Texture '%s' (original format)", outputFile.data());

	return ret;
}

bool MaterialImporter::Import(const void* buffer, uint size, std::string& outputFile, const ResourceTextureImportSettings& importSettings, uint forcedUuid) const
{
	assert(buffer != nullptr && size > 0);

	bool ret = false;

	// Generate the image name
	uint imageName = 0;
	ilGenImages(1, &imageName);

	// Bind the image
	ilBindImage(imageName);

	// Load the image
	if (ilLoadL(IL_TYPE_UNKNOWN, buffer, size))
	{
		ilEnable(IL_FILE_OVERWRITE);

		uint size = 0;
		ILubyte* data = nullptr;

		// Pick a specific DXT compression use
		int compression = 0;

		switch (importSettings.compression)
		{
		case ResourceTextureImportSettings::TextureCompression::DXT1:
			compression = IL_DXT1;
			break;
		case ResourceTextureImportSettings::TextureCompression::DXT3:
			compression = IL_DXT3;
			break;
		case ResourceTextureImportSettings::TextureCompression::DXT5:
			compression = IL_DXT5;
			break;
		}

		ilSetInteger(IL_DXTC_FORMAT, compression);

		// Get the size of the data buffer
		size = ilSaveL(IL_DDS, NULL, 0);

		if (size > 0)
		{
			ilEnable(IL_FILE_OVERWRITE);

			// Allocate the data buffer
			data = new ILubyte[size];

			// Save to the buffer
			if (ilSaveL(IL_DDS, data, size) > 0)
			{


				uint uuid = forcedUuid == 0 ? App->GenerateRandomNumber() : forcedUuid;
				outputFile = std::to_string(uuid);
				if (App->fs->SaveInGame((char*)data, size, FileTypes::TextureFile, outputFile) > 0)
				{
					CONSOLE_LOG(LogTypes::Normal, "MATERIAL IMPORTER: Successfully saved Texture '%s' to own format", outputFile.data());
					ret = true;
				}
				else
					CONSOLE_LOG(LogTypes::Error, "MATERIAL IMPORTER: Could not save Texture '%s' to own format", outputFile.data());
			}

			RELEASE_ARRAY(data);
		}

		ilDeleteImages(1, &imageName);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "MATERIAL IMPORTER: DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	return ret;
}

bool MaterialImporter::Load(const char* exportedFile, ResourceData& outputData, ResourceTextureData& outputTextureData) const
{
	assert(exportedFile != nullptr);

	bool ret = false;

	char* buffer;
	uint size = App->fs->Load(exportedFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG(LogTypes::Normal, "MATERIAL IMPORTER: Successfully loaded Texture '%s' (own format)", exportedFile);
		ret = Load(buffer, size, outputData, outputTextureData);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "MATERIAL IMPORTER: Could not load Texture '%s' (own format)", exportedFile);

	return ret;
}

bool MaterialImporter::Load(const void* buffer, uint size, ResourceData& outputData, ResourceTextureData& outputTextureData) const
{
	assert(buffer != nullptr && size > 0);

	bool ret = false;

	// Generate the image name
	uint imageName = 0;
	ilGenImages(1, &imageName);

	// Bind the image
	ilBindImage(imageName);

	// Load the image
	if (ilLoadL(IL_DDS, buffer, size))
	{
		ILinfo imageInfo;
		iluGetImageInfo(&imageInfo);

		if (imageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
			iluFlipImage();

		// Convert the image into a suitable format to work with
		if (ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
		{
			uint size = imageInfo.Width * imageInfo.Height * ilGetInteger(IL_IMAGE_BPP);
			outputTextureData.data = new uchar[size];
			memcpy(outputTextureData.data, ilGetData(), size);

			outputTextureData.width = imageInfo.Width;
			outputTextureData.height = imageInfo.Height;
			outputTextureData.bpp = ilGetInteger(IL_IMAGE_BPP);

			CONSOLE_LOG(LogTypes::Normal, "MATERIAL IMPORTER: New texture loaded with: %i x %i", imageInfo.Width, imageInfo.Height);
			ret = true;
		}
		else
			CONSOLE_LOG(LogTypes::Error, "MATERIAL IMPORTER: Image conversion failed. ERROR: %s", iluErrorString(ilGetError()));

		ilDeleteImages(1, &imageName);
	}
	else
		CONSOLE_LOG(LogTypes::Error, "MATERIAL IMPORTER: DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	return ret;
}

// ----------------------------------------------------------------------------------------------------

void MaterialImporter::LoadInMemory(uint& id, const ResourceTextureData& textureData)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate the texture name
	uint texName = 0;
	glGenTextures(1, &texName);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, texName);

	// http://openil.sourceforge.net/tuts/tut_8/index.htm

	// Set texture wrap mode
	int wrap = 0;
	switch (textureData.textureImportSettings.wrapS)
	{
	case ResourceTextureImportSettings::TextureWrapMode::REPEAT:
		wrap = GL_REPEAT;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::MIRRORED_REPEAT:
		wrap = GL_MIRRORED_REPEAT;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::CLAMP_TO_EDGE:
		wrap = GL_CLAMP_TO_EDGE;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::CLAMP_TO_BORDER:
		wrap = GL_CLAMP_TO_BORDER;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);

	switch (textureData.textureImportSettings.wrapT)
	{
	case ResourceTextureImportSettings::TextureWrapMode::REPEAT:
		wrap = GL_REPEAT;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::MIRRORED_REPEAT:
		wrap = GL_MIRRORED_REPEAT;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::CLAMP_TO_EDGE:
		wrap = GL_CLAMP_TO_EDGE;
		break;
	case ResourceTextureImportSettings::TextureWrapMode::CLAMP_TO_BORDER:
		wrap = GL_CLAMP_TO_BORDER;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	// Set texture filter mode (Mipmap for the highest visual quality)
	int filter = 0;
	bool mipmap = false;

	switch (textureData.textureImportSettings.minFilter)
	{
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST:
		filter = GL_NEAREST;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR:
		filter = GL_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_NEAREST:
		filter = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_NEAREST:
		filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_LINEAR:
		filter = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_LINEAR:
		filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

	switch (textureData.textureImportSettings.magFilter)
	{
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST:
		filter = GL_NEAREST;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR:
		filter = GL_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_NEAREST:
		filter = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_NEAREST:
		filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_LINEAR:
		filter = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case ResourceTextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_LINEAR:
		filter = GL_LINEAR_MIPMAP_LINEAR;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	// Anisotropic filtering
	if (isAnisotropySupported)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, textureData.textureImportSettings.anisotropy);

	glTexImage2D(GL_TEXTURE_2D, 0, textureData.bpp, textureData.width, textureData.height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);

	if (textureData.textureImportSettings.UseMipmap())
		glGenerateMipmap(GL_TEXTURE_2D);

	id = texName;

	CONSOLE_LOG(LogTypes::Normal, "MATERIAL IMPORTER: New texture loaded with: %i id", texName);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void MaterialImporter::DeleteTexture(uint& name) const
{
	if (glIsTexture(name))
		glDeleteTextures(1, &name);
	name = 0;
}

// ----------------------------------------------------------------------------------------------------

void MaterialImporter::SetIsAnisotropySupported(bool isAnisotropySupported)
{
	this->isAnisotropySupported = isAnisotropySupported;
}

bool MaterialImporter::IsAnisotropySupported() const
{
	return isAnisotropySupported;
}

void MaterialImporter::SetLargestSupportedAnisotropy(float largestSupportedAnisotropy)
{
	this->largestSupportedAnisotropy = largestSupportedAnisotropy;
}

float MaterialImporter::GetLargestSupportedAnisotropy() const
{
	return largestSupportedAnisotropy;
}

uint MaterialImporter::GetDevILVersion() const
{
	return ilGetInteger(IL_VERSION_NUM);
	// == iluGetInteger(ILU_VERSION_NUM)
	// == ilutGetInteger(ILUT_VERSION_NUM)
}

// ----------------------------------------------------------------------------------------------------

/*
void MaterialImporter::LoadSkyboxTexture()
{
	skyboxTextures.clear();

	std::string outputFile;
	ResourceTexture* sky = new ResourceTexture(ResourceTypes::TextureResource, App->GenerateRandomNumber());
	TextureImportSettings* importSettings = new TextureImportSettings();

	/// Right
	App->materialImporter->Load("Settings/Skybox/right.nekoDDS", sky, importSettings);
	skyboxTextures.push_back(sky->id);
	/// Left
	App->materialImporter->Load("Settings/Skybox/left.nekoDDS", sky, importSettings);
	skyboxTextures.push_back(sky->id);
	/// Bottom
	App->materialImporter->Load("Settings/Skybox/bottom.nekoDDS", sky, importSettings);
	skyboxTextures.push_back(sky->id);
	/// Top
	App->materialImporter->Load("Settings/Skybox/top.nekoDDS", sky, importSettings);
	skyboxTextures.push_back(sky->id);
	/// Front
	App->materialImporter->Load("Settings/Skybox/front.nekoDDS", sky, importSettings);
	skyboxTextures.push_back(sky->id);
	/// Back
	App->materialImporter->Load("Settings/Skybox/back.nekoDDS", sky, importSettings);
	skyboxTextures.push_back(sky->id);

	RELEASE(importSettings);
	RELEASE(sky);

	skyboxTexture = LoadCubemapTexture(skyboxTextures);
}
*/

uint MaterialImporter::LoadCubemapTexture(std::vector<uint>& faces)
{
	uint result = 0;

	glGenTextures(1, &result);
	glBindTexture(GL_TEXTURE_CUBE_MAP, result);

	for (uint i = 0; i < 6; i++)
	{
		if (faces[i] == 0)
			continue;

		glBindTexture(GL_TEXTURE_2D, faces[i]);
		int w, h;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		GLubyte* pixels = new GLubyte[w*h * 3];

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		glBindTexture(GL_TEXTURE_CUBE_MAP, result);

		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels
		);

		delete[] pixels;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return result;
}

uint MaterialImporter::GetSkyboxTexture() const
{
	return skyboxTexture;
}

std::vector<uint> MaterialImporter::GetSkyboxTextures() const
{
	return skyboxTextures;
}