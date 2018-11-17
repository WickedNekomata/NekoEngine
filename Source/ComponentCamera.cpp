#include "ComponentCamera.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleWindow.h"
#include "Application.h"

#include "imgui\imgui.h"

ComponentCamera::ComponentCamera(GameObject* parent) : Component(parent, ComponentType::Camera_Component)
{
	frustum.type = math::FrustumType::PerspectiveFrustum;

	frustum.pos = math::float3::zero;
	frustum.front = math::float3::unitZ;
	frustum.up = math::float3::unitY;

	frustum.nearPlaneDistance = 0.1f;
	frustum.farPlaneDistance = 500.0f;
	frustum.verticalFov = 60.0f * DEGTORAD;
	frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov / 2.0f) * 1.3f);
}

ComponentCamera::ComponentCamera(const ComponentCamera& componentCamera) : Component(componentCamera.parent, ComponentType::Camera_Component)
{
	frustum = componentCamera.frustum;

	frustumCulling = componentCamera.frustumCulling;
	mainCamera = componentCamera.mainCamera;
}

ComponentCamera::~ComponentCamera() 
{
	if (parent != nullptr) // Editor camera's parent is nullptr
		parent->camera = nullptr;
}

void ComponentCamera::UpdateTransform()
{
	math::float4x4 matrix = parent->transform->GetGlobalMatrix();
	frustum.pos = matrix.TranslatePart();
	frustum.front = matrix.WorldZ();
	frustum.up = matrix.WorldY();
}

void ComponentCamera::OnUniqueEditor()
{
#ifndef GAMEMODE
	ImGui::Text("Camera");
	ImGui::Spacing();

	ImGui::Checkbox("Main Camera", &mainCamera);

	ImGui::Text("Field of View");
	ImGui::SameLine();

	float fov = frustum.verticalFov * RADTODEG;
	if (ImGui::SliderFloat("##fov", &fov, 1.0f, 179.99f))
		SetFOV(fov);

	if (ImGui::Checkbox("Frustum Culling", &frustumCulling));

	if (frustumCulling)
	{
		ImGui::Text("Clipping Planes");

		ImGui::Text("Near");
		ImGui::SameLine();
		ImGui::InputFloat("##nearPlane", &frustum.nearPlaneDistance);

		ImGui::Text("Far ");
		ImGui::SameLine();
		ImGui::InputFloat("##farPlane", &frustum.farPlaneDistance);
	}
#endif
}

void ComponentCamera::SetFOV(float fov)
{
	frustum.verticalFov = fov * DEGTORAD;
	frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov / 2.0f) * App->window->GetWindowWidth() / App->window->GetWindowHeight());
}

void ComponentCamera::SetNearPlaneDistance(float nearPlane)
{
	frustum.nearPlaneDistance = nearPlane;
}

void ComponentCamera::SetFarPlaneDistance(float farPlane)
{
	frustum.farPlaneDistance = farPlane;
}

void ComponentCamera::SetAspectRatio(float aspectRatio)
{
	frustum.horizontalFov = 2.f * atanf(tanf(frustum.verticalFov * 0.5f) * aspectRatio);
}

math::float4x4& ComponentCamera::GetOpenGLViewMatrix() const
{
	math::float4x4 matrix = frustum.ViewMatrix();
	return matrix.Transposed();
}

math::float4x4& ComponentCamera::GetOpenGLProjectionMatrix() const
{
	math::float4x4 matrix = frustum.ProjectionMatrix();
	return matrix.Transposed();
}

void ComponentCamera::SetFrustumCulling(bool frustumCulling)
{
	this->frustumCulling = frustumCulling;
}

bool ComponentCamera::HasFrustumCulling() const
{
	return frustumCulling;
}

void ComponentCamera::SetMainCamera(bool mainCamera)
{
	this->mainCamera = mainCamera;
}

bool ComponentCamera::IsMainCamera() const
{
	return mainCamera;
}

void ComponentCamera::OnInternalSave(JSON_Object* file)
{
	json_object_set_number(file, "nearPlaneDistance", frustum.nearPlaneDistance);
	json_object_set_number(file, "farPlaneDistance", frustum.farPlaneDistance);
	json_object_set_number(file, "verticalFov", frustum.verticalFov);
	json_object_set_number(file, "horizontalFov", frustum.horizontalFov);
	json_object_set_boolean(file, "isMainCamera", mainCamera);
	json_object_set_boolean(file, "isFrustumCulling", frustumCulling);
}

void ComponentCamera::OnLoad(JSON_Object* file)
{
	frustum.nearPlaneDistance = json_object_get_number(file, "nearPlaneDistance");
	frustum.farPlaneDistance = json_object_get_number(file, "farPlaneDistance");
	frustum.verticalFov = json_object_get_number(file, "verticalFov");
	frustum.horizontalFov = json_object_get_number(file, "horizontalFov");

	mainCamera = json_object_get_boolean(file, "isMainCamera");
	frustumCulling = json_object_get_boolean(file, "isFrustumCulling");
}