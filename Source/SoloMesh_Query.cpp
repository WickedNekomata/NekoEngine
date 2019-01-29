#include "SoloMesh_Query.h"
#include "InputGeom.h"
#include "M_DebugDraw.h"

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

SoloMesh_Query::SoloMesh_Query()
{
}

SoloMesh_Query::~SoloMesh_Query()
{
}

void SoloMesh_Query::CleanUp()
{
	if (m_triareas) delete[] m_triareas;
	m_triareas = 0;
}

void SoloMesh_Query::SetInputGeom(InputGeom& inputGeom)
{
	if (m_geom) delete m_geom;
	m_geom = new InputGeom();
	memcpy(m_geom, &inputGeom, sizeof(InputGeom));
}

bool SoloMesh_Query::HandleBuild()
{
	if (!m_geom)
	{
		CONSOLE_LOG("buildNavigation: Input mesh is not specified.");
		return false;
	}

	CleanUp();

	const float* bmin = (const float*)m_geom->bMin;
	const float* bmax = (const float*)m_geom->bMax;
	const int nverts = m_geom->m_nverts;
	float* verts = new float[nverts * 3];
	memcpy(verts, m_geom->m_verts, sizeof(float) * nverts * 3);

	const int ntris = m_geom->m_ntris;
	int* tris = new int[ntris * 3];
	memcpy(tris, m_geom->m_tris, sizeof(int) * ntris * 3);

	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = m_geom->m_buildSettings.cellSize;
	m_cfg.ch = m_geom->m_buildSettings.cellHeight;
	m_cfg.walkableSlopeAngle = m_geom->m_buildSettings.agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(m_geom->m_buildSettings.agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(m_geom->m_buildSettings.agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(m_geom->m_buildSettings.agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(m_geom->m_buildSettings.edgeMaxLen / m_geom->m_buildSettings.cellSize);
	m_cfg.maxSimplificationError = m_geom->m_buildSettings.edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(m_geom->m_buildSettings.regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(m_geom->m_buildSettings.regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)m_geom->m_buildSettings.vertsPerPoly;
	m_cfg.detailSampleDist = m_geom->m_buildSettings.detailSampleDist < 0.9f ? 0 : m_geom->m_buildSettings.cellSize * m_geom->m_buildSettings.detailSampleDist;
	m_cfg.detailSampleMaxError = m_geom->m_buildSettings.cellHeight * m_geom->m_buildSettings.detailSampleMaxError;

	// area could be specified by an user defined box, etc.
	rcVcopy(m_cfg.bmin, bmin);
	rcVcopy(m_cfg.bmax, bmax);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		CONSOLE_LOG("buildNavigation: Out of memory 'solid'.");
		return false;
	}
	if (!rcCreateHeightfield(&ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		CONSOLE_LOG("buildNavigation: Could not create solid heightfield.");
		return false;
	}

	// allocate n_tris
	m_triareas = new unsigned char[ntris];
	if (!m_triareas)
	{
		CONSOLE_LOG("buildNavigation: Out of memory 'm_triareas' (%d).", 0);
		return false;
	}

	memset(m_triareas, 0, ntris * sizeof(unsigned char));

	// Getting each triangle using tris(indices) and vertices extract face normal and check if walkable
	rcMarkWalkableTriangles(&ctx, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	if (!rcRasterizeTriangles(&ctx, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb))
	{
		CONSOLE_LOG("buildNavigation: Could not rasterize triangles.");
		return false;
	}

	bool m_keepInterResults = false;
	if (!m_keepInterResults)
	{
		delete[] m_triareas;
		m_triareas = 0;
	}

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(&ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(&ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(&ctx, m_cfg.walkableHeight, *m_solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		CONSOLE_LOG("buildNavigation: Out of memory 'chf'.");
		return false;
	}
	if (!rcBuildCompactHeightfield(&ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		CONSOLE_LOG("buildNavigation: Could not build compact data.");
		return false;
	}

	if (!m_keepInterResults)
	{
		rcFreeHeightField(m_solid);
		m_solid = 0;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(&ctx, m_cfg.walkableRadius, *m_chf))
	{
		CONSOLE_LOG("buildNavigation: Could not erode.");
		return false;
	}

	// (Optional) Mark areas.
	//const ConvexVolume* vols = m_geom->getConvexVolumes();
	//for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
		//rcMarkConvexPolyArea(NULL, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(&ctx, *m_chf))
	{
		CONSOLE_LOG("buildNavigation: Could not build distance field.");
		return false;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(&ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
	{
		CONSOLE_LOG("buildNavigation: Could not build watershed regions.");
		return false;
	}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		CONSOLE_LOG("buildNavigation: Out of memory 'cset'.");
		return false;
	}
	if (!rcBuildContours(&ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		CONSOLE_LOG("buildNavigation: Could not create contours.");
		return false;
	}

	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		CONSOLE_LOG("buildNavigation: Out of memory 'pmesh'.");
		return false;
	}
	if (!rcBuildPolyMesh(&ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		CONSOLE_LOG("buildNavigation: Could not triangulate contours.");
		return false;
	}

	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		CONSOLE_LOG("buildNavigation: Out of memory 'pmdtl'.");
		return false;
	}

	if (!rcBuildPolyMeshDetail(&ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		CONSOLE_LOG("buildNavigation: Could not build detail mesh.");
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
		params.walkableHeight = 2.0f;
		params.walkableRadius = 0.6f;
		params.walkableClimb = 0.9f;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			//m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return false;
		}

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			dtFree(navData);
			//m_ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
			return false;
		}

		dtStatus status;

		status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
		//	m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
			return false;
		}


		// Next Step
		/*
		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			//m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
			return false;
		}	
		*/
	}

	patata = true;

	return true;
}

void SoloMesh_Query::Draw()
{
	if (patata)
	{
		M_DebugDraw dd;
		//for (int i = 0; i < m_pmesh->npolys; ++i)
			duDebugDrawNavMesh(&dd, *m_navMesh, 0);
			//duDebugDrawNavMeshPoly(&dd, *m_navMesh, m_pmesh->polys[i], duRGBA(0, 0, 0, 128));
	}
}
