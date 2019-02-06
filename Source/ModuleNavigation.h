#ifndef __NM_SUPPLIER_H__
#define __NM_SUPPLIER_H__

#include "Module.h"

#include "Recast&Detour/Recast/Include/Recast.h"
#include "NMBuildContext.h"

constexpr int max_Agents = 128;

class ModuleNavigation : Module
{
public:

	ModuleNavigation(bool start_enabled = true);
	virtual ~ModuleNavigation();
	bool Init(JSON_Object* jObject);
	update_status Update();
	bool CleanUp();

	void OnSystemEvent(System_Event e);

	void Draw() const;
	void SetInputGeom(class NMInputGeom& inputGeom);
	int  AddAgent(const float* p, float radius, float height, float maxAcc, float maxSpeed,
				  float collQueryRange, float pathOptimRange, unsigned char updateFlags,
				  unsigned char obstacleAvoidanceType) const;
	bool UpdateAgentParams(int indx, float radius, float height, float maxAcc, float maxSpeed,
						   float collQueryRange, float pathOptimRange, unsigned char updateFlags,
						   unsigned char obstacleAvoidanceType) const;
	void RemoveAgent(int indx) const;
	void SetDestination(const float* p, int indx) const;

	static void calcVel(float* vel, const float* pos, const float* tgt, const float speed);

	bool HandleBuild();

private:

	void cleanup();
	void InitCrowd();

protected:

	class dtNavMesh* m_navMesh = 0;
	class dtNavMeshQuery* m_navQuery = 0;
	class dtCrowd* m_crowd = 0;

	NMBuildContext* m_ctx = 0;

	// Navmesh Cration
	class NMInputGeom* m_geom = 0;
	unsigned char* m_triareas = 0;
	rcHeightfield* m_solid = 0;
	rcCompactHeightfield* m_chf = 0;
	rcContourSet* m_cset = 0;
	rcPolyMesh* m_pmesh = 0;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh = 0;
	//

};

#endif