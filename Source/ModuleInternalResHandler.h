#ifndef __MODULE_INTERNAL_RES_HANDLER_H__
#define __MODULE_INTERNAL_RES_HANDLER_H__

#include "Module.h"
#include "Globals.h"

#define PLANE_UUID 1428675893
#define CUBE_UUID 1451315056
#define DEFAULT_SHADER_PROGRAM_UUID 1608702687
#define DEFAULT_SHADER_PROGRAM_PARTICLE_UUID 2628722347
#define DEFAULT_SHADER_PROGRAM_UI_UUID 1246832795 
#define CUBEMAP_SHADER_PROGRAM_UUID 1676961097
#define DEFAULT_MATERIAL_UUID 2168314292
#define REPLACE_ME_TEXTURE_UUID 3462814329

enum ShaderProgramTypes;

class ModuleInternalResHandler : public Module
{
public:

	bool Start();

	// Mesh resources
	void CreatePlane();
	void CreateCube();


	// Shader resources
	void CreateDefaultShaderProgram(const char * vShader, const char * fShader, ShaderProgramTypes type);
	void CreateCubemapShaderProgram();
	void CreateUIShaderProgram();

	// Material resources
	void CreateDefaultMaterial();

public:

	// Mesh resources
	uint plane;
	uint cube;

	// Texture resources
	uint checkers;
	uint defaultTexture;
	// TODO: cubemap textures

	// Shader resources
	uint defaultShaderProgram;
	uint defaultParticleShaderProgram;
	uint cubemapShaderProgram;
	uint UIVertexShaderObject;
	uint UIFragmentShaderObject;
	uint UIShaderProgram;

	// Material resources
	uint defaultMaterial;
};

#endif
