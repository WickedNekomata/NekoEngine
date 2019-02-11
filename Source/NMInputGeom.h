#ifndef __NMINPUTGEOM_H__
#define __NMINPUTGEOM_H__

enum RecastPartitionType
{
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};


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
	RecastPartitionType partitionType;
	// Size of the tiles in voxels
	float tileSize;
};

struct M_Mesh
{
	float* m_verts;
	int* m_tris;
	int m_nverts;
	int m_ntris;
	bool walkable;
};

class NMInputGeom
{
public:
	BuildSettings i_buildSettings;
	M_Mesh* i_meshes;
	int i_nmeshes;
	int i_ntris;
	float* bMin[3], bMax[3];
};

#endif