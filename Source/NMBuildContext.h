#ifndef __NM_BUILD_CONTEXT_H__
#define __NM_BUILD_CONTEXT_H__

#include "Recast&Detour/Recast/Include/Recast.h"
#include "PerfTimer.h"

class NMBuildContext : public rcContext
{

public:
	NMBuildContext();

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