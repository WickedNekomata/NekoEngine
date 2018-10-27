#include "ComponentCamera.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui/imgui.h"

ComponentCamera::ComponentCamera(GameObject* parent) : Component(parent, ComponentType::Camera_Component)
{
	cameraFrustum.type = math::FrustumType::PerspectiveFrustum;

	cameraFrustum.pos = math::float3::zero;
	cameraFrustum.front = math::float3::unitZ;
	cameraFrustum.up = math::float3::unitY;

	cameraFrustum.nearPlaneDistance = 0.1f;
	cameraFrustum.farPlaneDistance = 1000.0f;
	cameraFrustum.verticalFov = DEGTORAD * 60.0f;
	cameraFrustum.horizontalFov = 2.f * atanf(tanf(cameraFrustum.verticalFov * 0.5f) * 1.3f);	
}

ComponentCamera::~ComponentCamera()
{
}

void ComponentCamera::Update()
{
	math::float4x4 matrix = parent->transform->GetGlobalMatrix();
	cameraFrustum.pos = matrix.TranslatePart();
	cameraFrustum.front = matrix.WorldZ();
	cameraFrustum.up = matrix.WorldY();
}

void ComponentCamera::OnUniqueEditor()
{
	ImGui::Text("This is a camera component");
}
