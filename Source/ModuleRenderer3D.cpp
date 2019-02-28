#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"
#include "ModuleCameraEditor.h"
#include "ModuleResourceManager.h"
#include "ModuleTimeManager.h"
#include "ModulePhysics.h"
#include "ModuleParticles.h"
#include "ModuleGui.h"
#include "ModuleGOs.h"
#include "ModuleParticles.h"
#include "DebugDrawer.h"
#include "ShaderImporter.h"
#include "Quadtree.h"
#include "PanelSkybox.h"

#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ComponentRigidActor.h"
#include "ComponentRigidDynamic.h"
#include "ComponentCollider.h"
#include "ComponentEmitter.h"
#include "ComponentProjector.h"

#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"
#include "ResourceShaderProgram.h"

#include "ModuleNavigation.h"

#include "Brofiler\Brofiler.h"

#pragma comment(lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment(lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment(lib, "glew\\libx86\\glew32.lib")

#include <stdio.h>
#include <algorithm>

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module(start_enabled)
{
	name = "Renderer3D";
}

ModuleRenderer3D::~ModuleRenderer3D() {}

bool ModuleRenderer3D::Init(JSON_Object* jObject)
{
	bool ret = true;

	DEPRECATED_LOG("Creating 3D Renderer context");
	
	// Create context
	context = SDL_GL_CreateContext(App->window->window);

	if (context == NULL)
	{
		DEPRECATED_LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	
	if (ret)
	{
		LoadStatus(jObject);

		// Initialize glew
		GLenum error = glewInit();
		if (error != GL_NO_ERROR)
		{
			DEPRECATED_LOG("Error initializing glew! %s\n", glewGetErrorString(error));
			ret = false;
		}

		// Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			DEPRECATED_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			DEPRECATED_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// Initialize clear depth
		glClearDepth(1.0f);

		// Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			DEPRECATED_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		directionalLight.direction = math::float3(-0.2f, -1.0f, -0.2);
		directionalLight.ambient = math::float3(0.25f, 0.25f, 0.25f);
		directionalLight.diffuse = math::float3(0.5f, 0.5f, 0.5f);
		directionalLight.specular = math::float3(1.0f, 1.0f, 1.0f);

		// GL capabilities
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_MATERIAL);

		// Multitexturing
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, (GLint*)&maxTextureUnits);

		for (uint i = 0; i < maxTextureUnits; ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glEnable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE0);

		// Material Importer: anisotropic filtering
		if (GLEW_EXT_texture_filter_anisotropic)
		{
			float largestSupportedAnisotropy = 0;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largestSupportedAnisotropy);

			App->materialImporter->SetIsAnisotropySupported(true);
			App->materialImporter->SetLargestSupportedAnisotropy(largestSupportedAnisotropy);
		}

		// Shader Importer: binary formats
		GLint formats = 0;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
		App->shaderImporter->SetBinaryFormats(formats);
	}

	App->shaderImporter->LoadDefaultShader();
	App->shaderImporter->LoadCubemapShader();
	App->materialImporter->LoadCheckers();
	App->materialImporter->LoadDefaultTexture();
	//App->materialImporter->LoadSkyboxTexture();

	skyboxTextures = App->materialImporter->GetSkyboxTextures();
	skyboxTexture = App->materialImporter->GetSkyboxTexture();
	App->sceneImporter->LoadCubemap(skyboxVBO, skyboxVAO);

	// Load primitives
	App->sceneImporter->LoadPrimitivePlane();

#ifndef GAMEMODE
	// Editor camera
	currentCamera = App->camera->camera;
	// Projection Matrix
	OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());
#endif // GAME

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(currentCamera->GetOpenGLViewMatrix().ptr());

	return UPDATE_CONTINUE;
}

// PostUpdate: present buffer to screen
update_status ModuleRenderer3D::PostUpdate()
{
#ifndef GAMEMODE
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);
#endif
	DrawSkybox();

	// 1. Level geometry
	App->scene->Draw();

	if (currentCamera != nullptr)
	{
		if (currentCamera->HasFrustumCulling())
			FrustumCulling();

		//std::sort(meshComponents.begin(), meshComponents.end(), ComponentMeshComparator());

		for (uint i = 0; i < projectorComponents.size(); ++i)
		{
			if (projectorComponents[i]->IsActive())
				DrawProjectors(projectorComponents[i]);
		}

		for (uint i = 0; i < meshComponents.size(); ++i)
		{
			if (meshComponents[i]->IsActive() && meshComponents[i]->GetParent()->seenLastFrame)
				DrawMesh(meshComponents[i]);
		}
	}

	//Draw All particles
	//glDepthMask(GL_FALSE);
	App->particle->Draw();
	//glDepthMask(GL_TRUE);

#ifndef GAMEMODE

	App->navigation->Draw();

	// 2. Debug geometry
	if (debugDraw)
	{
		App->debugDrawer->StartDebugDraw();

		if (drawBoundingBoxes) // boundingBoxesColor = Yellow
		{
			Color boundingBoxesColor = Yellow;

			for (uint i = 0; i < meshComponents.size(); ++i)
				App->debugDrawer->DebugDraw(meshComponents[i]->GetParent()->boundingBox, boundingBoxesColor);
		}

		if (drawFrustums) // boundingBoxesColor = Grey
		{
			Color frustumsColor = Grey;

			for (uint i = 0; i < cameraComponents.size(); ++i)
				App->debugDrawer->DebugDraw(cameraComponents[i]->frustum, frustumsColor);

			for (uint i = 0; i < projectorComponents.size(); ++i)
				App->debugDrawer->DebugDraw(projectorComponents[i]->GetFrustum(), frustumsColor);
		}

		if (drawColliders) // boundingBoxesColor = Green
		{
			Color collidersColor = Green;

			std::vector<ComponentCollider*> colliderComponents = App->physics->GetColliderComponents();
			for (uint i = 0; i < colliderComponents.size(); ++i)
			{
				if (colliderComponents[i]->GetParent()->cmp_rigidActor == nullptr)
					continue;

				physx::PxShape* gShape = colliderComponents[i]->GetShape();
				if (gShape == nullptr)
					continue;

				physx::PxTransform actorGlobalPose = gShape->getActor()->getGlobalPose();
				physx::PxTransform shapeLocalPose = gShape->getLocalPose();
				physx::PxTransform globalPose = actorGlobalPose * shapeLocalPose;

				math::float4x4 globalMatrix(math::Quat(globalPose.q.x, globalPose.q.y, globalPose.q.z, globalPose.q.w),
					math::float3(globalPose.p.x, globalPose.p.y, globalPose.p.z));

				switch (gShape->getGeometryType())
				{
				case physx::PxGeometryType::Enum::eSPHERE:
				{
					physx::PxSphereGeometry gSphereGeometry;
					gShape->getSphereGeometry(gSphereGeometry);

					App->debugDrawer->DebugDrawSphere(gSphereGeometry.radius, collidersColor, globalMatrix);
				}
				break;
				case physx::PxGeometryType::Enum::eCAPSULE:
				{
					physx::PxCapsuleGeometry gCapsuleGeometry;
					gShape->getCapsuleGeometry(gCapsuleGeometry);

					App->debugDrawer->DebugDrawCapsule(gCapsuleGeometry.radius, gCapsuleGeometry.halfHeight, collidersColor, globalMatrix);
				}
				break;
				case physx::PxGeometryType::Enum::eBOX:
				{
					physx::PxBoxGeometry gBoxGeometry;
					gShape->getBoxGeometry(gBoxGeometry);

					App->debugDrawer->DebugDrawBox(math::float3(gBoxGeometry.halfExtents.x, gBoxGeometry.halfExtents.y, gBoxGeometry.halfExtents.z), collidersColor, globalMatrix);
				}
				break;
				case physx::PxGeometryType::Enum::ePLANE:
					App->debugDrawer->DebugDrawBox(math::float3(0.0f, 10.0f, 10.0f), collidersColor, globalMatrix);
					break;
				}
			}
		}

		if (drawRigidActors) // rigidActorsColor = Orange (static actors), Red and DarkRed (dynamic actors)
		{
			Color rigidActorsColor = Red;

			std::vector<ComponentRigidActor*> rigidActorComponents = App->physics->GetRigidActorComponents();
			for (uint i = 0; i < rigidActorComponents.size(); ++i)
			{
				physx::PxRigidActor* gActor = rigidActorComponents[i]->GetActor();
				physx::PxShape* gShape = nullptr;
				gActor->getShapes(&gShape, 1);
				if (gShape == nullptr)
					continue;

				if (rigidActorComponents[i]->GetType() == ComponentTypes::RigidStaticComponent)
					rigidActorsColor = Orange;
				else if (rigidActorComponents[i]->GetType() == ComponentTypes::RigidDynamicComponent
					&& !((ComponentRigidDynamic*)rigidActorComponents[i])->IsSleeping())
					rigidActorsColor = DarkRed;
				else
					rigidActorsColor = Red;

				physx::PxTransform transform = gActor->getGlobalPose();
				math::float4x4 globalMatrix(math::Quat(transform.q.x, transform.q.y, transform.q.z, transform.q.w),
					math::float3(transform.p.x, transform.p.y, transform.p.z));

				switch (gShape->getGeometryType())
				{
				case physx::PxGeometryType::Enum::eSPHERE:
				{
					physx::PxSphereGeometry gSphereGeometry;
					gShape->getSphereGeometry(gSphereGeometry);

					App->debugDrawer->DebugDrawSphere(gSphereGeometry.radius, rigidActorsColor, globalMatrix);
				}
				break;
				case physx::PxGeometryType::Enum::eCAPSULE:
				{
					physx::PxCapsuleGeometry gCapsuleGeometry;
					gShape->getCapsuleGeometry(gCapsuleGeometry);

					App->debugDrawer->DebugDrawCapsule(gCapsuleGeometry.radius, gCapsuleGeometry.halfHeight, rigidActorsColor, globalMatrix);
				}
				break;
				case physx::PxGeometryType::Enum::eBOX:
				{
					physx::PxBoxGeometry gBoxGeometry;
					gShape->getBoxGeometry(gBoxGeometry);

					App->debugDrawer->DebugDrawBox(math::float3(gBoxGeometry.halfExtents.x, gBoxGeometry.halfExtents.y, gBoxGeometry.halfExtents.z), rigidActorsColor, globalMatrix);
				}
				break;
				case physx::PxGeometryType::Enum::ePLANE:
					App->debugDrawer->DebugDrawBox(math::float3(0.0f, 100.0f, 100.0f), rigidActorsColor, globalMatrix);
					break;
				}
			}
		}

		if (drawQuadtree) // quadtreeColor = Blue, DarkBlue
			RecursiveDrawQuadtree(App->scene->quadtree.root);

		for (std::list<ComponentEmitter*>::iterator emitter = App->particle->emitters.begin(); emitter != App->particle->emitters.end(); ++emitter)
		{
			if ((*emitter)->drawShape)
			{
				math::float4x4 globalMat = (*emitter)->GetParent()->transform->GetGlobalMatrix();;
				switch ((*emitter)->normalShapeType)
				{
				case ShapeType_BOX:
					App->debugDrawer->DebugDraw((*emitter)->boxCreation, White, globalMat);
					break;
				case ShapeType_SPHERE:
				case ShapeType_SPHERE_BORDER:
				case ShapeType_SPHERE_CENTER:
					App->debugDrawer->DebugDrawSphere((*emitter)->sphereCreation.r, White, globalMat);
					break;
				case ShapeType_CONE:
					App->debugDrawer->DebugDrawCone((*emitter)->circleCreation.r, (*emitter)->coneHeight, White, globalMat);
					break;
				default:
					break;
				}
			}
			if((*emitter)->drawAABB)
				App->debugDrawer->DebugDraw((*emitter)->GetParent()->boundingBox, White);
		}
		App->debugDrawer->EndDebugDraw();
	}

	// 3. Editor
	App->gui->Draw();
#endif // GAME

	// 4. Swap buffers
	SDL_GL_MakeCurrent(App->window->window, context);
	SDL_GL_SwapWindow(App->window->window);

	return UPDATE_CONTINUE;
}

bool ModuleRenderer3D::CleanUp()
{
	bool ret = true;

	ClearSkybox();

	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteTextures(1, &skyboxTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	uint x, y;
	App->window->GetScreenSize(x, y);
	glViewport(0, 0, x, y);

	DEPRECATED_LOG("Destroying 3D Renderer");
	SDL_GL_DeleteContext(context);

	return ret;
}

void ModuleRenderer3D::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::Play:
		break;
	case System_Event_Type::Stop:
#ifndef GAMEMODE
		currentCamera = App->camera->camera;
#endif // GAME
		break;
	}
}

void ModuleRenderer3D::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_boolean(jObject, "vSync", vsync);

	json_object_set_boolean(jObject, "debugDraw", debugDraw);
	json_object_set_boolean(jObject, "drawBoundingBoxes", drawBoundingBoxes);
	json_object_set_boolean(jObject, "drawCamerasFrustum", drawFrustums);
	json_object_set_boolean(jObject, "drawQuadtree", drawQuadtree);
}
void ModuleRenderer3D::LoadStatus(const JSON_Object* jObject)
{
	SetVSync(json_object_get_boolean(jObject, "vSync"));

	debugDraw = json_object_get_boolean(jObject, "debugDraw");
	drawBoundingBoxes = json_object_get_boolean(jObject, "drawBoundingBoxes");
	drawFrustums = json_object_get_boolean(jObject, "drawCamerasFrustum");
	drawQuadtree = json_object_get_boolean(jObject, "drawQuadtree");
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);
	currentCamera->SetAspectRatio((float)width / (float)height);
	CalculateProjectionMatrix();
}

void ModuleRenderer3D::CalculateProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(currentCamera->GetOpenGLProjectionMatrix().ptr());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

uint ModuleRenderer3D::GetMaxTextureUnits() const
{
	return maxTextureUnits;
}

bool ModuleRenderer3D::SetVSync(bool vsync) 
{
	bool ret = true;

	this->vsync = vsync;

	if (this->vsync) 
	{

		if (SDL_GL_SetSwapInterval(1) == -1)
		{
			ret = false;
			DEPRECATED_LOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}
	}
	else {
	
		if (SDL_GL_SetSwapInterval(0) == -1) 
		{
			ret = false;
			DEPRECATED_LOG("Warning: Unable to set immediate updates! SDL Error: %s\n", SDL_GetError());
		}
	}

	return ret;
}

bool ModuleRenderer3D::GetVSync() const 
{
	return vsync;
}

void ModuleRenderer3D::SetCapabilityState(GLenum capability, bool enable) const
{
	if (GetCapabilityState(capability))
	{
		if (!enable)
			glDisable(capability);
	}
	else
	{
		if (enable)
			glEnable(capability);
	}
}

bool ModuleRenderer3D::GetCapabilityState(GLenum capability) const
{
	bool ret = false;

	if (glIsEnabled(capability))
		ret = true;

	return ret;
}

void ModuleRenderer3D::SetWireframeMode(bool enable) const 
{
	if (enable)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool ModuleRenderer3D::IsWireframeMode() const 
{
	bool ret = false;

	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);

	if (polygonMode[0] == GL_LINE && polygonMode[1] == GL_LINE)
		ret = true;

	return ret;
}

void ModuleRenderer3D::SetDebugDraw(bool debugDraw)
{
	this->debugDraw = debugDraw;
}

bool ModuleRenderer3D::GetDebugDraw() const 
{
	return debugDraw;
}

void ModuleRenderer3D::SetDrawBoundingBoxes(bool drawBoundingBoxes)
{
	this->drawBoundingBoxes = drawBoundingBoxes;
}

bool ModuleRenderer3D::GetDrawBoundingBoxes() const
{
	return drawBoundingBoxes;
}

void ModuleRenderer3D::SetDrawFrustums(bool drawFrustums)
{
	this->drawFrustums = drawFrustums;
}

bool ModuleRenderer3D::GetDrawFrustums() const
{
	return drawFrustums;
}

void ModuleRenderer3D::SetDrawColliders(bool drawColliders)
{
	this->drawColliders = drawColliders;
}

bool ModuleRenderer3D::GetDrawColliders() const
{
	return drawColliders;
}

void ModuleRenderer3D::SetDrawRigidActors(bool drawRigidActors)
{
	this->drawRigidActors = drawRigidActors;
}

bool ModuleRenderer3D::GetDrawRigidActors() const
{
	return drawRigidActors;
}

void ModuleRenderer3D::SetDrawQuadtree(bool drawQuadtree)
{
	this->drawQuadtree = drawQuadtree;
}

bool ModuleRenderer3D::GetDrawQuadtree() const
{
	return drawQuadtree;
}

bool ModuleRenderer3D::AddMeshComponent(ComponentMesh* toAdd)
{
	bool ret = true;

	std::vector<ComponentMesh*>::const_iterator it = std::find(meshComponents.begin(), meshComponents.end(), toAdd);
	ret = it == meshComponents.end();

	if (ret)
		meshComponents.push_back(toAdd);

	return ret;
}

bool ModuleRenderer3D::EraseMeshComponent(ComponentMesh* toErase)
{
	bool ret = false;

	std::vector<ComponentMesh*>::const_iterator it = std::find(meshComponents.begin(), meshComponents.end(), toErase);
	ret = it != meshComponents.end();

	if (ret)
		meshComponents.erase(it);

	return ret;
}

bool ModuleRenderer3D::AddProjectorComponent(ComponentProjector* toAdd)
{
	bool ret = true;

	std::vector<ComponentProjector*>::const_iterator it = std::find(projectorComponents.begin(), projectorComponents.end(), toAdd);
	ret = it == projectorComponents.end();

	if (ret)
		projectorComponents.push_back(toAdd);

	return ret;
}

bool ModuleRenderer3D::EraseProjectorComponent(ComponentProjector* toErase)
{
	bool ret = false;

	std::vector<ComponentProjector*>::const_iterator it = std::find(projectorComponents.begin(), projectorComponents.end(), toErase);
	ret = it != projectorComponents.end();

	if (ret)
		projectorComponents.erase(it);

	return ret;
}

bool ModuleRenderer3D::AddCameraComponent(ComponentCamera* toAdd)
{
	bool ret = true;

	std::vector<ComponentCamera*>::const_iterator it = std::find(cameraComponents.begin(), cameraComponents.end(), toAdd);
	ret = it == cameraComponents.end();

	if (ret)
		cameraComponents.push_back(toAdd);

	return ret;
}

bool ModuleRenderer3D::EraseCameraComponent(ComponentCamera* toErase)
{
	bool ret = false;

	if (cameraComponents.size() <= 0)
		return false;

	// TODO: Sometimes crash trying to erase editor camera. (its ok to call this method even if editor camera isnt in this vector, but it shouldnt crash)
	std::vector<ComponentCamera*>::const_iterator it = std::find(cameraComponents.begin(), cameraComponents.end(), toErase);
	ret = it != cameraComponents.end();

	if (ret)
		cameraComponents.erase(it);

	return ret;
}

bool ModuleRenderer3D::RecalculateMainCamera()
{
	bool ret = false;

	ComponentCamera* mainCamera = nullptr;

	bool multipleMainCameras = false;

	for (uint i = 0; i < cameraComponents.size(); ++i)
	{
		if (cameraComponents[i]->IsActive() && cameraComponents[i]->IsMainCamera())
		{
			if (mainCamera == nullptr)
				mainCamera = cameraComponents[i];
			else
			{
				multipleMainCameras = true;
				mainCamera = nullptr;
				break;
			}
		}			
	}

	if (multipleMainCameras)
	{
		DEPRECATED_LOG("Warning! More than 1 Main Camera is defined");
	}
	else if (mainCamera == nullptr)
	{
		DEPRECATED_LOG("Warning! No Main Camera is defined");
	}

	ret = SetMainCamera(mainCamera);

	return ret;
}

bool ModuleRenderer3D::SetMainCamera(ComponentCamera* mainCamera)
{
	bool ret = mainCamera != nullptr;

	if (ret)
		this->mainCamera = mainCamera;
	else
		DEPRECATED_LOG("Main Camera could not be set");

	return ret;
}

bool ModuleRenderer3D::SetCurrentCamera()
{
	bool ret = RecalculateMainCamera();

	if (ret)
	{
		currentCamera = mainCamera;
		SetMeshComponentsSeenLastFrame(!currentCamera->HasFrustumCulling());
	}
	else
		DEPRECATED_LOG("Current Camera could not be set");

	return ret;
}

ComponentCamera* ModuleRenderer3D::GetMainCamera() const
{
	return mainCamera;
}

ComponentCamera* ModuleRenderer3D::GetCurrentCamera() const
{
	return currentCamera;
}

void ModuleRenderer3D::SetMeshComponentsSeenLastFrame(bool seenLastFrame)
{
	for (uint i = 0; i < meshComponents.size(); ++i)
		meshComponents[i]->GetParent()->seenLastFrame = seenLastFrame;
}

void ModuleRenderer3D::FrustumCulling() const
{
	std::vector<GameObject*> gameObjects;
	App->GOs->GetGameobjects(gameObjects);

	for (uint i = 0; i < gameObjects.size(); ++i)
		gameObjects[i]->seenLastFrame = false;

	// Static objects
	std::vector<GameObject*> seen;
	App->scene->quadtree.CollectIntersections(seen, currentCamera->frustum);

	// Dynamic objects
	std::vector<GameObject*> dynamicGameObjects;
	App->GOs->GetDynamicGameobjects(dynamicGameObjects);

	for (uint i = 0; i < dynamicGameObjects.size(); ++i)
	{
		if (dynamicGameObjects[i]->boundingBox.IsFinite())
		{
			if (currentCamera->frustum.Intersects(dynamicGameObjects[i]->boundingBox))
				seen.push_back(dynamicGameObjects[i]);
		}
	}

	for (uint i = 0; i < seen.size(); ++i)
		seen[i]->seenLastFrame = true;
}

void ModuleRenderer3D::DrawSkybox()
{
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

	uint shaderProgram = App->shaderImporter->GetCubemapShaderProgram();
	glUseProgram(shaderProgram);

	//math::float4x4 view = glm::mat4(glm::mat3(App->camera->camera->GetOpenGLViewMatrix())); // remove translation from the view matrix
	uint location = glGetUniformLocation(shaderProgram, "view_matrix");
	glUniformMatrix4fv(location, 1, GL_FALSE, currentCamera->GetOpenGLViewMatrix().ptr());
	location = glGetUniformLocation(shaderProgram, "proj_matrix");
	glUniformMatrix4fv(location, 1, GL_FALSE, currentCamera->GetOpenGLProjectionMatrix().ptr());
	
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glDepthFunc(GL_LESS); // set depth function back to default
}

void ModuleRenderer3D::DrawMesh(ComponentMesh* toDraw) const
{
	if (toDraw->res == 0)
		return;

	uint textureUnit = 0;

	const ComponentMaterial* materialRenderer = toDraw->GetParent()->cmp_material;
	ResourceMaterial* resourceMaterial = (ResourceMaterial*)App->res->GetResource(materialRenderer->res);
	uint shaderUuid = resourceMaterial->GetShaderUuid();
	const ResourceShaderProgram* resourceShaderProgram = (const ResourceShaderProgram*)App->res->GetResource(shaderUuid);
	GLuint shader = resourceShaderProgram->shaderProgram;

	glUseProgram(shader);

	// 1. Generic uniforms
	LoadGenericUniforms(shader);

	// 2. Known mesh uniforms
	math::float4x4 model_matrix = toDraw->GetParent()->transform->GetGlobalMatrix();
	model_matrix = model_matrix.Transposed();
	math::float4x4 view_matrix = currentCamera->GetOpenGLViewMatrix();
	math::float4x4 proj_matrix = currentCamera->GetOpenGLProjectionMatrix();
	math::float4x4 mvp_matrix = model_matrix * view_matrix * proj_matrix;
	math::float4x4 normal_matrix = model_matrix;
	normal_matrix.Inverse();
	normal_matrix.Transpose();

	uint location = glGetUniformLocation(shader, "model_matrix");
	glUniformMatrix4fv(location, 1, GL_FALSE, model_matrix.ptr());
	location = glGetUniformLocation(shader, "mvp_matrix");
	glUniformMatrix4fv(location, 1, GL_FALSE, mvp_matrix.ptr());
	location = glGetUniformLocation(shader, "normal_matrix");
	glUniformMatrix3fv(location, 1, GL_FALSE, normal_matrix.Float3x3Part().ptr());

	// 3. Unknown mesh uniforms
	std::vector<Uniform> uniforms = resourceMaterial->GetUniforms();
	LoadSpecificUniforms(textureUnit, uniforms);

	// Mesh
	const ResourceMesh* mesh = (const ResourceMesh*)App->res->GetResource(toDraw->res);

	glBindVertexArray(mesh->GetVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBO());
	glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	for (uint i = 0; i < maxTextureUnits; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glUseProgram(0);
}

void ModuleRenderer3D::DrawProjectors(ComponentProjector* toDraw) const
{
	if (toDraw->GetMaterialRes() == 0)
		return;

	uint textureUnit = 0;

	ResourceMaterial* resourceMaterial = (ResourceMaterial*)App->res->GetResource(toDraw->GetMaterialRes());
	uint shaderUuid = resourceMaterial->GetShaderUuid();
	const ResourceShaderProgram* resourceShaderProgram = (ResourceShaderProgram*)App->res->GetResource(shaderUuid);
	GLuint shaderProgram = resourceShaderProgram->shaderProgram;

	glUseProgram(shaderProgram);

	// 1. Generic uniforms
	LoadGenericUniforms(shaderProgram);

	// 2. Known projector uniforms
	math::float4x4 bias_matrix = math::float4x4(
		0.5f, 0.0f, 0.0f, 0.5f,
		0.0f, 0.5f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.5f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	math::float4x4 projector_view_matrix = toDraw->GetOpenGLViewMatrix().Transposed();
	math::float4x4 projector_proj_matrix = toDraw->GetOpenGLProjectionMatrix().Transposed();
	math::float4x4 projector_matrix = bias_matrix * projector_proj_matrix * projector_view_matrix;																					   

	uint location = glGetUniformLocation(shaderProgram, "projector_matrix");
	glUniformMatrix4fv(location, 1, GL_TRUE, projector_matrix.ptr());

	// 3. Unknown projector uniforms
	std::vector<Uniform> uniforms = resourceMaterial->GetUniforms();
	std::vector<const char*> ignore;
	ignore.push_back("material.albedo");
	ignore.push_back("material.specular");
	LoadSpecificUniforms(textureUnit, uniforms, ignore);

	// Frustum culling
	/// Static objects
	std::vector<GameObject*> seenGameObjects;
	App->scene->quadtree.CollectIntersections(seenGameObjects, toDraw->GetFrustum());

	/// Dynamic objects
	std::vector<GameObject*> dynamicGameObjects;
	App->GOs->GetDynamicGameobjects(dynamicGameObjects);

	for (uint i = 0; i < dynamicGameObjects.size(); ++i)
	{
		if (dynamicGameObjects[i]->boundingBox.IsFinite() && toDraw->GetFrustum().Intersects(dynamicGameObjects[i]->boundingBox))
			seenGameObjects.push_back(dynamicGameObjects[i]);
	}

	// Meshes
	for (uint i = 0; i < seenGameObjects.size(); ++i)
	{
		const ComponentMesh* componentMesh = seenGameObjects[i]->cmp_mesh;
		if (componentMesh == nullptr)
			continue;

		Layer* layer = App->layers->GetLayer(componentMesh->GetParent()->GetLayer());

		if (!(toDraw->GetFilterMask() & layer->GetFilterGroup())
			&& componentMesh->IsActive() && componentMesh->GetParent()->seenLastFrame)
		{
			uint meshTextureUnit = textureUnit;

			const ComponentMaterial* materialRenderer = componentMesh->GetParent()->cmp_material;
			resourceMaterial = (ResourceMaterial*)App->res->GetResource(materialRenderer->res);

			// 2. Known mesh uniforms
			math::float4x4 model_matrix = componentMesh->GetParent()->transform->GetGlobalMatrix();
			model_matrix = model_matrix.Transposed();
			math::float4x4 view_matrix = currentCamera->GetOpenGLViewMatrix();
			math::float4x4 proj_matrix = currentCamera->GetOpenGLProjectionMatrix();
			math::float4x4 mvp_matrix = model_matrix * view_matrix * proj_matrix;
			math::float4x4 normal_matrix = model_matrix;
			normal_matrix.Inverse();
			normal_matrix.Transpose();

			uint location = glGetUniformLocation(shaderProgram, "model_matrix");
			glUniformMatrix4fv(location, 1, GL_FALSE, model_matrix.ptr());
			location = glGetUniformLocation(shaderProgram, "mvp_matrix");
			glUniformMatrix4fv(location, 1, GL_FALSE, mvp_matrix.ptr());
			location = glGetUniformLocation(shaderProgram, "normal_matrix");
			glUniformMatrix3fv(location, 1, GL_FALSE, normal_matrix.Float3x3Part().ptr());

			// 3. Unknown mesh uniforms
			std::vector<Uniform> uniforms = resourceMaterial->GetUniforms();
			uint albedoId = 0;
			uint specularId = 0;
			for (uint i = 0; i < uniforms.size(); ++i)
			{
				Uniform uniform = uniforms[i];
				if (strcmp(uniform.common.name, "material.albedo") == 0)
					albedoId = uniform.sampler2DU.value.id;
				else if (strcmp(uniform.common.name, "material.specular") == 0)
					specularId = uniform.sampler2DU.value.id;
			}

			location = glGetUniformLocation(shaderProgram, "material.albedo");
			if (meshTextureUnit < maxTextureUnits && albedoId > 0)
			{
				glActiveTexture(GL_TEXTURE0 + meshTextureUnit);
				glBindTexture(GL_TEXTURE_2D, albedoId);
				glUniform1i(location, meshTextureUnit);
				++meshTextureUnit;
			}
			location = glGetUniformLocation(shaderProgram, "material.specular");
			if (meshTextureUnit < maxTextureUnits && specularId > 0)
			{
				glActiveTexture(GL_TEXTURE0 + meshTextureUnit);
				glBindTexture(GL_TEXTURE_2D, specularId);
				glUniform1i(location, meshTextureUnit);
				++meshTextureUnit;
			}

			// Mesh
			const ResourceMesh* mesh = (const ResourceMesh*)App->res->GetResource(componentMesh->res);

			glBindVertexArray(mesh->GetVAO());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBO());
			glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, NULL);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			for (uint i = textureUnit; i < meshTextureUnit; ++i)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
	}

	for (uint i = 0; i < maxTextureUnits; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glUseProgram(0);
}

void ModuleRenderer3D::RecursiveDrawQuadtree(QuadtreeNode* node) const
{
	App->debugDrawer->DebugDraw(node->boundingBox, Blue);

	for (std::list<GameObject*>::const_iterator it = node->objects.begin(); it != node->objects.end(); ++it)
		App->debugDrawer->DebugDraw((*it)->boundingBox, DarkBlue);

	if (!node->IsLeaf())
	{
		for (uint i = 0; i < 4; ++i)
			RecursiveDrawQuadtree(node->children[i]);
	}
}

void ModuleRenderer3D::ClearSkybox()
{
	for (uint i = 0; i < skyboxTextures.size(); ++i) 
	{
		ResourceTexture* res = (ResourceTexture*)App->res->GetResource(skyboxTextures[i]);
		if (res != nullptr)
			App->res->SetAsUnused(res->GetUuid());
	}

	skyboxTextures.clear();
}

void ModuleRenderer3D::LoadSpecificUniforms(uint& textureUnit, const std::vector<Uniform>& uniforms, const std::vector<const char*>& ignore) const
{
	for (uint i = 0; i < uniforms.size(); ++i)
	{
		Uniform uniform = uniforms[i];

		for (uint j = 0; j < ignore.size(); ++j)
		{
			if (strcmp(uniform.common.name, ignore[j]) == 0)
				goto hereWeGo;
		}

		switch (uniform.common.type)
		{
		case Uniforms_Values::FloatU_value:
			glUniform1f(uniform.common.location, uniform.floatU.value);
			break;
		case Uniforms_Values::IntU_value:
			glUniform1i(uniform.common.location, uniform.intU.value);
			break;
		case Uniforms_Values::Vec2FU_value:
			glUniform2f(uniform.common.location, uniform.vec2FU.value.x, uniform.vec2FU.value.y);
			break;
		case Uniforms_Values::Vec3FU_value:
			glUniform3f(uniform.common.location, uniform.vec3FU.value.x, uniform.vec3FU.value.y, uniform.vec3FU.value.z);
			break;
		case Uniforms_Values::Vec4FU_value:
			glUniform4f(uniform.common.location, uniform.vec4FU.value.x, uniform.vec4FU.value.y, uniform.vec4FU.value.z, uniform.vec4FU.value.w);
			break;
		case Uniforms_Values::Vec2IU_value:
			glUniform2i(uniform.common.location, uniform.vec2IU.value.x, uniform.vec2IU.value.y);
			break;
		case Uniforms_Values::Vec3IU_value:
			glUniform3i(uniform.common.location, uniform.vec3IU.value.x, uniform.vec3IU.value.y, uniform.vec3IU.value.z);
			break;
		case Uniforms_Values::Vec4IU_value:
			glUniform4i(uniform.common.location, uniform.vec4IU.value.x, uniform.vec4IU.value.y, uniform.vec4IU.value.z, uniform.vec4IU.value.w);
			break;
		case Uniforms_Values::Sampler2U_value:
			if (textureUnit < maxTextureUnits && uniform.sampler2DU.value.id > 0)
			{
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				glBindTexture(GL_TEXTURE_2D, uniform.sampler2DU.value.id);
				glUniform1i(uniform.common.location, textureUnit);
				++textureUnit;
			}
			break;
		}

	hereWeGo:;
	}
}

void ModuleRenderer3D::LoadGenericUniforms(uint shaderProgram) const
{
	uint location = glGetUniformLocation(shaderProgram, "light.direction");
	glUniform3fv(location, 1, directionalLight.direction.ptr());
	location = glGetUniformLocation(shaderProgram, "light.ambient");
	glUniform3fv(location, 1, directionalLight.ambient.ptr());
	location = glGetUniformLocation(shaderProgram, "light.diffuse");
	glUniform3fv(location, 1, directionalLight.diffuse.ptr());
	location = glGetUniformLocation(shaderProgram, "light.specular");
	glUniform3fv(location, 1, directionalLight.specular.ptr());

	location = glGetUniformLocation(shaderProgram, "viewPos");
	glUniform3fv(location, 1, currentCamera->frustum.pos.ptr());
	location = glGetUniformLocation(shaderProgram, "Time");
	switch (App->GetEngineState())
	{
		// Game
	case ENGINE_PLAY:
	case ENGINE_PAUSE:
	case ENGINE_STEP:
		glUniform1f(location, App->timeManager->GetTime());
		break;

		// Editor
	case ENGINE_EDITOR:
		glUniform1f(location, App->timeManager->GetRealTime());
		break;
	}
}