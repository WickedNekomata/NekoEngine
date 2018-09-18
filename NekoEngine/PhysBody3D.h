#pragma once

#include <list>

class btRigidBody;
class Module;

struct PhysBody3D
{
	friend class ModulePhysics3D;

public:

	PhysBody3D(btRigidBody* body);
	~PhysBody3D();

	void Push(float x, float y, float z);
	void GetTransform(float* matrix) const;
	void SetTransform(const float* matrix) const;
	void SetPos(float x, float y, float z);
	void SetAsSensor(bool is_sensor);
	bool IsSensor() const;

public:

	std::list<Module*> collision_listeners;

private:

	btRigidBody* body = nullptr;
	bool is_sensor = false;
};