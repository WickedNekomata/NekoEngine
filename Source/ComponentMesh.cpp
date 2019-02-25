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

ComponentMesh::ComponentMesh(const ComponentMesh& componentMesh) : Component(componentMesh.parent, ComponentTypes::MeshComponent)
{
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
	if (res != 0)
	{
		App->res->SetAsUnused(res);
		App->res->SetAsUnused(deformableMesh->GetUuid());
		App->res->DeleteResource(deformableMesh->GetUuid());
		deformableMesh = 0;
	}

	if (res_uuid != 0) 
	{
		if (App->res->SetAsUsed(res_uuid) == -1)
			return;

		ResourceMesh* currentRes = (ResourceMesh*)App->res->GetResource(res_uuid);
		ResourceData data;
		ResourceMeshData specificData;
		specificData.indicesSize = currentRes->GetIndicesCount();
		specificData.indices = new uint[specificData.indicesSize];
		currentRes->GetIndices(specificData.indices);
		specificData.verticesSize = currentRes->GetVerticesCount();
		specificData.vertices = new Vertex[specificData.verticesSize];

		Vertex* vertices;
		currentRes->GetVerticesReference(vertices);
		memcpy(specificData.vertices, vertices, sizeof(Vertex) * specificData.verticesSize);

		data.name = "Deformable Mesh :)";
		deformableMesh = (ResourceMesh*)App->res->CreateResource(ResourceTypes::MeshResource, data, &specificData);
		App->res->SetAsUsed(deformableMesh->GetUuid());
	}

	res = res_uuid;
	
	// Mesh updated: recalculate bounding boxes
	System_Event newEvent;
	newEvent.goEvent.gameObject = parent;
	newEvent.type = System_Event_Type::RecalculateBBoxes;
	App->PushSystemEvent(newEvent);

	if (parent->IsStatic())
	{
		// Bounding box changed: recreate quadtree
		System_Event newEvent;
		newEvent.type = System_Event_Type::RecreateQuadtree;
		App->PushSystemEvent(newEvent);
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
	return sizeof(uint) + sizeof(bool);
}

void ComponentMesh::OnInternalSave(char*& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &res, bytes);
	cursor += bytes;

	bytes = sizeof(bool);
	memcpy(cursor, &nv_walkable, bytes);
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

	ResourceMesh* currentRes = (ResourceMesh*)App->res->GetResource(loadedRes);
	ResourceData data;
	ResourceMeshData specificData;
	specificData.indicesSize = currentRes->GetIndicesCount();
	specificData.indices = new uint[specificData.indicesSize];
	currentRes->GetIndices(specificData.indices);
	specificData.verticesSize = currentRes->GetVerticesCount();
	specificData.vertices = new Vertex[specificData.verticesSize];

	Vertex* vertices;
	currentRes->GetVerticesReference(vertices);
	memcpy(specificData.vertices, vertices, sizeof(Vertex) * specificData.verticesSize);

	data.name = "Deformable Mesh :)";
	deformableMesh = (ResourceMesh*)App->res->CreateResource(ResourceTypes::MeshResource, data, &specificData);
	App->res->SetAsUsed(deformableMesh->GetUuid());
}