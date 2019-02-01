#ifndef __NM_SUPPLIER_H__
#define __NM_SUPPLIER_H__

#include "Recast&Detour/Recast/Include/Recast.h"
#include "NMBuildContext.h"

class NMSupplier
{
public:
	NMSupplier();
	virtual ~NMSupplier();

	void SetInputGeom(class NMInputGeom& inputGeom);

protected:
	class NMInputGeom* m_geom = nullptr;
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

	NMBuildContext* m_ctx = nullptr;

	bool patata = false;

public:
	void CleanUp();

	bool HandleBuild();

	void Draw();
};

#endif