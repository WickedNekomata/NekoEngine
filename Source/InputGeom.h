#ifndef __INPUTGEOM_H__
#define __INPUTGEOM_H__

struct BuildSettings
{
	// Dont reorder the elements cause of memcpy at panel navmesh. If required, you would also reorder cs at p_navmesh.

	// Cell size in world units
	float cellSize;
	// Cell height in world units
	float cellHeight;
	// Agent height in world units
	float agentHeight;
	// Agent radius in world units
	float agentRadius;
	// Agent max climb in world units
	float agentMaxClimb;
	// Agent max slope in degrees
	float agentMaxSlope;
	// Region minimum size in voxels.
	// regionMinSize = sqrt(regionMinArea)
	float regionMinSize;
	// Region merge size in voxels.
	// regionMergeSize = sqrt(regionMergeArea)
	float regionMergeSize;
	// Edge max length in world units
	float edgeMaxLen;
	// Edge max error in voxels
	float edgeMaxError;
	float vertsPerPoly;
	// Detail sample distance in voxels
	float detailSampleDist;
	// Detail sample max error in voxel heights.
	float detailSampleMaxError;
	// Partition type, see SamplePartitionType
	int partitionType;
	// Size of the tiles in voxels
	float tileSize;

	inline void Init()
	{
		
	}
};

class InputGeom
{
public:
	BuildSettings m_buildSettings;
	float* m_verts;
	int* m_tris;
	int m_nverts;
	int m_ntris;
	float* bMin[3], bMax[3];
};

#endif