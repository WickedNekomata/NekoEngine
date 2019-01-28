#ifndef __SOLOMESH_QUERY_H__
#define __SOLOMESH_QUERY_H__

#include "Recast&Detour/Recast/Include/Recast.h"

class SoloMesh_Query
{
public:
	SoloMesh_Query();
	virtual ~SoloMesh_Query();

	void SetInputGeom(class InputGeom& inputGeom);

protected:
	class InputGeom* m_geom = nullptr;
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
	class dtCrowd* m_crowd;

	unsigned char* m_triareas = nullptr;
	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh;

	rcContext ctx;

	void CleanUp();

public:
	bool HandleBuild();
};

#endif