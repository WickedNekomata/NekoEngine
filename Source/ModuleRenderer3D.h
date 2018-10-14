#ifndef __MODULE_RENDERER_3D_H__
#define __MODULE_RENDERER_3D_H__

#include "Module.h"
#include "Globals.h"
#include "Light.h"

#include <vector>
#include <list>

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3x3.h"
#include "MathGeoLib/include/Math/float4x4.h"
#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Geometry/AABB.h"

#include "glew\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include "SDL/include/SDL_video.h"

#define MAX_LIGHTS 8

class PrimitiveRay;
class PrimitiveCube;

struct Mesh
{
	const char* name = nullptr;

	math::float3 position;
	math::float3 scale;
	math::Quat rotation;

	math::AABB boundingBox;
	PrimitiveCube* boundingBoxDebug = nullptr;

	// Unique vertices
	float* vertices = nullptr;
	uint verticesID = 0;
	uint verticesSize = 0;

	// Indices
	uint* indices = nullptr;
	uint indicesID = 0;
	uint indicesSize = 0;

	// Normals
	//float* normals = nullptr;
	//PrimitiveRay** normalsVerticesDebug = nullptr;
	//PrimitiveRay** normalsFacesDebug = nullptr;

	// Texture Coords
	float* textureCoords = nullptr;
	uint textureCoordsID = 0;

	// Texture (multitextures)
	uint* texturesID = nullptr;
	uint* texturesWidth = nullptr;
	uint* texturesHeight = nullptr;

	// -----

	void Init();
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

	bool SetVSync(bool vsync);
	bool GetVSync() const;
	void SetDebugDraw(bool debugDraw);
	bool GetDebugDraw() const;
	void SetDebugDrawVerticesNormals(bool debugVerticesNormals);
	bool GetDebugDrawVerticesNormals() const;
	void SetDebugDrawFacesNormals(bool debugFacesNormals);
	bool GetDebugDrawFacesNormals() const;
	void SetDebugDrawBoundingBoxes(bool debugBoundingBoxes);
	bool GetDebugDrawBoundingBoxes() const;

	void OnResize(int width, int height);
	void SetFOV(float fov);
	void SetClipPlanes(const math::float2& clipPlanes);
	void CalculateProjectionMatrix();
	math::float4x4 Perspective(float fovy, float aspect, float n, float f) const;

	void SetCapabilityState(GLenum capability, bool enable) const;
	bool GetCapabilityState(GLenum capability) const;
	void SetWireframeMode(bool enable) const;
	bool IsWireframeMode() const;

	// Meshes
	bool AddMesh(Mesh* mesh);
	bool RemoveMesh(Mesh* mesh);
	Mesh* GetMeshAt(uint index) const;
	uint GetNumMeshes() const;

	void ClearMeshes();

	void DrawMesh(Mesh* mesh) const;
	void DrawMeshVerticesNormals(Mesh* mesh) const;
	void DrawMeshFacesNormals(Mesh* mesh) const;

	// Textures
	bool AddTextureToMeshes(uint textureUnit, uint textureID, uint width, uint height);
	bool AddTextureToRemove(uint textureUnit);
	bool RemoveTextureFromMeshes(uint textureUnit);

	void ClearTextures();

	void SetMultitexturing(bool multitexturing);
	bool GetMultitexturing() const;
	uint GetMaxTextureUnits() const;
	void SetCurrentTextureUnits(uint currentTextureUnits);
	uint GetCurrentTextureUnits() const;
	void SetCheckTexture(bool checkTexture);
	bool IsCheckTexture() const;

	// Geometry
	void SetGeometryName(const char* geometryName);
	const char* GetGeometryName() const;

	void SetGeometryActive(bool geometryActive);
	bool IsGeometryActive() const;

	void CreateGeometryBoundingBox();
	void LookAtGeometry() const;

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

public:

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	math::float3x3 NormalMatrix;
	math::float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

private:

	bool vsync = false;
	bool debugDraw = false;
	bool debugDrawVerticesNormals = true;
	bool debugDrawFacesNormals = true;
	bool debugDrawBoundingBoxes = true;

	float fov = 0.0f;
	math::float2 clipPlanes = { 0.0f, 0.0f };

	std::vector<Mesh*> meshes;

	bool checkTexture = false;
	bool multitexturing = false;
	uint maxTextureUnits = 0;
	uint currentTextureUnits = 0;
	std::list<uint> texturesToRemove;

	const char* geometryName = nullptr;
	bool geometryActive = false;

	math::AABB geometryBoundingBox;
	PrimitiveCube* geometryBoundingBoxDebug = nullptr;
};

#endif