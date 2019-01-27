#include "PanelNavMesh.h"

#include "imgui/imgui.h"

#include "Application.h"
#include "SoloMesh_Query.h"
#include "InputGeom.h"
#include "ModuleScene.h"

#include "ComponentMesh.h"
#include "ModuleResourceManager.h"

PanelNavMesh::PanelNavMesh(char* name) : Panel(name)
{
}

PanelNavMesh::~PanelNavMesh()
{
}

bool PanelNavMesh::Draw()
{
	ImGui::Begin(name, &enabled);

	if (ImGui::Button("Add Mesh"))
	{
		if (App->scene->selectedObject == CurrentSelection::SelectedType::gameObject)
		{
			GameObject* current = (GameObject*)App->scene->selectedObject.Get();
			InputGeom inputGeom;
			inputGeom.m_mesh = (ResourceMesh*)App->res->GetResource(current->meshRenderer->res);
			memcpy(inputGeom.bMin, current->boundingBox.minPoint.ptr(), sizeof(math::float3));
			memcpy(inputGeom.bMax, current->boundingBox.maxPoint.ptr(), sizeof(math::float3));
			App->soloMeshQuery->SetInputGeom(inputGeom);
		}
	}

	if (ImGui::Button("Handle Build"))
		App->soloMeshQuery->HandleBuild();

	ImGui::End();

	return true;
}
