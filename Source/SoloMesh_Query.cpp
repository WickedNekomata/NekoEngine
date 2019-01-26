#include "SoloMesh_Query.h"
#include "InputGeom.h"

#include "Globals.h"

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

	return true;
}
