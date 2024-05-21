#pragma once
#include "../context.h"
#include "../features/visuals/visuals.h"
#include "../features/misc/shots.h"

class CEventListener : public IGameEventListener2
{
public:
	void Setup( const std::deque<const char*>& arrEvents );
	void Destroy( );

	virtual void FireGameEvent( IGameEvent* pEvent ) override;
	virtual int GetEventDebugID( ) override
	{
		return EVENT_DEBUG_ID_INIT;
	}
};

inline CEventListener EventListener;