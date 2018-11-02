#include "ComponentCamera.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleWindow.h"
#include "Application.h"

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

ComponentCamera::~ComponentCamera() 
{
	if (parent != nullptr) // Editor camera's parent is nullptr
		parent->camera = nullptr;
}

void ComponentCamera::UpdateTransform()
{
	math::float4x4 matrix = parent->transform->GetGlobalMatrix();
	cameraFrustum.pos = matrix.TranslatePart();
	cameraFrustum.front = matrix.WorldZ();
	cameraFrustum.up = matrix.WorldY();
}

void ComponentCamera::OnUniqueEditor()
{
	ImGui::Checkbox("Main Camera", &mainCamera);

	ImGui::Text("Field of View");
	ImGui::SameLine();

	float fov = cameraFrustum.verticalFov * RADTODEG;
	if (ImGui::SliderFloat("##fov", &fov, 1.0f, 179.99f))
		SetFOV(fov);

	if (ImGui::Checkbox("Frustum Culling", &frustumCulling))
	{
		if (this == App->renderer3D->GetMainCamera())
			App->renderer3D->SetMeshComponentsSeenLastFrame(!frustumCulling);
	}

	if (frustumCulling)
	{
		ImGui::Text("Clipping Planes");

		ImGui::Text("Near");
		ImGui::SameLine();
		ImGui::InputFloat("##nearPlane", &cameraFrustum.nearPlaneDistance);

		ImGui::Text("Far ");
		ImGui::SameLine();
		ImGui::InputFloat("##farPlane", &cameraFrustum.farPlaneDistance);
	}
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
	json_object_set_number(file, "nearPlaneDistance", cameraFrustum.nearPlaneDistance);
	json_object_set_number(file, "farPlaneDistance", cameraFrustum.farPlaneDistance);
	json_object_set_number(file, "verticalFov", cameraFrustum.verticalFov);
	json_object_set_number(file, "horizontalFov", cameraFrustum.horizontalFov);
}

void ComponentCamera::OnLoad(JSON_Object* file)
{
	cameraFrustum.nearPlaneDistance = json_object_get_number(file, "nearPlaneDistance");
	cameraFrustum.farPlaneDistance = json_object_get_number(file, "farPlaneDistance");
	cameraFrustum.verticalFov = json_object_get_number(file, "verticalFov");
	cameraFrustum.horizontalFov = json_object_get_number(file, "horizontalFov");
}

void ComponentCamera::OnGameMode()
{

}

void ComponentCamera::OnEditorMode()
{

}