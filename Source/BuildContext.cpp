#include "BuildContext.h"
#include "Globals.h"

BuildContext::BuildContext()
{
}

void BuildContext::doResetLog()
{
}

void BuildContext::doLog(const rcLogCategory category, const char* msg, const int len)
{
	switch (category)
	{
	case RC_LOG_PROGRESS:
		CONSOLE_LOG("RC Progress: %s", msg);
		break;
	case RC_LOG_WARNING:
		CONSOLE_LOG("RC Warning: %s", msg);
		break;
	case RC_LOG_ERROR:
		CONSOLE_LOG("RC Error: %s", msg);
		break;
	}
}

void BuildContext::doResetTimers()
{
}

void BuildContext::doStartTimer(const rcTimerLabel label)
{
}

void BuildContext::doStopTimer(const rcTimerLabel label)
{
}

int BuildContext::doGetAccumulatedTime(const rcTimerLabel label) const
{
	return 0;
}
