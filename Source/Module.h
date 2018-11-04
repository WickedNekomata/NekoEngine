// ----------------------------------------------------
// Module
// Interface for all engine modules
// ----------------------------------------------------

#ifndef __MODULE_H__
#define __MODULE_H__

#include "parson/parson.h"

#include "Globals.h"

class Module
{
public:

	Module(bool start_enabled = true)
	{}

	virtual ~Module()
	{}

	virtual bool Init(JSON_Object* data)
	{
		return true; 
	}

	virtual bool Start()
	{
		return true;
	}

	virtual update_status PreUpdate()
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status Update()
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status PostUpdate()
	{
		return UPDATE_CONTINUE;
	}

	virtual bool CleanUp() 
	{ 
		return true; 
	}

	virtual void SaveStatus(JSON_Object*) const {};

	virtual void LoadStatus(const JSON_Object*) {};

	virtual bool OnGameMode() { return true; };

	virtual bool OnEditorMode() { return true; };

	inline char* GetName() const { return name; }

	bool IsActive() const { return enabled; }

	void SetActive(bool enable) { this->enabled = enable; }

	bool IsGame() const { return game; }

	void SetGame(bool game) { this->game = game; }

protected:

	char* name = "NoName";

	bool game = false;

private:

	bool enabled;
};

#endif