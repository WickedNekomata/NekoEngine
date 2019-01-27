#include "SoloMesh_Query.h"
#include "InputGeom.h"

#include "ResourceMesh.h"

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
	delete[] m_triareas;
	m_triareas = 0;
}

bool SoloMesh_Query::HandleBuild()
{
	if (!m_geom || !m_geom->m_mesh)
	{
		CONSOLE_LOG("buildNavigation: Input mesh is not specified.");
		return false;
	}

	CleanUp();

	//const float* bmin = m_geom->m_mesh->
	//const float* bmax = m_geom->getNavMeshBoundsMax();
	const int nverts = m_geom->m_mesh->GetVertsCount();
	float* verts = new float[nverts];
	m_geom->m_mesh->GetVerts(verts);

	const int ntris = m_geom->m_mesh->GetTrisCount();
	int* tris = new int[ntris];
	m_geom->m_mesh->GetTris(tris);

	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = 0.3f;
	m_cfg.ch = 0.2f;
	m_cfg.walkableSlopeAngle = 45.0f;
	m_cfg.walkableHeight = (int)ceilf(2.0f / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(0.9f / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(0.6 / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(12.0f / 0.3f);
	m_cfg.maxSimplificationError = 1.3f;
	m_cfg.minRegionArea = (int)rcSqr(8.0f);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(20.0f);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)6.0f;
	m_cfg.detailSampleDist = 6.0f < 0.9f ? 0 : 0.3f * 6.0f;
	m_cfg.detailSampleMaxError = 0.2f * 1.0f;

	// area could be specified by an user defined box, etc.
	float m_meshBMin[3] = {0.2f,0.2f,0.2f}, m_meshBMax[3] = { 0.2f,0.2f,0.2f };
	rcVcopy(m_cfg.bmin, m_meshBMin);
	rcVcopy(m_cfg.bmax, m_meshBMax);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		CONSOLE_LOG("buildNavigation: Out of memory 'solid'.");
		return false;
	}
	if (!rcCreateHeightfield(NULL, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
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
	rcMarkWalkableTriangles(NULL, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	if (!rcRasterizeTriangles(NULL, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb))
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
	rcFilterLowHangingWalkableObstacles(NULL, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(NULL, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(NULL, m_cfg.walkableHeight, *m_solid);

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
	if (!rcBuildCompactHeightfield(NULL, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
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
	if (!rcErodeWalkableArea(NULL, m_cfg.walkableRadius, *m_chf))
	{
		CONSOLE_LOG("buildNavigation: Could not erode.");
		return false;
	}

	// (Optional) Mark areas.
	//const ConvexVolume* vols = m_geom->getConvexVolumes();
	//for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
		//rcMarkConvexPolyArea(NULL, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(NULL, *m_chf))
	{
		CONSOLE_LOG("buildNavigation: Could not build distance field.");
		return false;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(NULL, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
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
	if (!rcBuildContours(NULL, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
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
	if (!rcBuildPolyMesh(NULL, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
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

	if (!rcBuildPolyMeshDetail(NULL, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
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

	return true;
}
