#ifndef __SOLOMESH_QUERY_H__
#define __SOLOMESH_QUERY_H__

#include "Recast&Detour/Recast/Include/Recast.h"

class SoloMesh_Query
{
public:
	SoloMesh_Query();
	virtual ~SoloMesh_Query();

protected:
	class InputGeom* m_geom;
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
	class dtCrowd* m_crowd;

	rcConfig m_cfg;

	unsigned char* m_triareas;
	rcHeightfield* m_solid;

	void CleanUp();

	bool HandleBuild();
};

#endif