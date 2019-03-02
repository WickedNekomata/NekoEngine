#include "ComponentMesh.h"
#include "GameObject.h"
#include "SceneImporter.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "Resource.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "ResourceMesh.h"

#include "imgui\imgui.h"

ComponentMesh::ComponentMesh(GameObject* parent) : Component(parent, ComponentTypes::MeshComponent)
{
	App->renderer3D->AddMeshComponent(this);
}

ComponentMesh::ComponentMesh(const ComponentMesh& componentMesh, GameObject* parent, bool include) : Component(parent, ComponentTypes::MeshComponent)
{
	if (include)
		App->renderer3D->AddMeshComponent(this);
	SetResource(componentMesh.res);
}

ComponentMesh::~ComponentMesh()
{
	App->renderer3D->EraseMeshComponent(this);

	SetResource(0);
}

void ComponentMesh::Update() {}

void ComponentMesh::SetResource(uint res_uuid)
{
	if (res > 0)
		App->res->SetAsUnused(res);

	if (res_uuid > 0)
		App->res->SetAsUsed(res_uuid);

	Resource* resource = App->res->GetResource(res_uuid);

	res = resource ? res_uuid : 0;

	if (resource)
	{
		//Calculate the new mesh BoundingBox
		System_Event createBB;
		createBB.goEvent.gameObject = parent;
		createBB.type = System_Event_Type::CalculateBBoxes;
		App->PushSystemEvent(createBB);

		// Mesh updated: recalculate bounding boxes
		System_Event updateBB;
		updateBB.goEvent.gameObject = parent;
		updateBB.type = System_Event_Type::RecalculateBBoxes;
		App->PushSystemEvent(updateBB);

		if (parent->IsStatic())
		{
			// Bounding box changed: recreate quadtree
			System_Event newEvent;
			newEvent.type = System_Event_Type::RecreateQuadtree;
			App->PushSystemEvent(newEvent);
		}
	}
}

void ComponentMesh::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Mesh");
		ImGui::SameLine();

		std::string fileName = "Empty Mesh";
		const Resource* resource = App->res->GetResource(res);
		if (resource != nullptr)
			fileName = resource->GetName();

		ImGui::PushID("mesh");
		ImGui::Button(fileName.data(), ImVec2(150.0f, 0.0f));
		ImGui::PopID();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", res);
			ImGui::EndTooltip();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_INSPECTOR_SELECTOR"))
			{
				uint payload_n = *(uint*)payload->Data;
				SetResource(payload_n);
			}
			ImGui::EndDragDropTarget();
		}
	}
#endif
}

uint ComponentMesh::GetInternalSerializationBytes()
{
	return sizeof(uint) + sizeof(bool) + sizeof(uint) + sizeof(uint);
}

void ComponentMesh::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &nv_walkable, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &root_bones_uid, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &root_bone, bytes);
	cursor += bytes;
}

void ComponentMesh::OnInternalLoad(char*& cursor)
{
	uint loadedRes;
	size_t bytes = sizeof(uint);
	memcpy(&loadedRes, cursor, bytes);
	cursor += bytes;
	SetResource(loadedRes);

	bytes = sizeof(bool);
	memcpy(&nv_walkable, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(&root_bones_uid, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(&root_bone, cursor, bytes);
	cursor += bytes;
}