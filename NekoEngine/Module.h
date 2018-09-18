// ----------------------------------------------------
// Module
// Interface for all engine modules
// ----------------------------------------------------

#ifndef __MODULE_H__
#define __MODULE_H__

class Application;
struct PhysBody3D;

class Module
{
public:

	Module(Application* parent, bool start_enabled = true) : App(parent)
	{}

	virtual ~Module()
	{}

	// Called before render is available
	virtual bool Init() 
	{
		return true; 
	}

	// Called before the first frame
	virtual bool Start()
	{
		return true;
	}

	// Called each loop iteration
	virtual update_status PreUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	// Called each loop iteration
	virtual update_status Update(float dt)
	{
		return UPDATE_CONTINUE;
	}

	// Called each loop iteration
	virtual update_status PostUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	// Called before quitting
	virtual bool CleanUp() 
	{ 
		return true; 
	}

	// Collision
	virtual void OnCollision(PhysBody3D* body1, PhysBody3D* body2)
	{}

public:

	Application* App;

private:

	bool enabled;
};

#endif