#include "MaterialImporter.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Globals.h"

#include "DevIL/include/il.h"
#include "DevIL/include/ilu.h"
#include "DevIL/include/ilut.h"

#pragma comment (lib, "DevIL/libx86/DevIL.lib")
#pragma comment (lib, "DevIL/libx86/ILU.lib")
#pragma comment (lib, "DevIL/libx86/ILUT.lib")

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
}

MaterialImporter::~MaterialImporter() {}

bool MaterialImporter::Import(const char* importFile, const char* importPath, const char* outputFile)
{
	bool ret = false;

	if (importPath == nullptr)
		return ret;

	char importFilePath[DEFAULT_BUF_SIZE];
	char outputFileName[DEFAULT_BUF_SIZE];

	strcpy_s(importFilePath, strlen(importPath) + 1, importPath);

	// If importFile == nullptr, it means that importPath already contains the name of the file to be imported
	// If outputFile == nullptr, use the name of importFile (or, if importFile == nullptr, find its name from importPath)
	if (outputFile != nullptr)
		strcpy_s(outputFileName, strlen(outputFile) + 1, outputFile);
	else if (importFile != nullptr)
	{
		strcat_s(importFilePath, strlen(importFile) + 1, importFile);
		strcpy_s(outputFileName, strlen(importFile) + 1, importFile);
	}
	else
	{
		const char* importFileName = App->filesystem->GetFileNameFromPath(importPath);
		strcpy_s(outputFileName, strlen(importFileName) + 1, importFileName);
	}

	char* buffer;
	uint size = App->filesystem->Load(importFilePath, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded texture %s (original format)", importFile);
		ret = Import(buffer, size, outputFileName);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load texture %s (original format)", importFile);

	return ret;
}

bool MaterialImporter::Import(const void* buffer, uint size, const char* outputFile)
{
	bool ret = false;

	if (buffer == nullptr || size <= 0)
		return ret;

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
		ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);

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
				if (App->filesystem->SaveInLibrary(outputFile, data, size, FileType::TextureFile) > 0)
				{
					CONSOLE_LOG("MATERIAL IMPORTER: Successfully saved texture %s to own format", outputFile);
					ret = true;
				}
				else
					CONSOLE_LOG("MATERIAL IMPORTER: Could not save texture %s to own format", outputFile);
			}

			RELEASE_ARRAY(data);
		}

		ilDeleteImages(1, &imageName);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	return ret;
}

bool MaterialImporter::Load(const char* exportedFile, Texture* outputTexture)
{
	bool ret = false;

	char* buffer;
	uint size = App->filesystem->LoadFromLibrary(exportedFile, &buffer, FileType::TextureFile);
	if (size > 0)
	{
		CONSOLE_LOG("MATERIAL IMPORTER: Successfully loaded texture %s (own format)", exportedFile);
		ret = Load(buffer, size, outputTexture);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("MATERIAL IMPORTER: Could not load texture %s (own format)", exportedFile);

	return ret;
}

bool MaterialImporter::Load(const void* buffer, uint size, Texture* outputTexture)
{
	bool ret = false;

	if (buffer == nullptr ||size <= 0 || outputTexture == nullptr)
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

			// Set texture clamping method
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			// Set texture interpolation method (Mipmap for the highest visual quality)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
			{
				GLfloat largest_supported_anisotropy;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
				0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

			glGenerateMipmap(GL_TEXTURE_2D);			

			outputTexture->id = texName;
			outputTexture->width = imageInfo.Width;
			outputTexture->height = imageInfo.Height;

			CONSOLE_LOG("MATERIAL IMPORTER: New texture loaded with: %i ID, %i x %i", texName, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));
			ret = true;
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

uint MaterialImporter::GetDevILVersion() const
{
	return ilGetInteger(IL_VERSION_NUM); 
	// == iluGetInteger(ILU_VERSION_NUM)
	// == ilutGetInteger(ILUT_VERSION_NUM)
}