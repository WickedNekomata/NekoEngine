#ifndef __MODULE_RENDERER_3D_H__
#define __MODULE_RENDERER_3D_H__

#include "Module.h"
#include "Globals.h"
#include "Light.h"

#include <list>

#include "MathGeoLib/include/Math/float3x3.h"
#include "MathGeoLib/include/Math/float4x4.h"

#include "glew\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include "SDL/include/SDL_video.h"

#define MAX_LIGHTS 8

class PrimitiveRay;

struct Mesh
{
	// Unique vertices
	float* vertices = nullptr;
	uint verticesID = 0;
	uint verticesSize = 0;

	// Indices
	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;

	// Normals (vertices normals)
	float* normals = nullptr;
	PrimitiveRay** normalsLines = nullptr;

	// Texture
	float* textureCoords = nullptr;
	uint textureCoordsID = 0;
	uint textureID = 0;

	// Colors
	float* colors = nullptr;

	// -----

	void Init();
	void EmbedTexture(uint textureID);
	~Mesh();
};

class ModuleRenderer3D : public Module
{
public:

	ModuleRenderer3D(bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSON_Object* jObject);
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	void OnResize(int width, int height);

	bool SetVSync(bool vsync);
	bool GetVSync() const;

	void SetCapabilityState(GLenum capability, bool enable) const;
	bool GetCapabilityState(GLenum capability) const;

	void SetWireframeMode(bool enable) const;
	bool IsWireframeMode() const;

	void SetDebugDraw(bool debugDraw);
	bool GetDebugDraw() const;

	math::float4x4 Perspective(float fovy, float aspect, float n, float f) const;

	// Meshes
	bool AddMesh(Mesh* mesh);
	bool RemoveMesh(Mesh* mesh);
	void ClearMeshes();
	void AddTextureToMeshes(uint textureID);

	void DrawMesh(Mesh* mesh) const;
	void DrawMeshNormals(Mesh* mesh) const;

public:

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	math::float3x3 NormalMatrix;
	math::float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

	bool vsync = false;
	bool debugDraw = false;

	// Meshes
	std::list<Mesh*> meshes;
};

#endif