#include "ComponentCamera.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleWindow.h"
#include "Application.h"
#include "ModuleRenderer3D.h"

#include "imgui/imgui.h"

ComponentCamera::ComponentCamera(GameObject* parent) : Component(parent, ComponentType::Camera_Component)
{
	cameraFrustum.type = math::FrustumType::PerspectiveFrustum;

	cameraFrustum.pos = math::float3::zero;
	cameraFrustum.front = math::float3::unitZ;
	cameraFrustum.up = math::float3::unitY;

	cameraFrustum.nearPlaneDistance = 0.1f;
	cameraFrustum.farPlaneDistance = 1000.0f;
	cameraFrustum.verticalFov = 60.0f * DEGTORAD;
	cameraFrustum.horizontalFov = 2.0f * atanf(tanf(cameraFrustum.verticalFov / 2.0f) * 1.3f);	
}

ComponentCamera::~ComponentCamera() {}

void ComponentCamera::UpdateTransform()
{
	math::float4x4 matrix = parent->transform->GetGlobalMatrix();
	cameraFrustum.pos = matrix.TranslatePart();
	cameraFrustum.front = matrix.WorldZ();
	cameraFrustum.up = matrix.WorldY();

	// REMOVE THIS ASAP
#pragma region  DEBUGDRAWFRUSTUM

	math::float3 corners[8];
	cameraFrustum.GetCornerPoints(corners);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBegin(GL_QUADS);

	glVertex3fv((GLfloat*)&corners[1]); //glVertex3f(-sx, -sy, sz);
	glVertex3fv((GLfloat*)&corners[5]); //glVertex3f( sx, -sy, sz);
	glVertex3fv((GLfloat*)&corners[7]); //glVertex3f( sx,  sy, sz);
	glVertex3fv((GLfloat*)&corners[3]); //glVertex3f(-sx,  sy, sz);

	glVertex3fv((GLfloat*)&corners[4]); //glVertex3f( sx, -sy, -sz);
	glVertex3fv((GLfloat*)&corners[0]); //glVertex3f(-sx, -sy, -sz);
	glVertex3fv((GLfloat*)&corners[2]); //glVertex3f(-sx,  sy, -sz);
	glVertex3fv((GLfloat*)&corners[6]); //glVertex3f( sx,  sy, -sz);

	glVertex3fv((GLfloat*)&corners[5]); //glVertex3f(sx, -sy,  sz);
	glVertex3fv((GLfloat*)&corners[4]); //glVertex3f(sx, -sy, -sz);
	glVertex3fv((GLfloat*)&corners[6]); //glVertex3f(sx,  sy, -sz);
	glVertex3fv((GLfloat*)&corners[7]); //glVertex3f(sx,  sy,  sz);

	glVertex3fv((GLfloat*)&corners[0]); //glVertex3f(-sx, -sy, -sz);
	glVertex3fv((GLfloat*)&corners[1]); //glVertex3f(-sx, -sy,  sz);
	glVertex3fv((GLfloat*)&corners[3]); //glVertex3f(-sx,  sy,  sz);
	glVertex3fv((GLfloat*)&corners[2]); //glVertex3f(-sx,  sy, -sz);

	glVertex3fv((GLfloat*)&corners[3]); //glVertex3f(-sx, sy,  sz);
	glVertex3fv((GLfloat*)&corners[7]); //glVertex3f( sx, sy,  sz);
	glVertex3fv((GLfloat*)&corners[6]); //glVertex3f( sx, sy, -sz);
	glVertex3fv((GLfloat*)&corners[2]); //glVertex3f(-sx, sy, -sz);

	glVertex3fv((GLfloat*)&corners[0]); //glVertex3f(-sx, -sy, -sz);
	glVertex3fv((GLfloat*)&corners[4]); //glVertex3f( sx, -sy, -sz);
	glVertex3fv((GLfloat*)&corners[5]); //glVertex3f( sx, -sy,  sz);
	glVertex3fv((GLfloat*)&corners[1]); //glVertex3f(-sx, -sy,  sz);

	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#pragma endregion
}

void ComponentCamera::OnUniqueEditor()
{
	if (ImGui::Checkbox("Main Camera", &mainCamera))
		App->renderer3D->SetMainCamera(this, mainCamera);

	if (ImGui::Checkbox("Frustum Culling", &frustumCulling)) { SetFrustumCulling(frustumCulling); }

	ImGui::Text("Field of View");
	ImGui::SameLine();
	float fov = cameraFrustum.verticalFov * RADTODEG;
	ImGui::SliderFloat("##fov", &fov, 1.0f, 179.99f);
	SetFOV(fov);

	ImGui::Text("Clipping Planes");

	ImGui::Text("Near");
	ImGui::SameLine();
	
	ImGui::InputFloat("##naerPlane", &cameraFrustum.nearPlaneDistance);
	ImGui::Text("Far ");
	ImGui::SameLine();
	ImGui::InputFloat("##farPlane", &cameraFrustum.farPlaneDistance);
}

void ComponentCamera::SetFOV(float fov)
{
	cameraFrustum.verticalFov = fov * DEGTORAD;
	cameraFrustum.horizontalFov = 2.0f * atanf(tanf(cameraFrustum.verticalFov / 2.0f) * App->window->GetWindowWidth() / App->window->GetWindowHeight());
}

void ComponentCamera::SetNearPlaneDistance(float nearPlane)
{
	cameraFrustum.nearPlaneDistance = nearPlane;
}

void ComponentCamera::SetFarPlaneDistance(float farPlane)
{
	cameraFrustum.farPlaneDistance = farPlane;
}

float* ComponentCamera::GetOpenGLViewMatrix()
{
	return ((math::float4x4)cameraFrustum.ViewMatrix()).Transposed().ptr();
}

float* ComponentCamera::GetOpenGLProjectionMatrix()
{
	return cameraFrustum.ProjectionMatrix().Transposed().ptr();
}

void ComponentCamera::SetPlay(bool play)
{
	this->play = play;
	// TODO
}

bool ComponentCamera::IsPlay() const
{
	return play;
}

void ComponentCamera::SetFrustumCulling(bool frustumCulling)
{
	this->frustumCulling = frustumCulling;
}

bool ComponentCamera::GetFrustumCulling() const
{
	return frustumCulling;
}

void ComponentCamera::SetMainCamera(bool mainCamera)
{
	this->mainCamera = mainCamera;
}

bool ComponentCamera::GetMainCamera() const
{
	return mainCamera;
}