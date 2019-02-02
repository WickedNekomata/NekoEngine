#ifndef __Emitter_H__
#define __Emitter_H__

#include "Component.h"
#include "GameObject.h"

#include "Timer.h"
#include "GameTimer.h"

#include "Particle.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Geometry/Sphere.h"
#include "MathGeoLib/include/Geometry/Circle.h"

#include "ImGui\imgui.h"

#include <list>
#include <queue>
enum ShapeType {
	ShapeType_BOX,
	ShapeType_SPHERE,
	ShapeType_SPHERE_CENTER,
	ShapeType_SPHERE_BORDER,
	ShapeType_CONE
};

enum SimulatedGame
{
	SimulatedGame_NONE,
	SimulatedGame_PLAY,
	SimulatedGame_PAUSE,
	SimulatedGame_STOP,
};
struct ColorTime
{
	math::float4 color = math::float4::one;
	float position = 0.0f;
	std::string name = " ";
	//open window for change particle color
	bool changingColor = false;

	bool operator<(const ColorTime &color) const
	{
		return position < color.position;
	}
};

struct StartValues
{
	// Start values
	math::float2 life = math::float2(5.0f, 5.0f);
	math::float2 speed = math::float2(3.0f, 3.0f);
	math::float2 acceleration = math::float2(0.0f, 0.0f);
	math::float2 sizeOverTime = math::float2(0.0f, 0.0f);
	math::float2 size = math::float2(1.0f, 1.0f);
	math::float2 rotation = math::float2(0.0f, 0.0f);
	math::float2 angularAcceleration = math::float2(0.0f, 0.0f);
	math::float2 angularVelocity = math::float2(0.0f, 0.0f);

	std::list<ColorTime> color;
	bool timeColor = false;

	math::float3 particleDirection = math::float3::unitY;

	bool subEmitterActive = false;

	StartValues()
	{
		ColorTime colorTime;
		colorTime.name = "Start Color";
		color.push_back(colorTime);
	}
};

/*
struct EmitterInfo : ComponentInfo
{

	float duration = 1.0f;

	bool loop = true;

	bool burst = false;
	int minPart = 0;
	int maxPart = 10;
	float repeatTime = 1.0f;

	bool drawAABB = false;
	math::float3 posDifAABB = math::float3::zero;
	float gravity = 0.0f;

	math::AABB boxCreation = math::AABB(math::float3(-0.5f, -0.5f, -0.5f), math::float3(0.5f, 0.5f, 0.5f));
	float SphereCreationRad = 1.0f;
	float circleCreationRad = 1.0f;

	ShapeType shapeType = ShapeType_BOX;

	ResourceTexture* texture = nullptr;

	StartValues startValues;

	bool checkLife = false;
	bool checkSpeed = false;
	bool checkAcceleration = false;
	bool checkSize = false;
	bool checkSizeOverTime = false;
	bool checkRotation = false;
	bool checkAngularAcceleration = false;
	bool checkAngularVelocity = false;

	int textureRows = 1;
	int textureColumns = 1;
	float animationSpeed = 0.1f;
	bool isParticleAnimated = false;
	bool dieOnAnimation = false;

	bool isSubEmitter = false;
	GameObject* subEmitter = nullptr;
	uint subEmitterUUID = 0u;

	int rateOverTime = 10;
	bool subEmitterActive = false;

	math::float3 sizeOBB = math::float3::zero;
};
*/
class ComponentEmitter : public Component
{
public:
	ComponentEmitter(GameObject* gameObject);
	//ComponentEmitter(GameObject* gameObject, EmitterInfo* info);
	~ComponentEmitter();

	void StartEmitter();
	void ChangeGameState(SimulatedGame state);

	void Update();

	void OnUniqueEditor();
	//void ParticleTexture();
	void ParticleAABB();
	void ParticleSubEmitter();
	void ParticleBurst();
	void ParticleColor();
	void ParticleValues();
	void ParticleShape();
	//void SetNewAnimation(int row, int col);
	math::float3 RandPos(ShapeType shapeType);
	void ShowFloatValue(math::float2 & value, bool checkBox, const char * name, float v_speed, float v_min, float v_max);
	void EqualsMinMaxValues(math::float2 & value);
	void CheckMinMax(math::float2 & value);
	void ClearEmitter();
	void SoftClearEmitter();
	void CreateParticles(int particlesToCreate, ShapeType shapeType, const math::float3& pos = math::float3::zero);
	bool EditColor(ColorTime & colorTime, uint pos = 0u);

#ifndef GAMEMODE
	ImVec4 EqualsFloat4(const math::float4 float4D);
#endif

	void SaveComponent(JSON_Object * parent);

	int GetEmition() const;
public:
	GameTimer timer;
	GameTimer burstTime;

	// Particle texture
	ResourceTexture* texture = nullptr;

	bool drawAABB = false;

	// Emitter particles
	std::list<Particle*> particles;

	bool emitterActive = true;

	SimulatedGame simulatedGame = SimulatedGame_NONE;
	GameTimer timeSimulating;

	bool dieOnAnimation = false;

	GameObject* subEmitter = nullptr;
	uint subEmitterUUID = 0u;
	ShapeType normalShapeType = ShapeType_BOX;

	std::list<math::float3> newPositions;

	StartValues startValues;

	bool isSubEmitter = false;
private:
	// General info
	//---------------------------------------
	// Duration of the particle emitter
	float duration = 1.0f;

	//Check box Randomize values
	bool checkLife = false;
	bool checkSpeed = false;
	bool checkAcceleration = false;
	bool checkSizeOverTime = false;
	bool checkSize = false;
	bool checkRotation = false;
	bool checkAngularAcceleration = false;
	bool checkAngularVelocity = false;

	// Loop the particle (if true the particle emitter will never stop)
	bool loop = true;
	GameTimer loopTimer;
	// Warm up the particle emitter (if true the particle emitter will be already started at play-time)
	bool preWarm = true;

	//Create other particle when he death

	//Burst options
	bool burst = false;
	int minPart = 0;
	int maxPart = 10;
	float repeatTime = 1.0f;

	math::float3 posDifAABB = math::float3::zero;
	float gravity = 0.0f;

	//Posibility space where particle is created
	math::AABB boxCreation = math::AABB(math::float3(-0.5f, -0.5f, -0.5f), math::float3(0.5f, 0.5f, 0.5f));
	math::Sphere sphereCreation = math::Sphere(math::float3::zero, 1.0f);
	math::Circle circleCreation = math::Circle(math::float3::unitY, math::float3::unitY, 1.0f);

	ShapeType burstType = ShapeType_BOX;
	std::string burstTypeName = "Box Burst";

	int nextPos = 100;
	math::float4 nextColor = math::float4(0.0f, 0.0f, 0.0f, 1.0f);

	//---------------------------------------

	// Emission info
	//---------------------------------------
	// Number of particles created per second
	int rateOverTime = 10;
	float timeToParticle = 0.0f;
	//---------------------------------------

	//ParticleUV particleAnimation;
	float animationSpeed = 0.1f;

	int textureRows = 1;
	int textureColumns = 1;

	bool isParticleAnimated = false;
};
#endif // !__Emitter_H__