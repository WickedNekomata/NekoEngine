#include "NMBuildContext.h"
#include "Globals.h"

NMBuildContext::NMBuildContext()
{
}

void NMBuildContext::doResetLog()
{
}

void NMBuildContext::doLog(const rcLogCategory category, const char* msg, const int len)
{
	switch (category)
	{
	case RC_LOG_PROGRESS:
		DEPRECATED_LOG("RC Progress: %s", msg);
		break;
	case RC_LOG_WARNING:
		DEPRECATED_LOG("RC Warning: %s", msg);
		break;
	case RC_LOG_ERROR:
		DEPRECATED_LOG("RC Error: %s", msg);
		break;
	}
}

void NMBuildContext::doResetTimers()
{
}

void NMBuildContext::doStartTimer(const rcTimerLabel label)
{
}

void NMBuildContext::doStopTimer(const rcTimerLabel label)
{
}

int NMBuildContext::doGetAccumulatedTime(const rcTimerLabel label) const
{
	return 0;
}
