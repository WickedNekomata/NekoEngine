#include "SoloMesh_Query.h"
#include "InputGeom.h"

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

	/*
	// allocate n_tris
	m_triareas = new unsigned char[0];
	if (!m_triareas)
	{
		CONSOLE_LOG("buildNavigation: Out of memory 'm_triareas' (%d).", 0);
		return false;
	}
	
	memset(m_triareas, 0, ntris * sizeof(unsigned char));
	rcMarkWalkableTriangles(NULL, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	if (!rcRasterizeTriangles(m_ctx, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
		return false;
	}

	bool m_keepInterResults = false;
	if (!m_keepInterResults)
	{
		delete[] m_triareas;
		m_triareas = 0;
	}
	*/

	return true;
}
