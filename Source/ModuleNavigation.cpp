#include "ModuleNavigation.h"
#include "NMInputGeom.h"
#include "NMDebugDraw.h"

#include "ModuleGOs.h"
#include "GameObject.h"
#include "ComponentNavAgent.h"
#include "ComponentTransform.h"

#include "Recast&Detour/Detour/Include/DetourNavMesh.h"
#include "Recast&Detour/Detour/Include/DetourNavMeshBuilder.h"
#include "Recast&Detour/DebugUtils/Include/DetourDebugDraw.h"
#include "Recast&Detour/DetourCrowd/Include/DetourCrowd.h"
#include "Recast&Detour/Detour/Include/DetourCommon.h"

#include "ResourceMesh.h"

#include "Application.h"
#include "ModuleTimeManager.h"
#include "DebugDrawer.h"

#include "Globals.h"

#include "MathGeoLib/include/Math/MathAll.h"
#include <math.h>

// Useful sites to understand the process
// official github: https://github.com/recastnavigation/recastnavigation
// non/official documentation: http://www.stevefsp.org/projects/rcndoc/prod/index.html

// Heightfields explanation -> http://www.critterai.org/projects/nmgen_study/heightfields.html

ModuleNavigation::ModuleNavigation(bool start_enabled)
{
	
}

ModuleNavigation::~ModuleNavigation()
{
	
}

bool ModuleNavigation::Init(JSON_Object* jObject)
{
	return true;
}

update_status ModuleNavigation::Update()
{
	if (m_navMesh && m_crowd)
	{
		m_crowd->update(App->timeManager->GetDt(), 0);

		for each(ComponentNavAgent* agent in c_agents)
		{
			
			int index = agent->GetIndex();
			const dtCrowdAgent* ag = m_crowd->getAgent(index);
			if (ag->targetState == DT_CROWDAGENT_STATE_WALKING || ag->targetState == DT_CROWDAGENT_STATE_OFFMESH)
			{
				// Here we are forcing new position and rotation and manually sending an event to recaculcate bb.
				// We are ignoring to update physics position and/or camera->frustum position.

				// Set new gameobject's position
				ComponentTransform* trm = agent->GetParent()->transform;
				memcpy(&trm->position, ag->npos, sizeof(float) * 3);

				// Add the offset to obtain the real gameobject position
				trm->position[0] += ag->offsetPos[0];
				trm->position[1] += ag->offsetPos[1];
				trm->position[2] += ag->offsetPos[2];

				// Face gameobject to velocity dir
				// vel equals to current velocity, nvel equals to desired velocity
				// using nvel instead of vel would end up with a non smoothy rotation.
				math::float3 direction;
				memcpy(&direction, ag->vel, sizeof(float) * 3);
				direction.Normalize();
				float angle = math::Atan2(direction.x, direction.z);
				math::Quat new_rotation;
				new_rotation.SetFromAxisAngle(math::float3(0, 1, 0), angle);
				trm->rotation = new_rotation;

				// Recalculate bounding box
				System_Event newEvent;
				newEvent.goEvent.gameObject = trm->GetParent();
				newEvent.type = System_Event_Type::RecalculateBBoxes;
				App->PushSystemEvent(newEvent);		
			}
		}
	}

	return UPDATE_CONTINUE;
}

bool ModuleNavigation::CleanUp()
{
	cleanup();
	return true;
}

void ModuleNavigation::OnSystemEvent(System_Event e)
{
	switch (e.type)
	{
	case System_Event_Type::Play:
	{
		if (!m_navMesh)
			return;

		m_navQuery = dtAllocNavMeshQuery();

		dtStatus status;

		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			DEPRECATED_LOG("Could not init Detour navmesh query");
			return;
		}

		InitCrowd();

		for each (ComponentNavAgent* ag in c_agents)
			ag->AddAgent();

		break;
	}
	case System_Event_Type::Stop:
	{
		if (!m_navMesh)
			return;

		for each (ComponentNavAgent* ag in c_agents)
			RemoveAgent(ag->GetIndex());

		dtFreeCrowd(m_crowd);
		dtFreeNavMeshQuery(m_navQuery);
		m_crowd = 0;
		m_navQuery = 0;
	}
		break;
	}
}

void ModuleNavigation::InitCrowd()
{
	m_crowd = dtAllocCrowd();

	//args 1- Max agents, 2- radius agent, 3- navmesh
	m_crowd->init(max_Agents, m_cfg.walkableRadius * m_cfg.cs, m_navMesh);

	// Setup local avoidance params to different qualities.
	dtObstacleAvoidanceParams params;
	// Use mostly default settings, copy from dtCrowd.
	memcpy(&params, m_crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));
	// check mikkos answer to understand local avoidance params
	// https://groups.google.com/forum/#!searchin/recastnavigation/local$20avoidance%7Csort:date/recastnavigation/3hFcUXb-Cjc/XeC7t2CPMCYJ

	// Low (11)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 1;
	m_crowd->setObstacleAvoidanceParams(0, &params);

	// Medium (22)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 2;
	m_crowd->setObstacleAvoidanceParams(1, &params);

	// Good (45)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 3;
	m_crowd->setObstacleAvoidanceParams(2, &params);

	// High (66)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 3;
	params.adaptiveDepth = 3;

	m_crowd->setObstacleAvoidanceParams(3, &params);
}

void ModuleNavigation::Draw() const
{
	if (m_navMesh && drawNavmesh)
	{
		NMDebugDraw dd;
		duDebugDrawNavMesh(&dd, *m_navMesh, 0);
	}
}

void ModuleNavigation::AddComponent(ComponentNavAgent* cmp_agent)
{
	c_agents.push_back(cmp_agent);
}

void ModuleNavigation::EraseComponent(ComponentNavAgent* cmp_agent)
{
	std::vector<ComponentNavAgent*>::const_iterator it = std::find(c_agents.begin(), c_agents.end(), cmp_agent);
	if (it != c_agents.end()) { c_agents.erase(it); }
}

void ModuleNavigation::cleanup()
{
	if (m_triareas) delete[] m_triareas;
	if (m_geom) delete m_geom;
	m_triareas = 0;
	m_geom = 0;

	rcFreeHeightField(m_solid);
	m_solid = 0;
	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeContourSet(m_cset);
	m_cset = 0;
	rcFreePolyMesh(m_pmesh);
	m_pmesh = 0;
	rcFreePolyMeshDetail(m_dmesh);
	m_dmesh = 0;
	dtFreeNavMesh(m_navMesh);
	m_navMesh = 0;
}

void ModuleNavigation::SetInputGeom(NMInputGeom& inputGeom)
{
	cleanup();
	m_geom = new NMInputGeom();
	memcpy(m_geom, &inputGeom, sizeof(NMInputGeom));
}

void ModuleNavigation::FindPath(float* start, float* end, float* path, int pathCount, int maxPath) const
{
	// TODO find path from point to point
	//dtPolyRef polyRefStart, polyRefEnd, polyPath;
	//float* nearestPTStart, *nearestPTEnd;
	//float extends[] = { 10.0f,10.0f,10.0f }; // thats odd
	//m_navQuery->findNearestPoly(start, extends, &m_filter, &polyRefStart, nearestPTStart);
	//m_navQuery->findNearestPoly(start, extends, &m_filter, &polyRefEnd, nearestPTEnd);
	//m_navQuery->findPath(polyRefStart, polyRefEnd, start, end, &m_filter, &polyPath, &pathCount, maxPath);
	//path = new float[pathCount * 3];
}

int ModuleNavigation::AddAgent(const float* p, float radius, float height, float maxAcc, float maxSpeed, float collQueryRange, float pathOptimRange, unsigned char updateFlags, unsigned char obstacleAvoidanceType) const
{
	if (!m_crowd) return -1;

	dtCrowdAgentParams params;
	memset(&params, 0, sizeof(dtCrowdAgentParams));
	params.radius = radius;
	params.height = height;
	params.maxAcceleration = maxAcc;
	params.maxSpeed = maxSpeed;
	params.collisionQueryRange = collQueryRange;
	params.pathOptimizationRange = pathOptimRange;
	params.updateFlags = updateFlags;
	params.obstacleAvoidanceType = obstacleAvoidanceType;

	return m_crowd->addAgent(p, &params);
}

bool ModuleNavigation::UpdateAgentParams(int indx, float radius, float height, float maxAcc, float maxSpeed, float collQueryRange, float pathOptimRange, unsigned char updateFlags, unsigned char obstacleAvoidanceType) const
{
	if (!m_crowd->getAgent(indx))
		return false;

	dtCrowdAgentParams params;
	memset(&params, 0, sizeof(dtCrowdAgentParams));
	params.radius = radius;
	params.height = height;
	params.maxAcceleration = maxAcc;
	params.maxSpeed = maxSpeed;
	params.collisionQueryRange = collQueryRange;
	params.pathOptimizationRange = pathOptimRange;
	params.updateFlags = updateFlags;
	params.obstacleAvoidanceType = obstacleAvoidanceType;

	m_crowd->updateAgentParameters(indx, &params);
}

void ModuleNavigation::RemoveAgent(int indx) const
{
	if (!m_crowd) return;

	m_crowd->removeAgent(indx);
}

void ModuleNavigation::SetDestination(const float* p, int indx) const
{
	if (!m_navMesh || !m_crowd) return;

	float vel[3];
	const dtCrowdAgent* ag = m_crowd->getAgent(indx);
	dtPolyRef polyRefTarget;
	float targetPos[3];
	m_navQuery->findNearestPoly(p, m_crowd->getQueryExtents(), m_crowd->getFilter(ag->params.queryFilterType), &polyRefTarget, targetPos);
	if (ag && ag->active)
		m_crowd->requestMoveTarget(indx, polyRefTarget, targetPos);
}

bool ModuleNavigation::IsWalking(int index) const
{
	const dtCrowdAgent* ag = m_crowd->getAgent(index);
	return ag->state == DT_CROWDAGENT_STATE_WALKING;
}

void ModuleNavigation::calcVel(float* vel, const float* pos, const float* tgt, const float speed)
{
	dtVsub(vel, tgt, pos);
	vel[1] = 0.0;
	dtVnormalize(vel);
	dtVscale(vel, vel, speed);
}

// Revise leaks and reduce vars to not keep intern results
bool ModuleNavigation::HandleBuild()
{
	if (!m_geom)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Input mesh is not specified.");
		return false;
	}

	m_ctx = new NMBuildContext();

	// Set all input settings to recast
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = m_geom->i_buildSettings.cellSize;
	m_cfg.ch = m_geom->i_buildSettings.cellHeight;
	m_cfg.walkableSlopeAngle = m_geom->i_buildSettings.agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(m_geom->i_buildSettings.agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(m_geom->i_buildSettings.agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(m_geom->i_buildSettings.agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(m_geom->i_buildSettings.edgeMaxLen / m_geom->i_buildSettings.cellSize);
	m_cfg.maxSimplificationError = m_geom->i_buildSettings.edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(m_geom->i_buildSettings.regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(m_geom->i_buildSettings.regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)m_geom->i_buildSettings.vertsPerPoly;
	m_cfg.detailSampleDist = m_geom->i_buildSettings.detailSampleDist < 0.9f ? 0 : m_geom->i_buildSettings.cellSize * m_geom->i_buildSettings.detailSampleDist;
	m_cfg.detailSampleMaxError = m_geom->i_buildSettings.cellHeight * m_geom->i_buildSettings.detailSampleMaxError;

	rcVcopy(m_cfg.bmin, (const float*)m_geom->bMin);
	rcVcopy(m_cfg.bmax, (const float*)m_geom->bMax);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	RecastPartitionType partition = m_geom->i_buildSettings.partitionType;

	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return false;
	}
	if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return false;
	}

	// allocate n_tris
	m_triareas = new unsigned char[m_geom->i_ntris];
	if (!m_triareas)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", m_geom->i_ntris);
		return false;
	}

	memset(m_triareas, 0, m_geom->i_ntris * sizeof(unsigned char));

	// For each mesh:
	// Getting each triangle using tris(indices) and vertices extract face normal and check if walkable
	// Rasterize all triangles from the mesh into the heightfield
	for (int i = 0; i < m_geom->i_nmeshes; ++i)
	{

		// method edited see this: https://groups.google.com/forum/#!searchin/recastnavigation/no$20walkable%7Csort:date/recastnavigation/Pj4zgOvhZCU/fnAK6zT3CwAJ
		//						   http://masagroup.github.io/recastdetour/group__recast.html#ga1346288993c8ab565bee3e212e9c9890
		rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, m_geom->i_meshes[i].m_verts, m_geom->i_meshes[i].m_nverts,
					m_geom->i_meshes[i].m_tris, m_geom->i_meshes[i].m_ntris, m_triareas, /* new argument*/ m_geom->i_meshes[i].walkable ? 63 : 0);

		if (!rcRasterizeTriangles(m_ctx, m_geom->i_meshes[i].m_verts, m_geom->i_meshes[i].m_nverts,
			m_geom->i_meshes[i].m_tris, m_triareas, m_geom->i_meshes[i].m_ntris, *m_solid, m_cfg.walkableClimb))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
			return false;
		}
	}
	
	delete[] m_triareas;
	m_triareas = 0;
	delete m_geom;
	m_geom = 0;

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return false;
	}
	if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return false;
	}

	rcFreeHeightField(m_solid);
	m_solid = 0;

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return false;
	}

	if (partition == SAMPLE_PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(m_ctx, *m_chf))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
			return false;
		}

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
			return false;
		}
	}
	else if (partition == SAMPLE_PARTITION_MONOTONE)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
			return false;
		}
	}
	else // SAMPLE_PARTITION_LAYERS
	{
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildLayerRegions(m_ctx, *m_chf, 0, m_cfg.minRegionArea))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
			return false;
		}
	}


	// Step 5. Trace and simplify region contours.
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return false;
	}
	if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return false;
	}

	// Step 6. Build polygons mesh from contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return false;
	}
	if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return false;
	}

	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
		return false;
	}

	if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		return false;
	}

	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeContourSet(m_cset);
	m_cset = 0;

	for (int i = 0; i < m_pmesh->npolys; ++i)
	{
		if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
			m_pmesh->flags[i] = 0x01;
		
	}
	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		params.offMeshConVerts = NULL;
		params.offMeshConRad = NULL;
		params.offMeshConDir = NULL;
		params.offMeshConAreas = NULL;
		params.offMeshConFlags = NULL;
		params.offMeshConUserID = NULL;
		params.offMeshConCount = NULL;
		params.walkableHeight = m_cfg.walkableHeight;
		params.walkableRadius = m_cfg.walkableRadius;
		params.walkableClimb = m_cfg.walkableClimb;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return false;
		}

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			dtFree(navData);
			m_ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
			return false;
		}

		dtStatus status;

		status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
			return false;
		}
	}

	delete m_ctx;
	m_ctx = NULL;

	return true;
}

int ModuleNavigation::GetNavMeshSerialitzationBytes() const
{
	if (!m_navMesh)
		return sizeof(size_t);

	size_t size = sizeof(size_t);
	const dtMeshTile* tile;
	for (int i = 0; i < m_navMesh->getMaxTiles(); ++i)
	{
		tile = ((const dtNavMesh*)m_navMesh)->getTile(i);
		size += tile->dataSize;
	}

	size += sizeof(rcConfig);

	return size;
}

void ModuleNavigation::SaveNavmesh(char*& cursor)
{
	if (!m_navMesh)
	{
		size_t noData = 0;
		memcpy(cursor, &noData, sizeof(size_t));
		return;
	}
	size_t size = 0;
	const dtMeshTile* tile;
	for (int i = 0; i < m_navMesh->getMaxTiles(); ++i)
	{
		tile = ((const dtNavMesh*)m_navMesh)->getTile(i);
		size += tile->dataSize;
	}
	memcpy(cursor, &size, sizeof(size_t));
	cursor += sizeof(size_t);
	for (int i = 0; i < m_navMesh->getMaxTiles(); ++i)
	{
		tile = ((const dtNavMesh*)m_navMesh)->getTile(i);
		memcpy(cursor, tile->data, sizeof(uchar) * tile->dataSize);
		cursor += sizeof(uchar) * tile->dataSize;
	}

	memcpy(cursor, &m_cfg, sizeof(rcConfig));
	cursor += sizeof(rcConfig);
}

void ModuleNavigation::LoadNavmesh(char*& cursor)
{
	if (m_navMesh)
		cleanup();

	size_t size;
	memcpy(&size, cursor, sizeof(size_t));
	cursor += sizeof(size_t);

	if (size <= 0)
	{
		cursor += sizeof(rcConfig);
		return;
	}

	uchar* data = (uchar*)dtAlloc(size, dtAllocHint::DT_ALLOC_PERM);
	memcpy(data, cursor, size);
	cursor += size;

	memcpy(&m_cfg, cursor, sizeof(rcConfig));
	cursor += sizeof(rcConfig);

	m_navMesh = dtAllocNavMesh();
	if (!m_navMesh)
	{
		dtFree(data);
		CONSOLE_LOG(LogTypes::Error, "Could not create Detour navmesh");
		return;
	}

	dtStatus status;

	status = m_navMesh->init(data, size, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status))
	{
		dtFree(data);
		CONSOLE_LOG(LogTypes::Error, "Could not init Detour navmesh");
		return;
	}
}
