#include "ModuleTextures.h"
#include "Application.h"
#include "Globals.h"

#include "DevIL/include/il.h"
#include "DevIL/include/ilu.h"
#include "DevIL/include/ilut.h"

#pragma comment (lib, "DevIL/libx86/DevIL.lib")
#pragma comment (lib, "DevIL/libx86/ILU.lib")
#pragma comment (lib, "DevIL/libx86/ILUT.lib")

#define CHECKERS_HEIGHT 4
#define CHECKERS_WIDTH 4

ModuleTextures::ModuleTextures(bool start_enabled) {}

ModuleTextures::~ModuleTextures() {}

bool ModuleTextures::Init(JSON_Object * jObject)
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

bool ModuleTextures::CleanUp()
{
	return true;
}

uint ModuleTextures::LoadImageFromFile(const char* path) const
{
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
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// Generate the texture name
			glGenTextures(1, &texName);

			// Bind the texture
			glBindTexture(GL_TEXTURE_2D, texName);

			// Set texture clamping method
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			// Set texture interpolation method
			/// Mipmap for the highest visual quality
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
				0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

			//glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			CONSOLE_LOG("Image conversion failed. ERROR: %s", iluErrorString(ilGetError()));
	}
	else
		CONSOLE_LOG("DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	ilDeleteImages(1, &imageName);
	
	return texName;
}