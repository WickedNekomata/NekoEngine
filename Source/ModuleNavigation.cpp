#include "ModuleNavigation.h"
#include "NMInputGeom.h"
#include "NMDebugDraw.h"

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

bool ModuleNavigation::Init(JSON_Object * jObject)
{
	return true;
}

update_status ModuleNavigation::Update()
{
	if (m_navMesh && m_crowd)
		m_crowd->update(App->timeManager->GetDt(), 0);

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
		/*
		case EnterPlayMode

		maybe init here navmesh query too?
		m_crowd = dtAllocCrowd();
		init crowd
		
		add agents
		
		*/
	}
}

bool ModuleNavigation::OnGameMode()
{
	if (m_navMesh)
		return;

	m_navQuery = dtAllocNavMeshQuery();

	dtStatus status;

	status = m_navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
	{
		CONSOLE_LOG("Could not init Detour navmesh query");
		return false;
	}

	m_crowd = dtAllocCrowd();

	//args 1- Max agents, 2- radius agent, 3- navmesh
	m_crowd->init(max_Agents, m_cfg.walkableRadius * m_cfg.cs, m_navMesh);

	//m_crowd->getEditableFilter(0)->setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED);

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

	return true;
}

bool ModuleNavigation::OnEditorMode()
{
	if (m_navMesh)
		return;

	dtFreeNavMeshQuery(m_navQuery);
	dtFreeCrowd(m_crowd);
	return true;
}

void ModuleNavigation::Draw()
{
	if (m_navMesh)
	{
		NMDebugDraw dd;
		duDebugDrawNavMesh(&dd, *m_navMesh, 0);
	}
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

int ModuleNavigation::AddAgent(const float* p, float radius, float height, float maxAcc, float maxSpeed, float collQueryRange, float pathOptimRange, unsigned char updateFlags, unsigned char obstacleAvoidanceType, unsigned char queryFilterType)
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
	params.queryFilterType = queryFilterType;

	return m_crowd->addAgent(p, &params);
}

void ModuleNavigation::RemoveAgent(int indx)
{
	if (!m_crowd) return;

	m_crowd->removeAgent(indx);
}

void ModuleNavigation::SetDestination(const float* p, int indx)
{
	if (!m_navMesh || !!m_crowd) return;

	float vel[3];
	const dtCrowdAgent* ag = m_crowd->getAgent(indx);
	if (ag && ag->active)
	{
		calcVel(vel, ag->npos, p, ag->params.maxSpeed);
		m_crowd->requestMoveVelocity(indx, vel);
	}
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
