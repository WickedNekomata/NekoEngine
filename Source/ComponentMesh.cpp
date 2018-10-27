#include "ComponentMesh.h"
#include "GameObject.h"
#include "SceneImporter.h"
#include "Primitive.h"
#include "ComponentTransform.h"

#include "imgui/imgui.h"

ComponentMesh::ComponentMesh(GameObject* parent) : Component(parent, ComponentType::Mesh_Component)
{
	mesh = new Mesh();
}

ComponentMesh::~ComponentMesh()
{
	RELEASE(mesh);
	RELEASE(debugBoundingBox);
}

void ComponentMesh::Update()
{
}

void ComponentMesh::OnUniqueEditor()
{
	ImGui::Text("Mesh"); ImGui::SameLine();

	// https://github.com/ocornut/imgui/issues/1566
}

void ComponentMesh::GrowBoundingBox() const
{
	math::float3* points = new math::float3[mesh->verticesSize];

	uint index = 0;
	for (uint i = 0; i < mesh->verticesSize * 3; i += 3)
	{
		points[index].x = mesh->vertices[i];
		points[index].y = mesh->vertices[i + 1];
		points[index].z = mesh->vertices[i + 2];

		++index;
	}

	parent->boundingBox.Enclose((const math::float3*)points, mesh->verticesSize);
}

void ComponentMesh::RecalculateDebugBoundingBox()
{
	RELEASE(debugBoundingBox);
	math::float3 size = parent->boundingBox.Size();
	CONSOLE_LOG("%f, %f, %f", size.x, size.y, size.z);
	debugBoundingBox = new PrimitiveCube(parent->boundingBox.Size(), parent->boundingBox.CenterPoint());
	debugBoundingBox->SetColor(Yellow);
	debugBoundingBox->SetWireframeMode(true);
}