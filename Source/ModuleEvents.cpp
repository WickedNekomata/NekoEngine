#include "ModuleEvents.h"
#include "EventSystem.h"

void ModuleEvents::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
		case System_Event_Type::ComponentDestroyed:
		{
			delete event.compEvent.component;
			break;
		}
		case System_Event_Type::GameObjectDestroyed:
		{
			delete event.goEvent.gameObject;
			break;
		}
		case System_Event_Type::ResourceDestroyed:
		{
			delete event.resEvent.resource;
			break;
		}
	}
}
