#ifndef __GAME_TIMER_H__
#define __GAME_TIMER_H__

#include "Globals.h"
#include "SDL\include\SDL.h"

class GameTimer
{
public:

	// Constructor
	GameTimer();
	~GameTimer();

	void Update(float dt);

	void Start();
	void Continue();
	void Stop();
	void Pause();


	Uint32 Read() const;
	float ReadSec() const;

private:

	bool	running = false;
	float	time = 0.0f;

};

#endif //__Game_TIMER_H__