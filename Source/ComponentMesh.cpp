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
}

void ComponentMesh::Update()
{
}

void ComponentMesh::OnUniqueEditor()
{
	ImGui::Text("Mesh");

	// https://github.com/ocornut/imgui/issues/1566

	/*
	if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen))
	{
		uint numMeshes = App->renderer3D->GetNumMeshes();
		ImGui::Text("Meshes: %i", numMeshes);

		for (int i = 0; i < numMeshes; ++i)
		{
			ImGui::Separator();
			Mesh* mesh = App->renderer3D->GetMeshAt(i);
			ImGui::TextColored(WHITE, "Mesh %i: %s", i + 1, mesh->name);
			ImGui::Separator();

			ImGui::Text("Vertices: %i", mesh->verticesSize);
			ImGui::Text("Vertices ID: %i", mesh->verticesID);

			ImGui::Text("Indices: %i", mesh->indicesSize);
			ImGui::Text("Indices ID: %i", mesh->indicesID);

			ImGui::Text("Texture Coords: %i", mesh->verticesSize);
			ImGui::Text("Texture Coords ID: %i", mesh->textureCoordsID);

			ImGui::Text("Triangles: %i", mesh->indicesSize / 3);
		}
	}
	*/
}

void ComponentMesh::OnInternalSave(JSON_Object* file)
{
	json_object_set_number(file, "ResourceMesh", 0012013);
}

void ComponentMesh::OnLoad(JSON_Object* file)
{
	// LOAD MESH
}