#include "GameMode.h"

#include "MaterialImporter.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Globals.h"
#include "ResourceTexture.h"

#include "DevIL/include/il.h"
#include "DevIL/include/ilu.h"
#include "DevIL/include/ilut.h"

#pragma comment (lib, "DevIL/libx86/DevIL.lib")
#pragma comment (lib, "DevIL/libx86/ILU.lib")
#pragma comment (lib, "DevIL/libx86/ILUT.lib")

// Reference: https://open.gl/textures

MaterialImporter::MaterialImporter()
{
	// Check versions
	bool versionCheck = true;
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: DevIL version is different. Exiting...");
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

	// Anisotropic filtering
	if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
	{
		isAnisotropySupported = true;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largestSupportedAnisotropy);
	}
}

MaterialImporter::~MaterialImporter() {}

bool MaterialImporter::Import(const char* importFileName, const char* importPath, std::string& outputFileName, const ImportSettings* importSettings) const
{
	bool ret = false;

	if (importPath == nullptr || importSettings == nullptr)
		return ret;

	std::string name;

	char fullImportPath[DEFAULT_BUF_SIZE];
	strcpy_s(fullImportPath, strlen(importPath) + 1, importPath);

	if (importFileName != nullptr)
	{
		name = importFileName;

		// Build the import path
		strcat_s(fullImportPath, strlen(fullImportPath) + strlen(importFileName) + 1, importFileName);
	}
	else
		App->filesystem->GetFileName(importPath, name);

	outputFileName = name.data();

	char* buffer;
	uint size = App->filesystem->Load(fullImportPath, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded Texture '%s' (original format)", name.data());

		outputFileName = name.data();
		ret = Import(buffer, size, outputFileName, importSettings);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load Texture '%s' (original format)", name.data());

	return ret;
}

bool MaterialImporter::Import(const void* buffer, uint size, std::string& outputFileName, const ImportSettings* importSettings) const
{
	bool ret = false;

	if (buffer == nullptr || size <= 0 || importSettings == nullptr)
		return ret;

	TextureImportSettings* textureImportSettings = (TextureImportSettings*)importSettings;

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

		switch (textureImportSettings->compression)
		{
		case TextureImportSettings::TextureCompression::DXT1:
			compression = IL_DXT1;
			break;
		case TextureImportSettings::TextureCompression::DXT2:
			compression = IL_DXT2;
			break;
		case TextureImportSettings::TextureCompression::DXT3:
			compression = IL_DXT3;
			break;
		case TextureImportSettings::TextureCompression::DXT4:
			compression = IL_DXT4;
			break;
		case TextureImportSettings::TextureCompression::DXT5:
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
				if (App->filesystem->SaveInLibrary((char*)data, size, FileType::TextureFile, outputFileName) > 0)
				{
					// TODO CHECK CRASH
					//CONSOLE_LOG("MATERIAL IMPORTER: Successfully saved Texture '%s' to own format", outputFileName);

					ret = true;
				}
				else
					CONSOLE_LOG("MATERIAL IMPORTER: Could not save Texture '%s' to own format", outputFileName);
			}

			RELEASE_ARRAY(data);
		}

		ilDeleteImages(1, &imageName);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	return ret;
}

void MaterialImporter::GenerateMeta(Resource* resource, const TextureImportSettings* textureImportSettings) const
{
	if (resource == nullptr)
		return;

	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	// Fill the JSON with data
	json_object_set_number(rootObject, "Time Created", App->timeManager->GetRealTime());
	json_object_set_boolean(rootObject, "UUID", resource->GetUUID());

	JSON_Value* materialImporterValue = json_value_init_object();
	JSON_Object* materialImporterObject = json_value_get_object(materialImporterValue);
	json_object_set_value(rootObject, "Material Importer", materialImporterValue);
	
	json_object_set_boolean(materialImporterObject, "Compression", textureImportSettings->compression);
	json_object_set_boolean(materialImporterObject, "Wrap S", textureImportSettings->wrapS);
	json_object_set_boolean(materialImporterObject, "Wrap T", textureImportSettings->wrapT);
	json_object_set_boolean(materialImporterObject, "Min Filter", textureImportSettings->minFilter);
	json_object_set_boolean(materialImporterObject, "Mag Filter", textureImportSettings->magFilter);
	json_object_set_number(materialImporterObject, "Anisotropy", textureImportSettings->anisotropy);

	// Build the path
	char path[DEFAULT_BUF_SIZE];
	strcpy_s(path, strlen(resource->file.data()) + 1, resource->file.data());

	const char extension[] = ".meta";
	strcat_s(path, strlen(path) + strlen(extension) + 1, extension);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);
	App->filesystem->Save(path, buf, sizeBuf);
	delete[] buf;
	json_value_free(rootValue);
}

bool MaterialImporter::GetTextureUUIDFromMeta(const char* metaFile, uint& UUID) const
{
	if (metaFile == nullptr)
		return false;

	char* buffer;
	uint size = App->filesystem->Load(metaFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	UUID = json_object_get_boolean(rootObject, "UUID");

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}

bool MaterialImporter::GetTextureImportSettingsFromMeta(const char* metaFile, TextureImportSettings* textureImportSettings) const
{
	if (metaFile == nullptr || textureImportSettings == nullptr)
		return false;

	char* buffer;
	uint size = App->filesystem->Load(metaFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	textureImportSettings->compression = (TextureImportSettings::TextureCompression)json_object_get_boolean(rootObject, "Compression");
	textureImportSettings->wrapS = (TextureImportSettings::TextureWrapMode)json_object_get_boolean(rootObject, "Wrap S");
	textureImportSettings->wrapT = (TextureImportSettings::TextureWrapMode)json_object_get_boolean(rootObject, "Wrap T");
	textureImportSettings->minFilter = (TextureImportSettings::TextureFilterMode)json_object_get_boolean(rootObject, "Min Filter");
	textureImportSettings->magFilter = (TextureImportSettings::TextureFilterMode)json_object_get_boolean(rootObject, "Mag Filter");
	textureImportSettings->anisotropy = json_object_get_number(rootObject, "Anisotropy");

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}

bool MaterialImporter::Load(const char* exportedFileName, Texture* outputTexture, const TextureImportSettings* textureImportSettings)
{
	bool ret = false;

	if (exportedFileName == nullptr || outputTexture == nullptr || textureImportSettings == nullptr)
		return ret;

	char* buffer;
	uint size = App->filesystem->LoadFromLibrary(exportedFileName, &buffer, FileType::TextureFile);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded Texture '%s' (own format)", exportedFileName);
		ret = Load(buffer, size, outputTexture, textureImportSettings);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load Texture '%s' (own format)", exportedFileName);

	return ret;
}

bool MaterialImporter::Load(const void* buffer, uint size, Texture* outputTexture, const TextureImportSettings* textureImportSettings)
{
	bool ret = false;

	if (buffer == nullptr ||size <= 0 || outputTexture == nullptr || textureImportSettings == nullptr)
		return ret;

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
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// Generate the texture name
			uint texName = 0;
			glGenTextures(1, &texName);

			// Bind the texture
			glBindTexture(GL_TEXTURE_2D, texName);

			// TODO: CHECK IF THIS WORKS
			// http://openil.sourceforge.net/tuts/tut_8/index.htm
			//iluAlienify();

			// Set texture wrap mode
			int wrap = 0;

			switch (textureImportSettings->wrapS)
			{
			case TextureImportSettings::TextureWrapMode::REPEAT:
				wrap = GL_REPEAT;
				break;
			case TextureImportSettings::TextureWrapMode::MIRRORED_REPEAT:
				wrap = GL_MIRRORED_REPEAT;
				break;
			case TextureImportSettings::TextureWrapMode::CLAMP_TO_EDGE:
				wrap = GL_CLAMP_TO_EDGE;
				break;
			case TextureImportSettings::TextureWrapMode::CLAMP_TO_BORDER:
				wrap = GL_CLAMP_TO_BORDER;
				break;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);

			switch (textureImportSettings->wrapT)
			{
			case TextureImportSettings::TextureWrapMode::REPEAT:
				wrap = GL_REPEAT;
				break;
			case TextureImportSettings::TextureWrapMode::MIRRORED_REPEAT:
				wrap = GL_MIRRORED_REPEAT;
				break;
			case TextureImportSettings::TextureWrapMode::CLAMP_TO_EDGE:
				wrap = GL_CLAMP_TO_EDGE;
				break;
			case TextureImportSettings::TextureWrapMode::CLAMP_TO_BORDER:
				wrap = GL_CLAMP_TO_BORDER;
				break;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

			// Set texture filter mode (Mipmap for the highest visual quality)
			int filter = 0;
			bool mipmap = false;

			switch (textureImportSettings->minFilter)
			{
			case TextureImportSettings::TextureFilterMode::NEAREST:
				filter = GL_NEAREST;
				break;
			case TextureImportSettings::TextureFilterMode::LINEAR:
				filter = GL_LINEAR;
				break;
			case TextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_NEAREST:
				filter = GL_NEAREST_MIPMAP_NEAREST;
				mipmap = true;
				break;
			case TextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_NEAREST:
				filter = GL_LINEAR_MIPMAP_LINEAR;
				mipmap = true;
				break;
			case TextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_LINEAR:
				filter = GL_NEAREST_MIPMAP_LINEAR;
				mipmap = true;
				break;
			case TextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_LINEAR:
				filter = GL_LINEAR_MIPMAP_LINEAR;
				mipmap = true;
				break;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

			switch (textureImportSettings->magFilter)
			{
			case TextureImportSettings::TextureFilterMode::NEAREST:
				filter = GL_NEAREST;
				break;
			case TextureImportSettings::TextureFilterMode::LINEAR:
				filter = GL_LINEAR;
				break;
			case TextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_NEAREST:
				filter = GL_NEAREST_MIPMAP_NEAREST;
				mipmap = true;
				break;
			case TextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_NEAREST:
				filter = GL_LINEAR_MIPMAP_LINEAR;
				mipmap = true;
				break;
			case TextureImportSettings::TextureFilterMode::NEAREST_MIPMAP_LINEAR:
				filter = GL_NEAREST_MIPMAP_LINEAR;
				mipmap = true;
				break;
			case TextureImportSettings::TextureFilterMode::LINEAR_MIPMAP_LINEAR:
				filter = GL_LINEAR_MIPMAP_LINEAR;
				mipmap = true;
				break;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

			// Anisotropic filtering
			// TODO: set this with the selected settings for the texture
			if (isAnisotropySupported)
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, textureImportSettings->anisotropy);

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
				0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

			if (mipmap)
				glGenerateMipmap(GL_TEXTURE_2D);

			outputTexture->id = texName;
			outputTexture->width = imageInfo.Width;
			outputTexture->height = imageInfo.Height;

			CONSOLE_LOG("MATERIAL IMPORTER: New texture loaded with: %i ID, %i x %i", texName, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));
			ret = true;

			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
			CONSOLE_LOG("MATERIAL IMPORTER: Image conversion failed. ERROR: %s", iluErrorString(ilGetError()));

		ilDeleteImages(1, &imageName);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));
}

#define CHECKERS_WIDTH 128
#define CHECKERS_HEIGHT 128

bool MaterialImporter::LoadCheckers(Texture* outputTexture)
{
	bool ret = false;

	if (outputTexture == nullptr)
		return ret;

	GLubyte checkImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];

	for (int i = 0; i < CHECKERS_HEIGHT; i++) {
		for (int j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			checkImage[i][j][0] = (GLubyte)c;
			checkImage[i][j][1] = (GLubyte)c;
			checkImage[i][j][2] = (GLubyte)c;
			checkImage[i][j][3] = (GLubyte)255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate the texture name
	uint texName = 0;
	glGenTextures(1, &texName);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, texName);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture interpolation method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
		0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

	glGenerateMipmap(GL_TEXTURE_2D);

	outputTexture->id = texName;
	outputTexture->width = CHECKERS_WIDTH;
	outputTexture->height = CHECKERS_HEIGHT;

	CONSOLE_LOG("MATERIAL IMPORTER: Succes at loading the checkers texture: %i ID, %i x %i", texName, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));
	ret = true;

	return ret;
}

bool MaterialImporter::IsAnisotropySupported() const
{
	return isAnisotropySupported;
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