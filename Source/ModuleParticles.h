#ifndef __MouduleParticle_H__
#define __MouduleParticle_H__

#define MAX_PARTICLES 20000

#include "Module.h"
#include "Application.h"

#include <queue>
#include "Particle.h"

#include "ComponentEmitter.h"

class ModuleParticle : public Module
{
public:
	ModuleParticle(bool start_enabled = true);
	~ModuleParticle();

	update_status Update();

	void StartAllEmiters();

	void Draw();
	void DrawParticles();
	void SortParticles();
	//void SortParticles();
	bool GetParticle(int& id);
	void ClearEmitters();

	void OnSystemEvent(System_Event event);

	void RemoveEmitter(ComponentEmitter* emitter);
public:

	std::list<ComponentEmitter*> emitters;

	Particle allParticles[MAX_PARTICLES];

	//ParticleUV particleAnimation;


	ParticlePlane* plane = nullptr;

	int activeParticles = 0;
	ComponentEmitter* firework = nullptr;
private:
	
	std::priority_queue<Particle*, std::vector<Particle*>,particleCompare> partQueue;

	std::vector<Particle*> partVec;

	int lastUsedParticle = 0;
};

#endif // !__MouduleParticle_H__
