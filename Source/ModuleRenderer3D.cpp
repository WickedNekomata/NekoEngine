#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Primitive.h"
#include "Color.h"

#include "MathGeoLib/include/Geometry/Sphere.h"

#pragma comment(lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment(lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment(lib, "glew/libx86/glew32.lib")

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module(start_enabled)
{
	name = "Renderer3D";
}

ModuleRenderer3D::~ModuleRenderer3D()
{}

bool ModuleRenderer3D::Init(JSON_Object* jObject)
{
	bool ret = true;

	SetVSync(json_object_get_boolean(jObject, "vSync"));
	if (vsync && App->GetCapFrames())
		App->SetCapFrames(false);
	debugDraw = json_object_get_boolean(jObject, "debugDraw");
	fov = json_object_get_number(jObject, "fov");
	clipPlanes = math::float2(json_object_get_number(jObject, "nearClipPlane"), json_object_get_number(jObject, "farClipPlane"));

	CONSOLE_LOG("Creating 3D Renderer context");
	
	// Create context
	context = SDL_GL_CreateContext(App->window->window);

	if (context == NULL)
	{
		CONSOLE_LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	
	if (ret)
	{
		// Initialize glew
		GLenum error = glewInit();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG("Error initializing glew! %s\n", glewGetErrorString(error));
			ret = false;
		}

		// Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// Initialize clear depth
		glClearDepth(1.0f);

		// Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			CONSOLE_LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		GLfloat LightModelAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();
		lights[0].Active(true);

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		// GL capabilities
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_BLEND);

		uint units = 0;
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, (GLint*)&units);

		/// Enable Multitexturing
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}

	// Projection Matrix for
	OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetViewMatrix());

	// Light 0 on cam pos
	lights[0].SetPos(App->camera->position.x, App->camera->position.y, App->camera->position.z);

	for (uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	return UPDATE_CONTINUE;
}

// PostUpdate: present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	// 1. Level geometry
	App->scene->Draw();

	if (geometryActive)
	{
		for (uint i = 0; i < meshes.size(); ++i)
			DrawMesh(meshes[i]);
	}

	// 2. Debug geometry
	if (debugDraw)
	{
		if (geometryActive)
		{
			for (uint i = 0; i < meshes.size(); ++i)
			{
				if (debugVerticesNormals)
					DrawMeshVerticesNormals(meshes[i]);
				if (debugFacesNormals)
					DrawMeshFacesNormals(meshes[i]);

				if (meshes[i]->boundingBoxDebug != nullptr && debugBoundingBox)
					meshes[i]->boundingBoxDebug->Render();
			}

			if (geometryBoundingBoxDebug != nullptr && debugBoundingBox)
				geometryBoundingBoxDebug->Render();
		}
	}

	// 3. Editor
	App->gui->Draw();

	// 4. Swap buffers
	SDL_GL_SwapWindow(App->window->window);

	return UPDATE_CONTINUE;
}

bool ModuleRenderer3D::CleanUp()
{
	bool ret = true;

	RELEASE_ARRAY(geometryName);

	ClearTextures();
	ClearMeshes();

	CONSOLE_LOG("Destroying 3D Renderer");
	SDL_GL_DeleteContext(context);

	return ret;
}

void ModuleRenderer3D::SaveStatus(JSON_Object* jObject) const
{
	json_object_set_boolean(jObject, "vSync", vsync);
	json_object_set_boolean(jObject, "debugDraw", debugDraw);
	json_object_set_number(jObject, "fov", fov);
	json_object_set_number(jObject, "nearClipPlane", clipPlanes.x);
	json_object_set_number(jObject, "farClipPlane", clipPlanes.y);
}
void ModuleRenderer3D::LoadStatus(const JSON_Object* jObject)
{
	SetVSync(json_object_get_boolean(jObject, "vSync"));
	if (vsync && App->GetCapFrames())
		App->SetCapFrames(false);
	debugDraw = json_object_get_boolean(jObject, "debugDraw");
	fov = json_object_get_number(jObject, "fov");
	clipPlanes = math::float2(json_object_get_number(jObject, "nearClipPlane"), json_object_get_number(jObject, "farClipPlane"));
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	CalculateProjectionMatrix();
}

void ModuleRenderer3D::CalculateProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	ProjectionMatrix = Perspective(fov, (float)App->window->GetWindowWidth() / (float)App->window->GetWindowHeight(), clipPlanes.x, clipPlanes.y);
	glLoadMatrixf((GLfloat*)ProjectionMatrix.ptr());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ModuleRenderer3D::SetFOV(float fov)
{
	this->fov = fov;

	CalculateProjectionMatrix();
}

void ModuleRenderer3D::SetClipPlanes(math::float2 clipPlanes)
{
	this->clipPlanes = clipPlanes;

	CalculateProjectionMatrix();
}

bool ModuleRenderer3D::SetVSync(bool vsync) 
{
	bool ret = true;

	this->vsync = vsync;

	if (this->vsync) {

		if (SDL_GL_SetSwapInterval(1) == -1)
		{
			ret = false;
			CONSOLE_LOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}
	}
	else {
	
		if (SDL_GL_SetSwapInterval(0) == -1) 
		{
			ret = false;
			CONSOLE_LOG("Warning: Unable to set immediate updates! SDL Error: %s\n", SDL_GetError());
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

math::float4x4 ModuleRenderer3D::Perspective(float fovy, float aspect, float n, float f) const
{
	math::float4x4 Perspective;

	float coty = 1.0f / tan(fovy * (float)M_PI / 360.0f);

	Perspective[0][0] = coty / aspect;
	Perspective[0][1] = 0.0f;
	Perspective[0][2] = 0.0f;
	Perspective[0][3] = 0.0f;
	Perspective[1][0] = 0.0f;
	Perspective[1][1] = coty;
	Perspective[1][2] = 0.0f;
	Perspective[1][3] = 0.0f;
	Perspective[2][0] = 0.0f;
	Perspective[2][1] = 0.0f;
	Perspective[2][2] = (n + f) / (n - f);
	Perspective[2][3] = -1.0f;
	Perspective[3][0] = 0.0f;
	Perspective[3][1] = 0.0f;
	Perspective[3][2] = 2.0f * n * f / (n - f);
	Perspective[3][3] = 0.0f;

	return Perspective;
}

bool ModuleRenderer3D::AddMesh(Mesh* mesh) 
{
	bool ret = false;

	if (std::find(meshes.begin(), meshes.end(), mesh) == meshes.end())
	{
		meshes.push_back(mesh);
		ret = true;
	}

	return ret;
}

bool ModuleRenderer3D::RemoveMesh(Mesh* mesh) 
{
	bool ret = false;

	std::vector<Mesh*>::const_iterator it = std::find(meshes.begin(), meshes.end(), mesh);
	if (it != meshes.end())
	{
		delete mesh;
		meshes.erase(it);
		ret = true;
	}

	return ret;
}

Mesh* ModuleRenderer3D::GetMeshAt(uint index) const
{
	if (index < meshes.size() && index >= 0)
		return meshes.at(index);

	return nullptr;
}

uint ModuleRenderer3D::GetNumMeshes() const
{
	return meshes.size();
}

void ModuleRenderer3D::ClearMeshes() 
{
	for (uint i = 0; i < meshes.size(); ++i)
		delete meshes[i];

	meshes.clear();

	// Clear geometry Bounding Box
	RELEASE(geometryBoundingBoxDebug);
}

void ModuleRenderer3D::DrawMesh(Mesh* mesh) const
{
	// Enable Multitexturing
	/// Texture 0
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->textureCoordsID);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);

	/// Texture 1
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mesh->texture2ID);

	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); /// params: GL_ADD, GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE, or GL_COMBINE

	glBindBuffer(GL_ARRAY_BUFFER, mesh->textureCoordsID);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);

	// -----

	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->verticesID);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesID);
	glDrawElements(GL_TRIANGLES, mesh->indicesSize, GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);

	// -----

	// Disable Multitexturing
	/// Texture 1
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	/// Texture 0
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ModuleRenderer3D::DrawMeshVerticesNormals(Mesh* mesh) const
{
	for (uint i = 0; i < mesh->verticesSize; ++i)
		mesh->normalsVerticesDebug[i]->Render();
}

void ModuleRenderer3D::DrawMeshFacesNormals(Mesh* mesh) const
{
	for (uint i = 0; i < mesh->indicesSize / 3; ++i)
		mesh->normalsFacesDebug[i]->Render();
}

void ModuleRenderer3D::AddTextureToMeshes(uint textureID, uint width, uint height)
{
	for (uint i = 0; i < meshes.size(); ++i)
	{
		meshes[i]->textureID = textureID;
		meshes[i]->textureWidth = width;
		meshes[i]->textureHeight = height;

		meshes[i]->checkTexture = false;
		meshes[i]->lastTextureID = 0;
	}
}

void ModuleRenderer3D::AddTexture2ToMeshes(uint textureID)
{
	for (uint i = 0; i < meshes.size(); ++i)
	{
		meshes[i]->texture2ID = textureID;
	}
}

void ModuleRenderer3D::SetCheckTextureToMeshes(bool checkTexture)
{
	uint checkTextureID = App->tex->GetCheckTextureID();

	for (uint i = 0; i < meshes.size(); ++i)
	{
		if (checkTexture)
		{
			meshes[i]->lastTextureID = meshes[i]->textureID;
			meshes[i]->textureID = checkTextureID;
			meshes[i]->checkTexture = true;
		}
		else
		{
			meshes[i]->textureID = meshes[i]->lastTextureID;
			meshes[i]->checkTexture = false;
			meshes[i]->lastTextureID = 0;
		}
	}
}

void ModuleRenderer3D::ClearTextures()
{
	for (uint i = 0; i < meshes.size(); ++i)
	{
		if (i == 0)
		{
			if (meshes[i]->checkTexture)
				SetCheckTextureToMeshes(false);

			if (meshes[i]->textureID != App->tex->GetCheckTextureID())
				glDeleteTextures(1, (GLuint*)&meshes[i]->textureID);
		}

		meshes[i]->textureID = 0;
		meshes[i]->textureWidth = 0;
		meshes[i]->textureHeight = 0;
	}
}

void ModuleRenderer3D::SetMultitexturing(bool multitexturing)
{
	this->multitexturing = multitexturing;
}

bool ModuleRenderer3D::GetMultitexturing() const
{
	return multitexturing;
}

void ModuleRenderer3D::SetGeometryName(const char* geometryName)
{
	RELEASE_ARRAY(this->geometryName);

	this->geometryName = geometryName;
}

const char* ModuleRenderer3D::GetGeometryName() const
{
	return geometryName;
}

void ModuleRenderer3D::SetGeometryActive(bool geometryActive)
{
	this->geometryActive = geometryActive;
}

bool ModuleRenderer3D::IsGeometryActive() const
{
	return geometryActive;
}

void ModuleRenderer3D::CreateGeometryBoundingBox()
{
	uint geometryVerticesSize = 0;
	for (uint i = 0; i < meshes.size(); ++i)
	{
		Mesh* mesh = GetMeshAt(i);
		geometryVerticesSize += mesh->verticesSize;
	}

	float* geometryVertices = new float[geometryVerticesSize * 3];

	int index = -1;
	for (uint i = 0; i < meshes.size(); ++i)
	{
		Mesh* mesh = GetMeshAt(i);

		for (uint j = 0; j < mesh->verticesSize * 3; ++j)
			geometryVertices[++index] = mesh->vertices[j];
	}

	geometryBoundingBox.SetNegativeInfinity();
	geometryBoundingBox.Enclose((const math::float3*)geometryVertices, geometryVerticesSize);

	// Debug draw
	geometryBoundingBoxDebug = new PrimitiveCube(geometryBoundingBox.Size(), geometryBoundingBox.CenterPoint());
	geometryBoundingBoxDebug->SetColor(Yellow);
	geometryBoundingBoxDebug->SetWireframeMode(true);
}

void ModuleRenderer3D::LookAtGeometry() const
{
	math::float3 target = geometryBoundingBox.CenterPoint(); // geometry center point
	float targetRadius = geometryBoundingBox.Size().Length(); // geometry diameter

	App->camera->MoveTo(math::float3(targetRadius, targetRadius, targetRadius));

	App->camera->SetTarget(target);
	App->camera->SetTargetRadius(targetRadius);
	App->camera->LookAt(target, targetRadius);
}

// Mesh --------------------------------------------------
void Mesh::Init()
{
	// Generate vertices buffer
	glGenBuffers(1, (GLuint*)&verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize * 3, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generate indices buffer
	glGenBuffers(1, (GLuint*)&indicesID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Generate Texture Coords
	glGenBuffers(1, (GLuint*)&textureCoordsID);
	glBindBuffer(GL_ARRAY_BUFFER, textureCoordsID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize * 2, textureCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create vertices normals
	normalsVerticesDebug = new PrimitiveRay*[verticesSize];

	uint index = 0;
	for (uint i = 0; i < verticesSize; ++i)
	{
		PrimitiveRay* ray = new PrimitiveRay(math::float3(normals[index], normals[index + 1], normals[index + 2]), 1.0f);
		ray->SetPosition(math::float3(vertices[index], vertices[index + 1], vertices[index + 2]));
		ray->SetColor(Green);
		index += 3;

		normalsVerticesDebug[i] = ray;
	}

	// Create faces normals
	normalsFacesDebug = new PrimitiveRay*[indicesSize / 3];

	index = 0;
	for (uint i = 0; i < indicesSize / 3; ++i)
	{
		math::float3 v1 = math::float3(vertices[indices[index] * 3], vertices[indices[index] * 3 + 1], vertices[indices[index] * 3 + 2]);
		++index;
		math::float3 v2 = math::float3(vertices[indices[index] * 3], vertices[indices[index] * 3 + 1], vertices[indices[index] * 3 + 2]);
		++index;
		math::float3 v3 = math::float3(vertices[indices[index] * 3], vertices[indices[index] * 3 + 1], vertices[indices[index] * 3 + 2]);
		++index;

		math::float3 a = v1 - v2;
		math::float3 b = v2 - v3;
		math::float3 vertexNormalDirection = math::Cross(a, b).Normalized();

		math::float3 vertexNormalPosition = (v1 + v2 + v3) / 3.0f;

		PrimitiveRay* ray = new PrimitiveRay(vertexNormalDirection, 1.0f);
		ray->SetPosition(vertexNormalPosition);
		ray->SetColor(Green);

		normalsFacesDebug[i] = ray;
	}

	// Create Bounding Box
	boundingBox.SetNegativeInfinity();
	boundingBox.Enclose((const math::float3*)vertices, verticesSize);

	// Debug draw Bounding Box
	boundingBoxDebug = new PrimitiveCube(boundingBox.Size(), boundingBox.CenterPoint());
	boundingBoxDebug->SetColor(Green);
	boundingBoxDebug->SetWireframeMode(true);
}

void Mesh::EmbedTexture(uint textureID)
{
	this->textureID = textureID;
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, (GLuint*)&verticesID);
	glDeleteBuffers(1, (GLuint*)&indicesID);
	glDeleteBuffers(1, (GLuint*)&textureCoordsID);

	RELEASE_ARRAY(name);
	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(normals);
	RELEASE_ARRAY(textureCoords);

	RELEASE(boundingBoxDebug);

	for (uint i = 0; i < verticesSize; ++i)
	{
		RELEASE(normalsVerticesDebug[i]);
	}

	RELEASE_ARRAY(normalsVerticesDebug);

	for (uint i = 0; i < indicesSize / 3; ++i)
	{
		RELEASE(normalsFacesDebug[i]);
	}

	RELEASE_ARRAY(normalsFacesDebug);
}