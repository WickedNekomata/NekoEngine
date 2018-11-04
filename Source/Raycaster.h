#ifndef __RAYCASTER_H__
#define __RAYCASTER_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "MathGeoLib/include/Math/float3.h"

class GameObject;

class Raycaster
{
public:

	Raycaster();
	~Raycaster();

	void ScreenPointToRay(int posX, int posY, float& distance, math::float3& hitPoint, GameObject** hit);
};

#endif

#endif
