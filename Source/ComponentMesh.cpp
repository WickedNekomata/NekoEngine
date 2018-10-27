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
	ImGui::Text("Mesh");// ImGui::SameLine();

	// https://github.com/ocornut/imgui/issues/1566
}

void ComponentMesh::GrowBoundingBox() const
{
	parent->boundingBox.Enclose((const math::float3*)mesh->vertices, mesh->verticesSize);
}

void ComponentMesh::CreateDebugBoundingBox()
{
	debugBoundingBox = new PrimitiveCube(parent->boundingBox.Size());
	debugBoundingBox->SetColor(Yellow);
	debugBoundingBox->SetWireframeMode(true);
}