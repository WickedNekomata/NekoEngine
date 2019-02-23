#include "PanelNavigation.h"

#include "imgui/imgui.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "NMInputGeom.h"
#include "ModuleGOs.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"

#include "ResourceMesh.h"
#include "ModuleResourceManager.h"

#include "MathGeoLib/include/Math/float4.h"

PanelNavigation::PanelNavigation(char* name) : Panel(name)
{
	ResetCommonSettings();
}

PanelNavigation::~PanelNavigation()
{
}

bool PanelNavigation::Draw()
{
	ImGui::Begin(name, &enabled);

	if (App->scene->selectedObject == CurrentSelection::SelectedType::gameObject)
	{
		GameObject* curr = (GameObject*)App->scene->selectedObject.Get();

		if (curr->cmp_mesh) {
			ImGui::Text("Object");

			bool isStatic = curr->IsStatic();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Static");
			ImGui::SameLine();
			if (ImGui::Checkbox("##Static", &isStatic))
			{
				curr->ToggleIsStatic();
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Walkable");
			ImGui::SameLine();
			ImGui::Checkbox("##Walkability", &curr->cmp_mesh->nv_walkable);
		}
	}

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Build Settings");
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		ResetCommonSettings();

	if (ImGui::CollapsingHeader("Rasteritzation"))
	{
		ImGui::SliderFloat("##CellSize", &cs.p_cellSize, 0.10f, 1.0f, "Cell Size %.01f");
		ImGui::SliderFloat("##CellHeight", &cs.p_cellHeight, 0.10f, 1.0f, "Cell Height %.01f");
	}

	if (ImGui::CollapsingHeader("Agent"))
	{
		ImGui::SliderFloat("##A_Height", &cs.p_agentHeight, 0.0f, 5.0f, "Height %.1f");
		ImGui::SliderFloat("##A_Radius", &cs.p_agentRadius, 0.0f, 5.0f, "Radius %.1f");
		ImGui::SliderFloat("##A_MClimb", &cs.p_agentMaxClimb, 0.0f, 5.0f, "Max Climb %.1f");
		ImGui::SliderFloat("##A_MSlope", &cs.p_agentMaxSlope, 0.0f, 90.0f, "Max Slope %1.f");
	}

	if (ImGui::CollapsingHeader("Region"))
	{
		ImGui::SliderFloat("##MinRegion", &cs.p_regionMinSize, 0, 150, "Min Region Size %1.0f");
		ImGui::SliderFloat("##MaxRegiom", &cs.p_regionMergeSize, 0, 150, "Merged Region Size %1.0f");
	}

	if (ImGui::CollapsingHeader("Partitioning"))
	{
		uint partition = cs.p_partitionType;
		
		if (ImGui::RadioButton("Watershed (recommended)", cs.p_partitionType == 0))
			cs.p_partitionType = 0; // Watershed
		if (ImGui::RadioButton("Monotone", cs.p_partitionType == 1))
			cs.p_partitionType = 1; // Monotone
		if (ImGui::RadioButton("Layers", cs.p_partitionType == 2))
			cs.p_partitionType = 2; // Layers
	}

	if (ImGui::CollapsingHeader("Polygonitzation"))
	{
		ImGui::SliderFloat("##MaxEdgeLength", &cs.p_edgeMaxLen, 0, 50, "Max Edge Length %1.0f");
		ImGui::SliderFloat("##MaxEdgeError", &cs.p_edgeMaxError, 0.1f, 3.0f, "Max Edge Error %.1f");
		ImGui::SliderFloat("##VertsPerPoly", &cs.p_vertsPerPoly, 3, 12, "Verts Per Poly %1.0f");
	}

	if (ImGui::CollapsingHeader("Detail Mesh"))
	{
		ImGui::SliderFloat("##SampleDistance", &cs.p_detailSampleDist, 0, 16, "Sample Distance %1.0f");
		ImGui::SliderFloat("##SampleError", &cs.p_detailSampleMaxError, 0, 16, "Max Sample Error %1.0f");
	}

	if (ImGui::Button("Bake"))
		HandleInputMeshes();

	ImGui::SameLine();

	if (ImGui::Button("Delete Navmesh"))
		App->navigation->CleanUp();

	ImGui::Checkbox("Draw Navmesh", &App->navigation->drawNavmesh);

	ImGui::End();

	return true;
}

void PanelNavigation::ResetCommonSettings()
{
	cs.p_cellSize = 0.3f;
	cs.p_cellHeight = 0.2f;
	cs.p_agentHeight = 2.0f;
	cs.p_agentRadius = 0.6f;
	cs.p_agentMaxClimb = 0.9f;
	cs.p_agentMaxSlope = 45.0f;
	cs.p_regionMinSize = 8.0f;
	cs.p_regionMergeSize = 20.0f;
	cs.p_partitionType = 0;
	cs.p_edgeMaxLen = 12.0f;
	cs.p_edgeMaxError = 1.3f;
	cs.p_vertsPerPoly = 6.0f;
	cs.p_detailSampleDist = 6.0f;
	cs.p_detailSampleMaxError = 1.0f;
}

void PanelNavigation::HandleInputMeshes() const
{
	std::vector<ComponentMesh*> staticsMeshComp;
	std::vector<GameObject*> gos;
	App->GOs->GetStaticGameobjects(gos);

	for each (auto go in gos)
	{
		assert(go->IsStatic());
		if (go->cmp_mesh)
			staticsMeshComp.push_back(go->cmp_mesh);
	}

	if (staticsMeshComp.size() <= 0)
	{
		DEPRECATED_LOG("Rc: No static gameobjects");
		return;
	}

	NMInputGeom p_inputGeom;
	math::AABB aabb;
	aabb.SetNegativeInfinity();

	memset(&p_inputGeom, 0, sizeof(NMInputGeom));

	for (int i = 0; i < staticsMeshComp.size(); ++i)
	{
		const ResourceMesh* res = (const ResourceMesh*)App->res->GetResource(staticsMeshComp[i]->res);
		if (!res)
			continue;
		p_inputGeom.i_nmeshes += 1;
		aabb.Enclose(staticsMeshComp[i]->GetParent()->boundingBox);
	}

	if (p_inputGeom.i_nmeshes <= 0)
	{
		DEPRECATED_LOG("rc: No meshes in static gameobjects");
		return;
	}

	p_inputGeom.i_meshes = new M_Mesh[p_inputGeom.i_nmeshes];

	for (int i = 0; i < staticsMeshComp.size(); ++i)
	{
		const ResourceMesh* res = (const ResourceMesh*)App->res->GetResource(staticsMeshComp[i]->res);
		if (res)
		{
			p_inputGeom.i_meshes[i].m_ntris = res->GetIndicesCount() / 3;
			p_inputGeom.i_ntris += p_inputGeom.i_meshes[i].m_ntris;
			p_inputGeom.i_meshes[i].m_nverts = res->GetVerticesCount();
			p_inputGeom.i_meshes[i].m_tris = new int[p_inputGeom.i_meshes[i].m_ntris * 3];
			p_inputGeom.i_meshes[i].m_verts = new float[p_inputGeom.i_meshes[i].m_nverts * 3];
			res->GetIndices((uint*)p_inputGeom.i_meshes[i].m_tris);
			res->GetTris(p_inputGeom.i_meshes[i].m_verts);

			p_inputGeom.i_meshes[i].walkable = staticsMeshComp[i]->nv_walkable;

			math::float4x4 gMatrix = staticsMeshComp[i]->GetParent()->transform->GetGlobalMatrix();
			for (int j = 0; j < p_inputGeom.i_meshes[i].m_nverts * 3; j += 3)
			{
				math::float3 globalVert = (gMatrix * math::float4(p_inputGeom.i_meshes[i].m_verts[j],
																  p_inputGeom.i_meshes[i].m_verts[j + 1],
																  p_inputGeom.i_meshes[i].m_verts[j + 2],
																  1)).xyz();

				memcpy(&p_inputGeom.i_meshes[i].m_verts[j], globalVert.ptr(), sizeof(float) * 3);
			}
		}
	}

	memcpy(&p_inputGeom + offsetof(NMInputGeom, i_buildSettings), &cs, sizeof(CommonSettings));
	memcpy(p_inputGeom.bMin, aabb.minPoint.ptr(), sizeof(math::float3));
	memcpy(p_inputGeom.bMax, aabb.maxPoint.ptr(), sizeof(math::float3));
	App->navigation->SetInputGeom(p_inputGeom);
	App->navigation->HandleBuild();

	for (int i = 0; i < p_inputGeom.i_nmeshes; ++i)
	{
		delete[] p_inputGeom.i_meshes[i].m_tris;
		delete[] p_inputGeom.i_meshes[i].m_verts;
	}
	delete[] p_inputGeom.i_meshes;
}
