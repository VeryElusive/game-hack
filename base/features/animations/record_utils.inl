#pragma once
#include "../rage/exploits.h"
#include "animation.h"

inline void PlayerEntry::Clear( ) {
	this->m_flPreviousYaws.clear( );
	this->m_pRecords.clear( );
}

inline void PlayerEntry::OnPlayerChange( CBasePlayer* player ) {
	OnNewRound( );

	const auto playerInfo{ Interfaces::Engine->GetPlayerInfo( player->Index( ) ) };
	if ( playerInfo.has_value( ) )
		this->m_bBot = playerInfo->bFakePlayer;

	this->m_pPlayer = player;
	this->m_flSpawnTime = 0;
	this->m_iResolverSide = 0;
}

inline void PlayerEntry::OnNewRound( ) {
	if ( !this->m_pPlayer )
		return;

	OutOfDormancy( );

	this->m_iMissedShots = 0;
	//if ( !this->m_iResolverSide )
		this->m_iResolverSide = 0;

	this->m_flSpawnTime = this->m_pPlayer->m_flSpawnTime( );
}

inline void PlayerEntry::OutOfDormancy( ) {
	this->m_optPreviousData.reset( );
	this->m_pRecords.clear( );
	this->m_flPreviousYaws.clear( );
	// idc about matrix itll just get updated anyway before render

	this->m_iLastNewCmds = 0;
	this->m_bBrokeLC = false;
	this->m_vecUpdatedOrigin = { 0, 0, 0 };
}

inline float ClampCycle( float cycleIn ) {
	cycleIn -= int( cycleIn );

	if ( cycleIn < 0 )
		cycleIn += 1;
	else if ( cycleIn > 1 )
		cycleIn -= 1;

	return cycleIn;
}


inline void LagRecord_t::FinalAdjustments( CBasePlayer* player, const std::optional<AnimData_t>& previous ) {
	if ( !previous.has_value( ) ) {
		this->m_iNewCmds = 1;
		return;
	}

	const auto& prevLayers{ previous->m_pLayers };
	const auto& curLayers{ this->m_cAnimData.m_pLayers };
	auto& curVel{ this->m_cAnimData.m_vecVelocity };


	// TODO: m_flLastUpdateIncrement on first run
	//const auto fakeCmdCount{ Interfaces::ClientState->iServerTick - TIME_TO_TICKS( player->m_flSimulationTime( ) ) };

	/* choked fix */
	if ( curLayers[ 11 ].flPlaybackRate > 0.f
		&& prevLayers[ 11 ].flPlaybackRate > 0.f
		&& prevLayers[ 11 ].nSequence == curLayers[ 11 ].nSequence ) {

		this->m_iNewCmds = 0;

		auto simCycle{ prevLayers[ 11 ].flCycle };
		auto targetCycle{ curLayers[ 11 ].flCycle };
		auto simPlayBackRate{ prevLayers[ 11 ].flPlaybackRate * Interfaces::Globals->flIntervalPerTick };

		if ( curLayers[ 11 ].flPlaybackRate != simCycle ) {
			if ( simCycle > targetCycle )
				targetCycle += 1.f;
		}

		do {
			++this->m_iNewCmds;

			// IsLayerSequenceCompleted takes old playback into account
			if ( simCycle + simPlayBackRate >= 1.f )
				simPlayBackRate = curLayers[ 11 ].flPlaybackRate * Interfaces::Globals->flIntervalPerTick;

			simCycle += simPlayBackRate;
		} while ( simCycle < targetCycle );

		//if ( simCycle > curLayers[ 11 ].flCycle 
		//	&& simCycle - curLayers[ 11 ].flCycle > simPlayBackRate * 0.5f )
		//	--this->m_iNewCmds;
	}

	/* jump_fall fix */
	if ( curLayers[ 4 ].flCycle > 0.f && curLayers[ 4 ].flCycle < 0.99f && !( this->m_cAnimData.m_iFlags & FL_ONGROUND ) ) {
		this->m_flLeftGroundTime = this->m_cAnimData.m_flSimulationTime - TICKS_TO_TIME( curLayers[ 4 ].flCycle / ( Interfaces::Globals->flIntervalPerTick * curLayers[ 4 ].flPlaybackRate ) );
		if ( !( this->m_cAnimData.m_iFlags & FL_ONGROUND ) )
			this->m_bFixJumpFall = this->m_flLeftGroundTime >= this->m_cAnimData.m_flSimulationTime - TICKS_TO_TIME( this->m_iNewCmds );
	}

	auto chokedTime{ TICKS_TO_TIME( this->m_iNewCmds ) };

	/* velo fix */
	if ( this->m_iNewCmds > 0
		&& this->m_iNewCmds < 16 )
		curVel = ( this->m_cAnimData.m_vecOrigin - previous->m_vecOrigin ) / chokedTime;

	auto maxCurrentSpeed{ player->m_flMaxSpeed( ) };

	if ( this->m_cAnimData.m_iFlags & FL_ONGROUND
		&& previous->m_iFlags & FL_ONGROUND ) {
		while ( curVel.Length( ) > maxCurrentSpeed ) {
			++this->m_iNewCmds;
			chokedTime = TICKS_TO_TIME( this->m_iNewCmds );
			curVel = ( this->m_cAnimData.m_vecOrigin - previous->m_vecOrigin ) / chokedTime;
		}
	}

	// fix shuffling animations due to rounding errors
	if ( std::abs( curVel.x ) < 0.001f )
		curVel.x = 0.f;
	if ( std::abs( curVel.y ) < 0.001f )
		curVel.y = 0.f;
	if ( std::abs( curVel.z ) < 0.001f )
		curVel.z = 0.f;

	if ( curLayers[ 6 ].flPlaybackRate == 0.f )
		curVel = { 0, 0, 0 };
	else if ( this->m_cAnimData.m_iFlags & FL_ONGROUND ) {
		curVel.z = 0.f;

		const auto avgSpeed{ curVel.Length( ) };

		if ( avgSpeed > 0.f ) {
			if ( prevLayers[ 11 ].nSequence == curLayers[ 11 ].nSequence
				&& curLayers[ 11 ].flPlaybackRate == prevLayers[ 11 ].flPlaybackRate
				&& !player->m_flDuckAmount( )
				&& !player->m_bIsWalking( ) ) {

				if ( curLayers[ 11 ].flWeight > 0.f && curLayers[ 11 ].flWeight < 1.f ) {
					//const auto cval{ -( curLayers[ 11 ].flWeight - 1.f ) };
					//const auto m_flSpeedAsPortionOfRunTopSpeed{ 0.55f + 0.35f * cval };

					const auto m_flSpeedAsPortionOfRunTopSpeed{ 0.55f + ( 1.f - curLayers[ 11 ].flWeight ) * 0.35f };
					const auto m_flVelocityLengthXY{ m_flSpeedAsPortionOfRunTopSpeed * maxCurrentSpeed };

					if ( m_flSpeedAsPortionOfRunTopSpeed > 0.f && m_flSpeedAsPortionOfRunTopSpeed < 1.f
						/* && avgSpeed > m_flVelocityLengthXY */ ) {
						this->m_bAccurateVelocity = true;
						curVel.x *= ( m_flVelocityLengthXY / avgSpeed );
						curVel.y *= ( m_flVelocityLengthXY / avgSpeed );
					}
				}
			}

			/*if ( !this->m_bAccurateVelocity ) {
				const auto curDir{ RAD2DEG( std::atan2f( curVel.y, curVel.x ) ) };
				const auto prevDir{ RAD2DEG( std::atan2f( previous->m_vecVelocity.y, previous->m_vecVelocity.x ) ) };
				const auto delta{ Math::AngleDiff( curDir, prevDir ) };

				// these requirements pass only when movement weight can be converted to velocity
				if ( curVel.Length2D( ) >= 0.1f
					&& std::abs( delta ) < 1.f
					&& std::abs( this->m_cAnimData.m_flDuckAmount - previous->m_flDuckAmount ) <= 0.001f
					&& curLayers[ 6 ].flPlaybackRate > prevLayers[ 6 ].flPlaybackRate
					&& curLayers[ 6 ].flWeight > prevLayers[ 6 ].flWeight ) {

					if ( curLayers[ 6 ].flWeight <= 0.7f && curLayers[ 6 ].flWeight > 0.0f ) {
						if ( this->m_cAnimData.m_iFlags & FL_DUCKING || this->m_cAnimData.m_iFlags & FL_ANIMDUCKING )
							maxCurrentSpeed *= 0.34f;

						curVel.x = ( curVel.x / avgSpeed ) * ( curLayers[ 6 ].flWeight * maxCurrentSpeed );
						curVel.y = ( curVel.y / avgSpeed ) * ( curLayers[ 6 ].flWeight * maxCurrentSpeed );
						this->m_bAccurateVelocity = true;
					}
				}
			}*/
		}
	}
}
inline uint8_t LagRecord_t::Validity( ) {
	int delay{ };
	if ( ctx.m_bFakeDucking )
		delay = 15 - Interfaces::ClientState->nChokedCommands;

	const auto serverCurtime{ TICKS_TO_TIME( Interfaces::ClientState->iServerTick + ctx.m_iRealOutLatencyTicks + 1 ) };
	const auto flDeadtime{ static_cast< int >( serverCurtime - Displacement::Cvars.sv_maxunlag->GetFloat( ) ) };

	if ( this->m_cAnimData.m_flSimulationTime < flDeadtime )
		return 0;

	const auto curtime{ ctx.m_flFixedCurtime };

	const auto correct{ std::clamp( ctx.m_flOutLatency + ctx.m_flLerpTime, 0.f, Displacement::Cvars.sv_maxunlag->GetFloat( ) ) };
	auto delta{ correct - ( curtime - this->m_cAnimData.m_flSimulationTime ) };
	// pred not right
	if ( std::fabsf( delta ) > 0.19f
		/* && std::abs( ( curtime - correct ) - this->m_cAnimData.m_flSimulationTime ) >= Interfaces::Globals->flIntervalPerTick*/ )
		return 0;

	auto ret{ 1 };

	for ( int i{ 1 }; i <= 14; ++i ) {
		bool invalid{ };
		if ( std::fabsf( correct - ( ( curtime + TICKS_TO_TIME( i ) ) - this->m_cAnimData.m_flSimulationTime ) ) <= 0.19f )
			ret++;
		else
			invalid = true;

		if ( std::fabsf( correct - ( ( curtime - TICKS_TO_TIME( i ) ) - this->m_cAnimData.m_flSimulationTime ) ) <= 0.19f )
			ret++;
		else
			invalid = true;

		if ( invalid )
			break;
	}

	return ret;
}

#define SWAP_RESIK_SIDE( side ) ( side == 2 ? 1 : 2 )
/*FORCEINLINE void PlayerEntry::GetSide( ) {
	if ( this->m_iMissedShots && !this->m_iFirstResolverSide )
		this->m_iFirstResolverSide = this->m_iLastResolvedSide ? this->m_iLastResolvedSide : 2;


	switch ( this->m_iMissedShots % 5 ) {
	case 0: { // normal angle
		if ( this->m_iFirstResolverSide )
			this->m_iResolverSide = this->m_iFirstResolverSide;
		else if ( !this->m_iResolverSide )
			this->m_iResolverSide = this->m_iLastResolvedSide ? this->m_iLastResolvedSide : 2;

		break;
	}
	case 1: { // opposite
		this->m_iResolverSide = SWAP_RESIK_SIDE( this->m_iFirstResolverSide );
		break;
	}
	case 2: { // half normal
		this->m_iResolverSide = this->m_iFirstResolverSide;
		break;
	}
	case 3: { // half opposite
		this->m_iResolverSide = SWAP_RESIK_SIDE( this->m_iFirstResolverSide );
		break;
	}
	case 4: { // no
		break;
	}
	default: { // no
		break;
	}
	}
}*/

FORCEINLINE void LagRecord_t::Apply( CBasePlayer* ent, int side ) {
	//ent->m_vecOrigin( ) = this->m_cAnimData.m_vecOrigin;
	//ent->m_iEFlags( ) |= EFL_DIRTY_ABSTRANSFORM;

	/*ent->m_vecMins( ) = this->m_cAnimData.m_vecMins;
	ent->m_vecMaxs( ) = this->m_cAnimData.m_vecMaxs;

	ent->GetCollideable( )->OBBMins( ) = this->m_cAnimData.m_vecMins;
	ent->GetCollideable( )->OBBMaxs( ) = this->m_cAnimData.m_vecMaxs;

	if ( ent->m_vecMins( ) != this->m_cAnimData.m_vecMins
		|| ent->m_vecMaxs( ) != this->m_cAnimData.m_vecMaxs ) {
		// rebuild: server.dll/client.dll @ 55 8B EC 8B 45 10 F3 0F 10 81
		ent->m_flNewBoundsMaxs( ) = ent->m_flUnknownVar( ) + ent->m_vecMaxs( ).z;
		ent->m_flNewBoundsTime( ) = ctx.m_flFixedCurtime;
	}*/

	ent->SetCollisionBounds(
		this->m_cAnimData.m_vecMins, this->m_cAnimData.m_vecMaxs
	);

	//ent->m_iEFlags( ) |= EFL_DIRTY_ABSTRANSFORM;
	ent->SetAbsOrigin( this->m_cAnimData.m_vecOrigin );

	std::memcpy( ent->m_CachedBoneData( ).Base( ), this->m_cAnimData.m_arrSides.at( side ).m_pMatrix, ent->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

	//ent->SetAbsAngles( { 0.f, this->m_cAnimData.m_arrSides.at( side ).m_flAbsYaw, 0.f } );// we dont need this so remove when i cleanup
}

FORCEINLINE void LagBackup_t::Apply( CBasePlayer* ent ) {
	/*ent->m_vecMins( ) = this->m_vecMins;
	ent->m_vecMaxs( ) = this->m_vecMaxs;

	ent->GetCollideable( )->OBBMins( ) = this->m_vecMins;
	ent->GetCollideable( )->OBBMaxs( ) = this->m_vecMaxs;*/

	ent->SetCollisionBounds(
		this->m_vecMins, this->m_vecMaxs
	);

	ent->m_flNewBoundsTime( ) = this->m_flNewBoundsTime;
	ent->m_flNewBoundsMaxs( ) = this->m_flNewBoundsMaxs;

	//ent->m_vecOrigin( ) = this->m_vecOrigin;
	//ent->m_iEFlags( ) |= EFL_DIRTY_ABSTRANSFORM;
	ent->SetAbsOrigin( this->m_vecAbsOrigin );

	//ent->SetAbsAngles( { 0.f, this->m_flAbsYaw, 0.f } );

	std::memcpy( ent->m_CachedBoneData( ).Base( ), this->m_matMatrix, ent->m_iBoneCount( ) * sizeof( matrix3x4_t ) );
}

FORCEINLINE LagBackup_t::LagBackup_t( CBasePlayer* ent ) {
	//this->m_vecOrigin = ent->m_vecOrigin( );
	this->m_vecAbsOrigin = ent->GetAbsOrigin( );

	this->m_vecMins = ent->m_vecMins( );
	this->m_vecMaxs = ent->m_vecMaxs( );

	this->m_flNewBoundsTime = ent->m_flNewBoundsTime( );
	this->m_flNewBoundsMaxs = ent->m_flNewBoundsMaxs( );

	memcpy( this->m_matMatrix, ent->m_CachedBoneData( ).Base( ), ent->m_iBoneCount( ) * sizeof( matrix3x4_t ) );
}