#include "ComponentProjector.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ModuleInternalResHandler.h"
#include "ModuleLayers.h"

#include "Resource.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui\imgui.h"

#include <assert.h>

ComponentProjector::ComponentProjector(GameObject* parent) : Component(parent, ComponentTypes::ProjectorComponent)
{
	SetMaterialRes(App->resHandler->defaultMaterial);

	// Init frustum
	frustum.type = math::FrustumType::PerspectiveFrustum;

	frustum.pos = math::float3::zero;
	frustum.front = math::float3::unitZ;
	frustum.up = math::float3::unitY;

	frustum.nearPlaneDistance = 1.0f;
	frustum.farPlaneDistance = 500.0f;
	frustum.verticalFov = 60.0f * DEGTORAD;
	frustum.horizontalFov = 60.0f * DEGTORAD;

	// -----

	App->renderer3D->AddProjectorComponent(this);
}

ComponentProjector::ComponentProjector(const ComponentProjector& componentProjector) : Component(componentProjector.parent, ComponentTypes::ProjectorComponent)
{
	// TODO
}

ComponentProjector::~ComponentProjector()
{
	App->renderer3D->EraseProjectorComponent(this);
	parent->cmp_projector = nullptr;
}

void ComponentProjector::UpdateTransform()
{
	math::float4x4 matrix = parent->transform->GetGlobalMatrix();
	frustum.pos = matrix.TranslatePart();
	frustum.front = matrix.WorldZ();
	frustum.up = matrix.WorldY();
}

void ComponentProjector::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Projector", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Near clip plane"); ImGui::SameLine(); ImGui::AlignTextToFramePadding();
		ImGui::InputFloat("##nearClipPlane", &frustum.nearPlaneDistance);

		ImGui::Text("Far clip plane "); ImGui::SameLine(); ImGui::AlignTextToFramePadding();
		ImGui::InputFloat("##farClipPlane", &frustum.farPlaneDistance);

		ImGui::Text("Field of view"); ImGui::SameLine(); ImGui::AlignTextToFramePadding();
		float fov = frustum.verticalFov * RADTODEG;
		if (ImGui::SliderFloat("##fov", &fov, 0.0f, 180.0f))
			SetFOV(fov);

		// Material
		const Resource* resource = App->res->GetResource(materialRes);
		std::string materialName = resource->GetName();

		ImGui::PushID("material");
		ImGui::Button(materialName.data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", materialRes);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetMaterialRes(payload_n);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::SmallButton("Use default material"))
			SetMaterialRes(App->resHandler->defaultMaterial);

		// Ignore layers
		std::string title;
		std::vector<Layer*> activeLayers;
		uint enabledLayers = 0;

		for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
		{
			Layer* layer = App->layers->GetLayer(i);
			const char* layerName = layer->name.data();
			if (strcmp(layerName, "") == 0)
				continue;

			if (filterMask & layer->GetFilterGroup())
			{
				title.append(layer->name);
				title.append(", ");

				++enabledLayers;
			}

			activeLayers.push_back(layer);
		}

		if (enabledLayers == 0)
			title = "Nothing";
		else if (enabledLayers == activeLayers.size())
			title = "Everything";
		else
		{
			uint found = title.find_last_of(",");
			if (found != std::string::npos)
				title = title.substr(0, found);
		}

		ImGui::PushItemWidth(150.0f);
		if (ImGui::BeginCombo("Ignore layers", title.data()))
		{
			if (ImGui::Selectable("Nothing", enabledLayers == 0 ? true : false))
				filterMask = 0;

			if (ImGui::Selectable("Everything", enabledLayers == activeLayers.size() ? true : false))
			{
				for (uint i = 0; i < activeLayers.size(); ++i)
					filterMask |= activeLayers[i]->GetFilterGroup();
			}

			for (uint i = 0; i < activeLayers.size(); ++i)
			{
				Layer* layer = activeLayers[i];
				if (ImGui::Selectable(layer->name.data(), filterMask & layer->GetFilterGroup() ? true : false))
					filterMask ^= layer->GetFilterGroup();
			}
			ImGui::EndCombo();
		}
	}
#endif
}

uint ComponentProjector::GetInternalSerializationBytes()
{
	// TODO
	return uint();
}

void ComponentProjector::OnInternalSave(char*& cursor)
{
	// TODO
}

void ComponentProjector::OnInternalLoad(char*& cursor)
{
	// TODO
}

// ----------------------------------------------------------------------------------------------------

void ComponentProjector::SetFOV(float fov)
{
	frustum.verticalFov = fov * DEGTORAD;
	frustum.horizontalFov = fov * DEGTORAD;
}

float ComponentProjector::GetFOV() const
{
	return frustum.verticalFov * RADTODEG;
}

void ComponentProjector::SetNearPlaneDistance(float nearPlane)
{
	frustum.nearPlaneDistance = nearPlane;
}

void ComponentProjector::SetFarPlaneDistance(float farPlane)
{
	frustum.farPlaneDistance = farPlane;
}

void ComponentProjector::SetAspectRatio(float aspectRatio)
{
	frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov * 0.5f) * aspectRatio);
}

void ComponentProjector::SetMaterialRes(uint materialUuid)
{
	if (materialRes > 0)
		App->res->SetAsUnused(materialRes);

	if (materialUuid > 0)
		App->res->SetAsUsed(materialUuid);

	materialRes = materialUuid;
}

uint ComponentProjector::GetMaterialRes() const
{
	return materialRes;
}

void ComponentProjector::SetFilterMask(uint filterMask)
{
	this->filterMask = filterMask;
}

math::Frustum ComponentProjector::GetFrustum() const
{
	return frustum;
}

math::float4x4 ComponentProjector::GetOpenGLViewMatrix() const
{
	math::float4x4 matrix = frustum.ViewMatrix();
	return matrix.Transposed();
}

math::float4x4 ComponentProjector::GetOpenGLProjectionMatrix() const
{
	math::float4x4 matrix = frustum.ProjectionMatrix();
	return matrix.Transposed();
}