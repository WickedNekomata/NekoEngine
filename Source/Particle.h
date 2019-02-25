#ifndef __Particle_H__
#define __Particle_H__

#include"MathGeoLib/include/Math/float4.h"
#include"MathGeoLib/include/Math/float2.h"
#include"MathGeoLib/include/Math/Quat.h"

class ComponentEmitter;

struct ParticleAnimation;
struct StartValues;
struct ColorTime;

struct ParticleTrans
{
	math::float3 position = math::float3::zero;
	math::Quat rotation = math::Quat::identity;
	math::float3 scale = math::float3::one;

	math::float4x4 GetMatrix() const;
};

class Particle
{
public:
	Particle(math::float3 pos, StartValues data);
	Particle();
	~Particle();

	bool operator<(const Particle& particle2) const
	{
		return camDistance < particle2.camDistance;
	}

	void SetActive(math::float3 pos, StartValues data, ParticleAnimation animPart);

	bool Update(float dt);

	void EndParticle();

	void LookAtCamera();

	float GetCamDistance() const;
	void SetCamDistance();
	void Draw();

	float CreateRandomNum(math::float2 edges);

	void ChangeAnim(ParticleAnimation partAnim);

public:
	float camDistance = 0.0f;
	bool active = false;

	ComponentEmitter* owner = nullptr;

private:
	float lifeTime = 0.0f;
	float life = 0.0f;

	float speed = 0.0f;
	math::float3 acceleration3 = math::float3::zero;
	math::float3 direction = math::float3::zero;

	float rotationSpeed = 0.0f;
	float angularVelocity = 0.0f;
	float angularAcceleration = 0.0f;
	float sizeOverTime = 0.0f;

	bool subEmitterActive = false;

	std::vector<ColorTime> color;
	int index = 0;
	bool multicolor = false;

	math::float4 currentColor = math::float4::one;

	ParticleTrans transform;

	float rotation = 0.0f;
	float angle = 0.0f;

	//ParticleAnimation particleAnim;

	bool isParticleAnimated = false;
	int  textureRows = 1;
	int  textureColumns = 1;
	float textureRowsNorm = 1.0f;
	float textureColumnsNorm = 1.0f;
	float animationSpeed = 0.1f;

	float animationTime = 0.0f;
	uint currentFrame = 0;
	math::float2 currMinUVCoord = math::float2::zero;
};

struct particleCompare
{
	bool operator()(const Particle* particle1, const Particle* particle2) const
	{
		return particle1->camDistance > particle2->camDistance;
	}
};
#endif // !__Particle_H__

