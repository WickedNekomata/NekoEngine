#include "ModuleParticles.h"
#include "ModuleTimeManager.h"
#include "ModuleInput.h"

#include "Brofiler/Brofiler.h"
#include <algorithm>
ModuleParticle::ModuleParticle(bool start_enabled) : Module(start_enabled)
{

}

ModuleParticle::~ModuleParticle()
{
	delete plane;
}

update_status ModuleParticle::Update()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		if (firework)
		{
			firework->StartEmitter();
			DEPRECATED_LOG("Module Particle: Firework Comming");
		}
	}


	for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
	{
		(*emitter)->Update();
	}

	int count = 0;

	float dt;
	if(App->IsEditor())
		dt = App->timeManager->GetRealDt();
	else
		dt = App->timeManager->GetDt();

	partVec.resize(activeParticles);
	
	int j = 0;
	for (int i = 0; i < MAX_PARTICLES; ++i)
	{
		if (allParticles[i].active)
		{
			allParticles[i].Update(dt); //Particles can be created here, they sould not be updated yet
			++count;
			allParticles[i].SetCamDistance();
			partVec[j++] = &allParticles[i];
		}
		else
		{
			allParticles[i].camDistance = -1;
		}
	}
	return UPDATE_CONTINUE;
}

void ModuleParticle::StartAllEmiters()
{
	for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
	{
		if((*emitter) != firework)
			(*emitter)->StartEmitter();
	}
}

void ModuleParticle::Draw()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	if (plane == nullptr)
		plane = new ParticlePlane();

	SortParticles();

	DrawParticles();
}

void ModuleParticle::DrawParticles()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	//while (!partQueue.empty())
	//{
	//	Particle* currPart = partQueue.top();

	//	//if (currPart->owner && currPart->owner->gameObject->canDraw)
	//		currPart->Draw();

	//	partQueue.pop();
	//}

	for (int i = 0; i < partVec.size(); ++i)
	{
		//TODO Particles: Need boolean canDraw??
		if (partVec[i]->owner /*&& partVec[i]->owner->GetParent()->canDraw*/)
			partVec[i]->Draw();
	}
}

void ModuleParticle::SortParticles()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	std::sort(partVec.begin(), partVec.end(), particleCompare());
}

bool ModuleParticle::GetParticle(int& id)
{
	for (int i = lastUsedParticle; i < MAX_PARTICLES; ++i) 
	{
		if (!allParticles[i].active) 
		{
			lastUsedParticle = i;
			id = i;
			return true;
		}
	}

	for (int i = 0; i < lastUsedParticle; ++i)
	{
		if (!allParticles[i].active)
		{
			lastUsedParticle = i;
			id = i;
			return true;
		}
	}
	return false;
}

void ModuleParticle::ClearEmitters()
{
	for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
	{
		(*emitter)->SoftClearEmitter();
	}
	emitters.clear();

	for (int i = 0; i < MAX_PARTICLES; ++i)
	{
		allParticles[i].active = false;
		allParticles[i].owner = nullptr;		
	}

	activeParticles = 0;
	lastUsedParticle = 0;
}

void ModuleParticle::RemoveEmitter(ComponentEmitter * emitter)
{
	emitters.remove(emitter);

	if (emitter->isSubEmitter)
	{
		for (std::list<ComponentEmitter*>::iterator iterator = emitters.begin(); iterator != emitters.end(); ++iterator)
		{
			if ((*iterator)->subEmitter == emitter->GetParent())
			{
				(*iterator)->startValues.subEmitterActive = false;
				(*iterator)->subEmitter = nullptr;
			}
		}
	}
}
