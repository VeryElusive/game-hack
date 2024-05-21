#pragma once
#include "../../context.h"
#include "../../utils/math.h"
#include "../../core/config.h"

class CMisc {
public:
	void Thirdperson( );
	void Movement( CUserCmd& cmd );
	void MoveMINTFix( CUserCmd& cmd, QAngle wish_angles, int flags, int move_type );
	void NormalizeMovement( CUserCmd& cmd );
	void AutoPeek( CUserCmd& cmd );
	void AutoStop( CUserCmd& cmd );
	bool InPeek( CUserCmd& cmd );
	bool IsDefensivePositionHittable( );
	void SlowWalk( CUserCmd& cmd );
	void AutoStrafer( CUserCmd& cmd );
	bool MicroMove( CUserCmd& cmd );
	void LimitSpeed( CUserCmd& cmd, float speed, CBasePlayer* player = nullptr );

	// FAKE PING
	void UpdateIncomingSequences( INetChannel* pNetChannel );
	void ClearIncomingSequences( );
	void AddLatencyToNetChannel( INetChannel* pNetChannel, float flLatency );

	float TPFrac{ };
	Vector OldOrigin{ };
	bool AutoPeeking{ };

	Vector2D m_ve2OldMovement{ };
	Vector2D m_ve2SubAutostopMovement{ };
	bool m_bWasJumping{ };
	std::deque<SequenceObject_t> m_vecSequences = { };
private:
	QAngle MovementAngle{ };

	float OldYaw{ };

	bool ShouldRetract{ };

	void QuickStop( CUserCmd& cmd );
	void FakeDuck( CUserCmd& cmd );
	void Stop( CUserCmd& cmd );

	// FAKE PING
	/* our real incoming sequences count */
	int m_nRealIncomingSequence = 0;
	/* count of incoming sequences what we can spike */
	int m_nLastIncomingSequence = 0;
};

namespace Features { inline CMisc Misc; };