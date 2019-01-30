#ifndef __BUILD_CONTEXT_H__
#define __BUILD_CONTEXT_H__

#include "Recast&Detour/Recast/Include/Recast.h"
#include "PerfTimer.h"

class BuildContext : public rcContext
{

public:
	BuildContext();

protected:
	/// Virtual functions for custom implementations.
	///@{
	virtual void doResetLog();
	virtual void doLog(const rcLogCategory category, const char* msg, const int len);
	virtual void doResetTimers();
	virtual void doStartTimer(const rcTimerLabel label);
	virtual void doStopTimer(const rcTimerLabel label);
	virtual int doGetAccumulatedTime(const rcTimerLabel label) const;
	///@}
};


#endif