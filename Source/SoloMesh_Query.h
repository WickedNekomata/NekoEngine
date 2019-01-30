#ifndef __SOLOMESH_QUERY_H__
#define __SOLOMESH_QUERY_H__

#include "Recast&Detour/Recast/Include/Recast.h"
#include "BuildContext.h"

class SoloMesh_Query
{
public:
	SoloMesh_Query();
	virtual ~SoloMesh_Query();

	void SetInputGeom(class InputGeom& inputGeom);

protected:
	class InputGeom* m_geom = nullptr;
	class dtNavMesh* m_navMesh = nullptr;
	class dtNavMeshQuery* m_navQuery;
	class dtCrowd* m_crowd;

	unsigned char* m_triareas = nullptr;
	rcHeightfield* m_solid = nullptr;
	rcCompactHeightfield* m_chf = nullptr;
	rcContourSet* m_cset = nullptr;
	rcPolyMesh* m_pmesh = nullptr;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh = nullptr;

	BuildContext* m_ctx = nullptr;

	bool patata = false;

public:
	void CleanUp();

	bool HandleBuild();

	void Draw();
};

#endif