#include "PanelNavMesh.h"

#include "imgui/imgui.h"

#include "Application.h"
#include "SoloMesh_Query.h"
#include "InputGeom.h"
#include "ModuleGOs.h"
#include "GameObject.h"
#include "ComponentMesh.h"

#include "ResourceMesh.h"
#include "ModuleResourceManager.h"

#include "MathGeoLib/include/Math/float4.h"

PanelNavMesh::PanelNavMesh(char* name) : Panel(name)
{
	ResetCommonSettings();
}

PanelNavMesh::~PanelNavMesh()
{
}

bool PanelNavMesh::Draw()
{
	ImGui::Begin(name, &enabled);

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Build Settings");
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		ResetCommonSettings();

	if (ImGui::CollapsingHeader("Rasteritzation"))
	{
		ImGui::SliderFloat("##CellSize", &cs.p_cellSize, 0.10f, 1.0f, "Cell Size	 %.01f");
		ImGui::SliderFloat("##CellHeight", &cs.p_cellHeight, 0.10f, 1.0f, "Cell Height %.01f");
	}

	if (ImGui::CollapsingHeader("Agent"))
	{
		ImGui::SliderFloat("##A_Height", &cs.p_agentHeight, 0.0f, 5.0f, "Height %.1f");
		ImGui::SliderFloat("A_Radius", &cs.p_agentRadius, 0.0f, 5.0f, "Radius %.1f");
		ImGui::SliderFloat("A_MClimb", &cs.p_agentMaxClimb, 0.0f, 5.0f, "Max Climb %.1f");
		ImGui::SliderFloat("A_MSlope", &cs.p_agentMaxSlope, 0.0f, 90.0f, "Max Slope %1.f");
	}

	if (ImGui::CollapsingHeader("Region"))
	{
		ImGui::SliderFloat("##MinRegion", &cs.p_regionMinSize, 0, 150, "Min Region Size %d");
		ImGui::SliderFloat("#MaxRegiom", &cs.p_regionMergeSize, 0, 150, "Merged Region Size %d");
	}

	if (ImGui::CollapsingHeader("Partitioning"))
	{
		uint partition = cs.p_partitionType;
		
		if (ImGui::RadioButton("Watershed (recommended)", cs.p_partitionType == 0))
			cs.p_partitionType = 0;
		if (ImGui::RadioButton("Monotone", cs.p_partitionType == 1))
			cs.p_partitionType = 1;
		if (ImGui::RadioButton("Layers", cs.p_partitionType == 2))
			cs.p_partitionType = 2;
	}

	if (ImGui::CollapsingHeader("Polygonitzation"))
	{
		ImGui::SliderFloat("##MaxEdgeLength", &cs.p_edgeMaxLen, 0, 50, "Max Edge Length %d");
		ImGui::SliderFloat("##MaxEdgeError", &cs.p_edgeMaxError, 0.1f, 3.0f, "Max Edge Error %.1f");
		ImGui::SliderFloat("##VertsPerPoly", &cs.p_vertsPerPoly, 3, 12, "Verts Per Poly %d");
	}

	if (ImGui::CollapsingHeader("Detail Mesh"))
	{
		ImGui::SliderFloat("##SampleDistance", &cs.p_detailSampleDist, 0, 16, "Sample Distance %d");
		ImGui::SliderFloat("##SampleError", &cs.p_detailSampleMaxError, 0, 16, "Max Sample Error %d");
	}

	if (ImGui::Button("Bake"))
	{
		HandleInputMeshes();
	}	

	ImGui::End();

	return true;
}

void PanelNavMesh::ResetCommonSettings()
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

void PanelNavMesh::HandleInputMeshes() const
{
	std::vector<GameObject*> statics;
	App->GOs->GetStaticGameObjects(statics);

	InputGeom p_inputGeom;
	memset(&p_inputGeom, 0, sizeof(InputGeom));

	for (int i = 0; i < statics.size(); ++i)
	{
		if (!statics[i]->meshRenderer)
			continue;
		const ResourceMesh* res = (const ResourceMesh*)App->res->GetResource(statics[i]->meshRenderer->res);
		if (!res)
			continue;
		p_inputGeom.m_nverts += res->GetVertsCount();
		p_inputGeom.m_ntris += res->GetIndicesCount() / 3;
	}

	p_inputGeom.m_verts = new float[p_inputGeom.m_nverts * 3];
	p_inputGeom.m_tris = new int[p_inputGeom.m_ntris * 3];

	float* v_cursor = p_inputGeom.m_verts;
	int* t_cursor = p_inputGeom.m_tris;

	math::AABB aabb;
	aabb.SetNegativeInfinity();

	for (int i = 0; i < statics.size(); ++i)
	{
		if (!statics[i]->meshRenderer)
			continue;
		const ResourceMesh* res = (const ResourceMesh*)App->res->GetResource(statics[i]->meshRenderer->res);
		if (!res)
			continue;

		res->GetVerts(v_cursor);
		v_cursor += res->GetVertsCount() * 3;
		res->GetIndices(t_cursor);
		t_cursor += res->GetIndicesCount();

		aabb.Enclose(statics[i]->boundingBox);
	}
	memcpy(&p_inputGeom + offsetof(InputGeom, m_buildSettings), &cs, sizeof(CommonSettings));
	memcpy(p_inputGeom.bMin, aabb.minPoint.ptr(), sizeof(math::float3));
	memcpy(p_inputGeom.bMax, aabb.maxPoint.ptr(), sizeof(math::float3));
	App->soloMeshQuery->SetInputGeom(p_inputGeom);
	App->soloMeshQuery->HandleBuild();

	delete[] p_inputGeom.m_verts;
	delete[] p_inputGeom.m_tris;
}
