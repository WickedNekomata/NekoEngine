#include "ComponentCamera.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleWindow.h"
#include "Application.h"
#include "ModuleRenderer3D.h"

#include "imgui\imgui.h"

#include "MathGeoLib/include/Geometry/LineSegment.h"

ComponentCamera::ComponentCamera(GameObject* parent, bool dummy) : Component(parent, ComponentTypes::CameraComponent)
{
	if (!dummy)
		App->renderer3D->AddCameraComponent(this);

	frustum.type = math::FrustumType::PerspectiveFrustum;

	frustum.pos = math::float3::zero;
	frustum.front = math::float3::unitZ;
	frustum.up = math::float3::unitY;

	frustum.nearPlaneDistance = 1.0f;
	frustum.farPlaneDistance = 500.0f;
	frustum.verticalFov = 60.0f * DEGTORAD;
	frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov / 2.0f) * 1.3f);
}

ComponentCamera::ComponentCamera(const ComponentCamera& componentCamera) : Component(componentCamera.parent, ComponentTypes::CameraComponent)
{
	App->renderer3D->AddCameraComponent(this);

	frustum = componentCamera.frustum;

	frustumCulling = componentCamera.frustumCulling;
	mainCamera = componentCamera.mainCamera;
}

ComponentCamera::~ComponentCamera() 
{
	App->renderer3D->EraseCameraComponent(this);

	if (parent != nullptr) // Editor camera's parent is nullptr
		parent->cmp_camera = nullptr;
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
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
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
	}
#endif
}

void ComponentCamera::SetFOV(float fov)
{
	frustum.verticalFov = fov * DEGTORAD;
	frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov / 2.0f) * App->window->GetWindowWidth() / App->window->GetWindowHeight());
}

float ComponentCamera::GetFOV() const
{
	return frustum.verticalFov * RADTODEG;
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

math::float4x4 ComponentCamera::GetOpenGLViewMatrix() const
{
	math::float4x4 matrix = frustum.ViewMatrix();
	return matrix.Transposed();
}

math::float4x4 ComponentCamera::GetOpenGLProjectionMatrix() const
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

uint ComponentCamera::GetInternalSerializationBytes()
{
	return sizeof(bool) * 2 + sizeof(float) * 4;
}

void ComponentCamera::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(float);
	memcpy(cursor, &frustum.nearPlaneDistance, bytes);
	cursor += bytes;

	memcpy(cursor, &frustum.farPlaneDistance, bytes);
	cursor += bytes;

	memcpy(cursor, &frustum.verticalFov, bytes);
	cursor += bytes;

	memcpy(cursor, &frustum.horizontalFov, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &mainCamera, bytes);
	cursor += bytes;

	memcpy(cursor, &frustumCulling, bytes);
	cursor += bytes;
}

void ComponentCamera::OnInternalLoad(char*& cursor)
{
	size_t bytes = sizeof(float);
	memcpy(&frustum.nearPlaneDistance, cursor, bytes);
	cursor += bytes;

	memcpy(&frustum.farPlaneDistance, cursor, bytes);
	cursor += bytes;

	memcpy(&frustum.verticalFov, cursor, bytes);
	cursor += bytes;

	memcpy(&frustum.horizontalFov, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(&mainCamera, cursor, bytes);
	cursor += bytes;

	memcpy(&frustumCulling, cursor, bytes);
	cursor += bytes;
}

math::Ray ComponentCamera::ScreenToRay(math::float2 screenPoint)
{
	float normalizedX = -(1.0f - (screenPoint.x * 2.0f) / App->window->GetWindowWidth());
	float normalizedY = 1.0f - (screenPoint.y * 2.0f) / App->window->GetWindowHeight();
	return frustum.UnProjectLineSegment(normalizedX, normalizedY).ToRay();
}
