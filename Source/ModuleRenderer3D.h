#ifndef __MODULE_RENDERER_3D_H__
#define __MODULE_RENDERER_3D_H__

#include "GameMode.h"

#include "Module.h"
#include "Globals.h"
#include "Light.h"

#include "MathGeoLib\include\Math\float4x4.h"

#include "glew\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include "SDL\include\SDL_video.h"

#include <vector>

class GameObject;
class ComponentMesh;
class ComponentCamera;
class QuadtreeNode;

struct DirectionalLight
{
	math::float3 direction;

	math::float3 ambient;
	math::float3 diffuse;
	math::float3 specular;
};

class ModuleRenderer3D : public Module
{
public:

	ModuleRenderer3D(bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSON_Object* jObject);
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

	bool OnGameMode();
	bool OnEditorMode();

	void OnResize(int width, int height);
	void CalculateProjectionMatrix();

	uint GetMaxTextureUnits() const;

	bool SetVSync(bool vsync);
	bool GetVSync() const;

	void SetCapabilityState(GLenum capability, bool enable) const;
	bool GetCapabilityState(GLenum capability) const;

	void SetWireframeMode(bool enable) const;
	bool IsWireframeMode() const;

	void SetDebugDraw(bool debugDraw);
	bool GetDebugDraw() const;

	void SetDrawBoundingBoxes(bool drawBoundingBoxes);
	bool GetDrawBoundingBoxes() const;

	void SetDrawCamerasFrustum(bool drawCamerasFrustum);
	bool GetDrawCamerasFrustum() const;

	void SetDrawQuadtree(bool drawQuadtree);
	bool GetDrawQuadtree() const;

	ComponentMesh* CreateMeshComponent(GameObject* parent);
	bool AddMeshComponent(ComponentMesh* toAdd);
	bool EraseMeshComponent(ComponentMesh* toErase);

	ComponentCamera* CreateCameraComponent(GameObject* parent);
	bool AddCameraComponent(ComponentCamera* toAdd);
	bool EraseCameraComponent(ComponentCamera* toErase);

	bool RecalculateMainCamera();
	bool SetMainCamera(ComponentCamera* mainCamera);
	bool SetCurrentCamera();
	ComponentCamera* GetMainCamera() const;
	ComponentCamera* GetCurrentCamera() const;

	void SetMeshComponentsSeenLastFrame(bool seenLastFrame);
	void FrustumCulling() const;

	void DrawSkybox();
	void DrawMesh(ComponentMesh* toDraw) const;
	void RecursiveDrawQuadtree(QuadtreeNode* node) const;

	void ClearSkybox();

private:

	std::vector<ComponentMesh*> meshComponents;

	std::vector<ComponentCamera*> cameraComponents;
	ComponentCamera* mainCamera = nullptr;
	ComponentCamera* currentCamera = nullptr;

	uint maxTextureUnits = 0;

public:

	// Skybox
	GLuint skyboxTexture = 0;
	std::vector<GLuint> skyboxTextures;
	GLuint skyboxVBO = 0;
	GLuint skyboxVAO = 0;

	DirectionalLight directionalLight;
	SDL_GLContext context;
	math::float3x3 NormalMatrix;
	math::float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

	bool vsync = false;

	bool debugDraw = false;
	bool drawBoundingBoxes = true;
	bool drawCamerasFrustum = true;
	bool drawQuadtree = false;
};

#endif