#include "NMSupplier.h"
#include "NMInputGeom.h"
#include "NMDebugDraw.h"

#include "Recast&Detour/Detour/Include/DetourNavMesh.h"
#include "Recast&Detour/Detour/Include/DetourNavMeshBuilder.h"
#include "Recast&Detour/DebugUtils/Include/DetourDebugDraw.h"

#include "ResourceMesh.h"

#include "Application.h"
#include "DebugDrawer.h"

#include "Globals.h"
#include <math.h>

// Useful sites to understand the process
// official github: https://github.com/recastnavigation/recastnavigation
// non/official documentation: http://www.stevefsp.org/projects/rcndoc/prod/index.html

// Heightfields explanation -> http://www.critterai.org/projects/nmgen_study/heightfields.html

NMSupplier::NMSupplier()
{
	m_navQuery = dtAllocNavMeshQuery();
}

NMSupplier::~NMSupplier()
{
	dtFreeNavMeshQuery(m_navQuery);
}

void NMSupplier::CleanUp()
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

	patata = false;
}

void NMSupplier::SetInputGeom(NMInputGeom& inputGeom)
{
	CleanUp();
	m_geom = new NMInputGeom();
	memcpy(m_geom, &inputGeom, sizeof(NMInputGeom));
}

bool NMSupplier::HandleBuild()
{
	if (!m_geom)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Input mesh is not specified.");
		return false;
	}

	bool m_keepInterResults = false;

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

	if (!m_keepInterResults)
	{
		delete[] m_triareas;
		m_triareas = 0;
		delete m_geom;
		m_geom = 0;
	}

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

	if (!m_keepInterResults)
	{
		rcFreeHeightField(m_solid);
		m_solid = 0;
	}

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

	if (!m_keepInterResults)
	{
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
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

		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
			return false;
		}	
	}
	patata = true;

	delete m_ctx;
	m_ctx = NULL;

	return true;
}

void NMSupplier::Draw()
{
	if (patata)
	{
		NMDebugDraw dd;
		duDebugDrawNavMesh(&dd, *m_navMesh, 0);
	}
}
