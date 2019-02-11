#ifndef __PHYSICS_CONSTANTS_H__
#define __PHYSICS_CONSTANTS_H__

#include "Globals.h"

namespace PhysicsConstants
{
	constexpr float FIXED_DT(1.0f / 60.0f);
	constexpr float GRAVITY_X(0.0f);
	constexpr float GRAVITY_Y(-9.8f);
	constexpr float GRAVITY_Z(0.0f);
	constexpr float STATIC_FRICTION(0.5f);
	constexpr float DYNAMIC_FRICTION(0.5f);
	constexpr float RESTITUTION(0.6f);
	constexpr float DENSITY(10.0f);
	constexpr float GEOMETRY_HALF_SIZE(0.5f);
	constexpr uint MAX_HITS(256);
}

#endif