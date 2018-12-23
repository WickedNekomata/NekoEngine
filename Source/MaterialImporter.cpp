#include "GameMode.h"

#include "MaterialImporter.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleRenderer3D.h"
#include "ModuleTimeManager.h"
#include "Globals.h"
#include "ResourceTexture.h"

#include "DevIL\include\il.h"
#include "DevIL\include\ilu.h"
#include "DevIL\include\ilut.h"

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

	skyboxTextures.reserve(6);
}

MaterialImporter::~MaterialImporter() 
{
	glDeleteTextures(1, (GLuint*)&checkers);
	glDeleteTextures(1, (GLuint*)&defaultTexture);
	glDeleteTextures(1, (GLuint*)&skyboxTexture);

	for (uint i = 0; i < skyboxTextures.size(); ++i)
		glDeleteTextures(1, (GLuint*)&skyboxTextures[i]);
}

bool MaterialImporter::Import(const char* importFile, std::string& outputFile, const ImportSettings* importSettings) const
{
	bool ret = false;

	if (importFile == nullptr || importSettings == nullptr)
	{
		assert(importFile != nullptr && importSettings != nullptr);
		return ret;
	}

	std::string importFileName;
	App->fs->GetFileName(importFile, importFileName);
	outputFile = importFileName.data();

	// Search for the meta associated to the file
	char metaFile[DEFAULT_BUF_SIZE];
	strcpy_s(metaFile, strlen(importFile) + 1, importFile); // file
	strcat_s(metaFile, strlen(metaFile) + strlen(EXTENSION_META) + 1, EXTENSION_META); // extension

	char* buffer;
	uint size = App->fs->Load(importFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded Texture '%s' (original format)", outputFile.data());
		ret = Import(buffer, size, outputFile, importSettings, metaFile);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load Texture '%s' (original format)", outputFile.data());

	return ret;
}

bool MaterialImporter::Import(const void* buffer, uint size, std::string& outputFile, const ImportSettings* importSettings, const char* metaFile) const
{
	bool ret = false;

	if (buffer == nullptr || size <= 0 || importSettings == nullptr)
	{
		assert(buffer != nullptr && size > 0 && importSettings != nullptr);
		return ret;
	}

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
		case TextureImportSettings::TextureCompression::DXT3:
			compression = IL_DXT3;
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
				uint UUID = 0;
				if (metaFile != nullptr && App->fs->Exists(metaFile))
					GetTextureUUIDFromMeta(metaFile, UUID);
				if (UUID == 0)
					UUID = App->GenerateRandomNumber();

				outputFile = std::to_string(UUID);

				if (App->fs->SaveInGame((char*)data, size, FileType::TextureFile, outputFile) > 0)
				{
					CONSOLE_LOG("MATERIAL IMPORTER: Successfully saved Texture '%s' to own format", outputFile.data());
					ret = true;
				}
				else
					CONSOLE_LOG("MATERIAL IMPORTER: Could not save Texture '%s' to own format", outputFile.data());
			}

			RELEASE_ARRAY(data);
		}

		ilDeleteImages(1, &imageName);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	return ret;
}

bool MaterialImporter::GenerateMeta(Resource* resource, std::string& outputMetaFile, const TextureImportSettings* textureImportSettings) const
{
	if (resource == nullptr || textureImportSettings == nullptr)
	{
		assert(resource != nullptr && textureImportSettings != nullptr);
		return false;
	}

	JSON_Value* rootValue = json_value_init_object();
	JSON_Object* rootObject = json_value_get_object(rootValue);

	// Fill the JSON with data
	int lastModTime = App->fs->GetLastModificationTime(resource->file.data());
	json_object_set_number(rootObject, "Time Created", lastModTime);
	json_object_set_number(rootObject, "UUID", resource->GetUUID());

	JSON_Value* materialImporterValue = json_value_init_object();
	JSON_Object* materialImporterObject = json_value_get_object(materialImporterValue);
	json_object_set_value(rootObject, "Material Importer", materialImporterValue);
	
	json_object_set_number(materialImporterObject, "Compression", textureImportSettings->compression);
	json_object_set_number(materialImporterObject, "Wrap S", textureImportSettings->wrapS);
	json_object_set_number(materialImporterObject, "Wrap T", textureImportSettings->wrapT);
	json_object_set_number(materialImporterObject, "Min Filter", textureImportSettings->minFilter);
	json_object_set_number(materialImporterObject, "Mag Filter", textureImportSettings->magFilter);
	json_object_set_number(materialImporterObject, "Anisotropy", textureImportSettings->anisotropy);

	// Build the path of the meta file
	outputMetaFile.append(resource->file.data());
	outputMetaFile.append(EXTENSION_META);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);
	char* buf = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, buf, sizeBuf);
	
	uint size = App->fs->Save(outputMetaFile.data(), buf, sizeBuf);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully saved meta '%s'", outputMetaFile.data());
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not save meta '%s'", outputMetaFile.data());
		return false;
	}

	RELEASE_ARRAY(buf);
	json_value_free(rootValue);

	return true;
}

bool MaterialImporter::SetTextureUUIDToMeta(const char* metaFile, uint UUID) const
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
		//CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	json_object_set_number(rootObject, "UUID", UUID);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);

	RELEASE_ARRAY(buffer);

	char* newBuffer = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, newBuffer, sizeBuf);

	size = App->fs->Save(metaFile, newBuffer, sizeBuf);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully saved meta '%s' and set its UUID", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not save meta '%s' nor set its UUID", metaFile);
		return false;
	}

	RELEASE_ARRAY(newBuffer);
	json_value_free(rootValue);

	return true;
}

bool MaterialImporter::GetTextureUUIDFromMeta(const char* metaFile, uint& UUID) const
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
		//CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	UUID = json_object_get_number(rootObject, "UUID");

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}

bool MaterialImporter::SetTextureImportSettingsToMeta(const char* metaFile, const TextureImportSettings* textureImportSettings) const
{
	if (metaFile == nullptr || textureImportSettings == nullptr)
	{
		assert(metaFile != nullptr && textureImportSettings != nullptr);
		return false;
	}

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		//CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	JSON_Object* materialImporterObject = json_object_get_object(rootObject, "Material Importer");

	json_object_set_number(materialImporterObject, "Compression", textureImportSettings->compression);
	json_object_set_number(materialImporterObject, "Wrap S", textureImportSettings->wrapS);
	json_object_set_number(materialImporterObject, "Wrap T", textureImportSettings->wrapT);
	json_object_set_number(materialImporterObject, "Min Filter", textureImportSettings->minFilter);
	json_object_set_number(materialImporterObject, "Mag Filter", textureImportSettings->magFilter);
	json_object_set_number(materialImporterObject, "Anisotropy", textureImportSettings->anisotropy);

	// Create the JSON
	int sizeBuf = json_serialization_size_pretty(rootValue);

	RELEASE_ARRAY(buffer);

	char* newBuffer = new char[sizeBuf];
	json_serialize_to_buffer_pretty(rootValue, newBuffer, sizeBuf);

	size = App->fs->Save(metaFile, newBuffer, sizeBuf);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully saved meta '%s' and set its texture import settings", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not save meta '%s' nor set its texture import settings", metaFile);
		return false;
	}

	RELEASE_ARRAY(newBuffer);
	json_value_free(rootValue);

	return true;
}

bool MaterialImporter::GetTextureImportSettingsFromMeta(const char* metaFile, TextureImportSettings* textureImportSettings) const
{
	if (metaFile == nullptr || textureImportSettings == nullptr)
	{
		assert(metaFile != nullptr && textureImportSettings != nullptr);
		return false;
	}

	char* buffer;
	uint size = App->fs->Load(metaFile, &buffer);
	if (size > 0)
	{
		//CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded meta '%s'", metaFile);
	}
	else
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load meta '%s'", metaFile);
		return false;
	}

	textureImportSettings->metaFile = metaFile;

	JSON_Value* rootValue = json_parse_string(buffer);
	JSON_Object* rootObject = json_value_get_object(rootValue);

	JSON_Object* materialImporterObject = json_object_get_object(rootObject, "Material Importer");
	textureImportSettings->compression = (TextureImportSettings::TextureCompression)(uint)json_object_get_number(materialImporterObject, "Compression");
	textureImportSettings->wrapS = (TextureImportSettings::TextureWrapMode)(uint)json_object_get_number(materialImporterObject, "Wrap S");
	textureImportSettings->wrapT = (TextureImportSettings::TextureWrapMode)(uint)json_object_get_number(materialImporterObject, "Wrap T");
	textureImportSettings->minFilter = (TextureImportSettings::TextureFilterMode)(uint)json_object_get_number(materialImporterObject, "Min Filter");
	textureImportSettings->magFilter = (TextureImportSettings::TextureFilterMode)(uint)json_object_get_number(materialImporterObject, "Mag Filter");
	textureImportSettings->anisotropy = json_object_get_number(materialImporterObject, "Anisotropy");

	RELEASE_ARRAY(buffer);
	json_value_free(rootValue);

	return true;
}

bool MaterialImporter::Load(const char* exportedFile, ResourceTexture* outputTexture, const TextureImportSettings* textureImportSettings) const
{
	bool ret = false;

	if (exportedFile == nullptr || outputTexture == nullptr || textureImportSettings == nullptr)
	{
		assert(exportedFile != nullptr && outputTexture != nullptr && textureImportSettings != nullptr);
		return ret;
	}

	char* buffer;
	uint size = App->fs->Load(exportedFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded Texture '%s' (own format)", exportedFile);
		ret = Load(buffer, size, outputTexture, textureImportSettings);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load Texture '%s' (own format)", exportedFile);

	return ret;
}

bool MaterialImporter::Load(const void* buffer, uint size, ResourceTexture* outputTexture, const TextureImportSettings* textureImportSettings) const
{
	bool ret = false;

	if (buffer == nullptr || size <= 0 || outputTexture == nullptr || textureImportSettings == nullptr)
	{
		assert(buffer != nullptr && size > 0 && outputTexture != nullptr && textureImportSettings != nullptr);
		return ret;
	}

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

			// TODO (someday): http://openil.sourceforge.net/tuts/tut_8/index.htm // iluAlienify();

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

void MaterialImporter::LoadCheckers()
{
	GLubyte checkImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];

	for (uint i = 0; i < CHECKERS_HEIGHT; i++) {
		for (uint j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			checkImage[i][j][0] = (GLubyte)c;
			checkImage[i][j][1] = (GLubyte)c;
			checkImage[i][j][2] = (GLubyte)c;
			checkImage[i][j][3] = (GLubyte)255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate the texture name
	glGenTextures(1, &checkers);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, checkers);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture interpolation method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT,
		0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

	glGenerateMipmap(GL_TEXTURE_2D);

	CONSOLE_LOG("MATERIAL IMPORTER: Success at loading Checkers: %i ID, %i x %i", checkers, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));

	glBindTexture(GL_TEXTURE_2D, 0);
}

void MaterialImporter::LoadDefaultTexture()
{
	GLubyte replaceMeTexture[REPLACE_ME_WIDTH][REPLACE_ME_HEIGHT][4]; // REPLACE ME!

	for (uint i = 0; i < 2; i++) {
		for (uint j = 0; j < 2; j++) {
			replaceMeTexture[i][j][0] = (GLubyte)255;
			replaceMeTexture[i][j][1] = (GLubyte)0;
			replaceMeTexture[i][j][2] = (GLubyte)144;
			replaceMeTexture[i][j][3] = (GLubyte)255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate the texture name
	glGenTextures(1, &defaultTexture);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, defaultTexture);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture interpolation method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2,
		0, GL_RGBA, GL_UNSIGNED_BYTE, replaceMeTexture);

	glGenerateMipmap(GL_TEXTURE_2D);

	CONSOLE_LOG("MATERIAL IMPORTER: Success at loading Default Texture: %i ID, %i x %i", defaultTexture, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));

	glBindTexture(GL_TEXTURE_2D, 0);
}

void MaterialImporter::LoadSkyboxTexture()
{
	skyboxTextures.clear();

	std::string outputFile;
	ResourceTexture* sky = new ResourceTexture(ResourceType::TextureResource, App->GenerateRandomNumber());
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

bool MaterialImporter::DeleteTexture(uint texture)
{
	if (!glIsTexture(texture))
		return false;

	glDeleteTextures(1, &texture);
	return true;
}

uint MaterialImporter::GetCheckers() const
{
	return checkers;
}

uint MaterialImporter::GetDefaultTexture() const
{
	return defaultTexture;
}

uint MaterialImporter::GetSkyboxTexture() const
{
	return skyboxTexture;
}

std::vector<uint> MaterialImporter::GetSkyboxTextures() const
{
	return skyboxTextures;
}

uint MaterialImporter::GetDevILVersion() const
{
	return ilGetInteger(IL_VERSION_NUM); 
	// == iluGetInteger(ILU_VERSION_NUM)
	// == ilutGetInteger(ILUT_VERSION_NUM)
}