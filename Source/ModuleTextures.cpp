#include "ModuleTextures.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Globals.h"

#include "DevIL/include/il.h"
#include "DevIL/include/ilu.h"
#include "DevIL/include/ilut.h"

#pragma comment (lib, "DevIL/libx86/DevIL.lib")
#pragma comment (lib, "DevIL/libx86/ILU.lib")
#pragma comment (lib, "DevIL/libx86/ILUT.lib")

#define CHECKERS_HEIGHT 128
#define CHECKERS_WIDTH 128

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

bool ModuleTextures::Start()
{
	checkTextureID = LoadCheckImage();
	multitexturingTextureID = LoadImageFromFile("Assets\\Textures\\smile.png", false);
	multitexturingTexture2ID = LoadImageFromFile("Assets\\Textures\\mask.png", false);

	return true;
}

bool ModuleTextures::CleanUp()
{
	if (checkTextureID > 0)
		glDeleteTextures(1, (GLuint*)&checkTextureID);

	if (multitexturingTextureID > 0)
		glDeleteTextures(1, (GLuint*)&multitexturingTextureID);

	if (multitexturingTexture2ID > 0)
		glDeleteTextures(1, (GLuint*)&multitexturingTexture2ID);

	return true;
}

uint ModuleTextures::LoadImageFromFile(const char* path, bool needsMeshes)
{
	uint texName = 0;

	if (needsMeshes && App->renderer3D->GetNumMeshes() <= 0)
	{
		CONSOLE_LOG("Error at loading texture. ERROR: No meshes in the scene");
		return texName;
	}

	// Generate the image name
	uint imageName = 0;
	ilGenImages(1, (ILuint*)&imageName);

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
			// Create the texture
			if (needsMeshes)
				App->renderer3D->ClearTextures();

			texName = CreateTextureFromPixels(ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), ilGetInteger(IL_IMAGE_FORMAT), ilGetData());
			
			if (needsMeshes)
				App->renderer3D->AddTextureToMeshes(texName, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));
		}
		else
			CONSOLE_LOG("Image conversion failed. ERROR: %s", iluErrorString(ilGetError()));
	}
	else
		CONSOLE_LOG("DevIL could not load the image. ERROR: %s", iluErrorString(ilGetError()));

	ilDeleteImages(1, (const ILuint*)&imageName);
	
	return texName;
}

uint ModuleTextures::LoadCheckImage()
{
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

	// Create the texture
	return CreateTextureFromPixels(GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT, GL_RGBA, checkImage, true);
}

uint ModuleTextures::CreateTextureFromPixels(int internalFormat, uint width, uint height, uint format, const void* pixels, bool checkTexture)
{
	uint texName = 0;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate the texture name
	glGenTextures(1, (GLuint*)&texName);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture interpolation method
	if (checkTexture)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		/// Mipmap for the highest visual quality
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
		{
			GLfloat largest_supported_anisotropy;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	if (!checkTexture)
	{
		/// Mipmap
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	CONSOLE_LOG("Succes at loading texture: %i ID, %i Width, %i Height", texName, width, height);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texName;
}

uint ModuleTextures::GetCheckTextureID() const
{
	return checkTextureID;
}

uint ModuleTextures::GetMultitexturingTextureID() const
{
	return multitexturingTextureID;
}

uint ModuleTextures::GetMultitexturingTexture2ID() const
{
	return multitexturingTexture2ID;
}