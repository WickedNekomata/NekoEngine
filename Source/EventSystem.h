#ifndef __EVENT_SYSTEM_H__
#define __EVENT_SYSTEM_H__

enum System_Event_Type { NoEvent, FileDropped, NewFile, FileRemoved, MetaRemoved, FileOverwritten, ReimportFile };

struct FileEvent
{
	System_Event_Type type;
	const char* file;
	const char* metaFile;
};

union System_Event
{
	System_Event_Type type;      /**< Event type, shared with all events */
	FileEvent fileEvent;             /**< Drag and drop event data */

	/* This is necessary for ABI compatibility between Visual C++ and GCC
	   Visual C++ will respect the push pack pragma and use 52 bytes for
	   this structure, and GCC will use the alignment of the largest datatype
	   within the union, which is 8 bytes.

	   So... we'll add padding to force the size to be 56 bytes for both.

	   padding?
	*/
};

#endif
