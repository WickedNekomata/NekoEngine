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

MaterialImporter::MaterialImporter(bool start_enabled) {}

MaterialImporter::~MaterialImporter() {}

bool MaterialImporter::Init(JSON_Object * jObject)
{
	bool ret = true;

	// Check versions
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION)
	{
		CONSOLE_LOG("DevIL version is different. Exiting...");
		ret = false;
	}

	// Initialize all DevIL functionality
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL); // Tell DevIL that we're using OpenGL for our rendering

	return ret;
}

bool MaterialImporter::CleanUp()
{
	return true;
}

bool MaterialImporter::LoadImageFromFile(const char* path) const
{
	bool ret = false;

	uint texName = 0;

	// Generate the image name
	uint imageName = 0;
	ilGenImages(1, &imageName);

	// Bind the image
	ilBindImage(imageName);

	// Load the image
	if (ilLoadImage(path))
	{
		ILinfo imageInfo;
		iluGetImageInfo(&imageInfo);

		if (imageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
			iluFlipImage();

		// Convert the image into a suitable format to work with
		if (ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
		{		
			//App->renderer3D->ClearTextures();

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// Generate the texture name
			glGenTextures(1, &texName);

			// Bind the texture
			glBindTexture(GL_TEXTURE_2D, texName);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			// Set texture interpolation method
			/// Mipmap for the highest visual quality
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

			//App->renderer3D->AddTextureToMeshes(texName, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));

			CONSOLE_LOG("Succes at loading texture: %i ID, %i Width, %i Height", texName, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));

			ret = true;
		}
		else
			CONSOLE_LOG("Image conversion failed. ERROR: %s", iluErrorString(ilGetError()));
	}
	else
		CONSOLE_LOG("DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	ilDeleteImages(1, &imageName);
	
	return ret;
}

bool MaterialImporter::Import(const char* importFile, const char* path, const char* outputFile)
{
	bool ret = false;

	char* buffer;
	uint size = App->filesystem->Load(importFile, &buffer);
	if (size > 0)
	{
		CONSOLE_LOG("Successfully loaded texture %s", importFile);

		ret = Import(buffer, size, outputFile);
		RELEASE_ARRAY(buffer);
	}
	else
		CONSOLE_LOG("Could not load texture %s", importFile);

	return ret;
}

bool MaterialImporter::Import(const void* buffer, uint size, const char* outputFile)
{
	uint ret = 0;

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
				if (App->filesystem->Save(outputFile, data, size) > 0)
				{
					ret = true;
					CONSOLE_LOG("Successfully saved texture %s", outputFile);
				}
				else
					CONSOLE_LOG("Could not save texture %s", outputFile);
			}

			RELEASE_ARRAY(data);
		}

		ilDeleteImages(1, &imageName);
	}

	return ret;
}