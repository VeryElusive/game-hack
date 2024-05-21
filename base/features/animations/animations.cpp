#include "animation.h"
#include "../../utils/performance_monitor.h"
#include "../../utils/threading/thread_pool.hpp"

// https://www.youtube.com/watch?v=a3Z7zEc7AXQ
void CAnimationSys::RunAnimationSystem( ) {
	static const auto once = [ ]( )
	{
		const auto fn{ reinterpret_cast< int( _cdecl* )( ) >(
			GetProcAddress( GetModuleHandle( _( "tier0.dll" ) ), _( "AllocateThreadID" ) )
			) };

		std::counting_semaphore<> sem{ 0u };

		for ( std::size_t i{ }; i < std::thread::hardware_concurrency( ); ++i )
			sdk::g_thread_pool->enqueue(
				[ ]( decltype( fn ) fn, std::counting_semaphore<>& sem )
				{
					sem.acquire( );
					fn( );
				}, fn, std::ref( sem )
					);

		for ( std::size_t i{ }; i < std::thread::hardware_concurrency( ); ++i )
			sem.release( );

		sdk::g_thread_pool->wait( );

		return true;
	}( );

	const auto serverCurtime{ TICKS_TO_TIME( Interfaces::ClientState->iServerTick + ctx.m_iRealOutLatencyTicks ) };

	const auto flDeadtime{ static_cast< int >( serverCurtime - Displacement::Cvars.sv_maxunlag->GetFloat( ) ) };

	{
		auto updatedPlayers{ new std::vector<PlayerEntry*> };
		CScopedPerformanceMonitor as{ &ctx.m_iAnimsysPerfTimer };

		for ( int i{ 1 }; i < 64; ++i ) {
			const auto player{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
			auto& entry{ m_arrEntries.at( i - 1 ) };

			if ( !player || !player->IsPlayer( ) || player == ctx.m_pLocal ) {
				entry.Clear( );
				continue;
			}

			if ( entry.m_pPlayer != player )
				entry.OnPlayerChange( player );

			if ( player->IsDead( ) )
				continue;

			if ( player->IsDormant( ) ) {
				entry.OutOfDormancy( );
				continue;
			}

			const auto state{ player->m_pAnimState( ) };
			if ( !state )
				continue;

			entry.m_pRecords.erase(
				std::remove_if(
					entry.m_pRecords.begin( ), entry.m_pRecords.end( ),
					[ & ]( const std::shared_ptr< LagRecord_t >& lag_record ) -> bool {
						return lag_record->m_cAnimData.m_flSimulationTime < flDeadtime;
					}
				),
				entry.m_pRecords.end( )
						);

			if ( entry.m_optPreviousData.has_value( )
				&& entry.m_optPreviousData->m_pLayers[ 11 ].flPlaybackRate == player->m_AnimationLayers( )[ 11 ].flPlaybackRate
				&& entry.m_optPreviousData->m_pLayers[ 11 ].flCycle == player->m_AnimationLayers( )[ 11 ].flCycle ) {
				player->m_flSimulationTime( ) = entry.m_optPreviousData->m_flSimulationTime;
				player->m_flOldSimulationTime( ) = entry.m_optPreviousData->m_flOldSimulationTime;
				continue;
			}

			if ( player->m_flSpawnTime( ) != entry.m_flSpawnTime ) {
				state->Reset( );
				entry.OnNewRound( );
			}

			if ( player->m_fFlags( ) & FL_DUCKING )
				entry.m_pPlayer->m_vecViewOffset( ).z = 46.f;
			else
				entry.m_pPlayer->m_vecViewOffset( ).z = 64.f;

			// NOW we add record
			const auto current{ std::make_shared< LagRecord_t >( player ) };

			current->FinalAdjustments( player, entry.m_optPreviousData );

			const bool outOfDormancy{ entry.m_vecUpdatedOrigin.IsZero( ) };

			AnimatePlayer( current.get( ), entry );


			entry.m_vecPreviousVelocity = entry.m_optPreviousData.has_value( ) ? entry.m_optPreviousData->m_vecVelocity : Vector{ 0, 0, 0 };// used in extrapolation
			entry.m_optPreviousData = current->m_cAnimData;

			const auto oldSimTime{ entry.m_pRecords.size( ) ? entry.m_pRecords.back( )->m_cAnimData.m_flSimulationTime : player->m_flOldSimulationTime( ) };

			entry.m_bRecordAdded = ( ( !Config::Get<bool>( Vars.RagebotLagcompensation )
				|| player->m_flSimulationTime( ) > oldSimTime )
				&& !player->IsTeammate( ) );

			if ( player->m_flSimulationTime( ) > entry.m_flHighestSimulationTime )
				entry.m_flHighestSimulationTime = player->m_flSimulationTime( );

			if ( entry.m_bRecordAdded ) {
				entry.m_bBrokeLC = entry.m_pRecords.size( ) && ( player->m_vecOrigin( ) - entry.m_pRecords.back( )->m_cAnimData.m_vecOrigin ).LengthSqr( ) > 4096.f;
				if ( entry.m_bBrokeLC )
					entry.m_pRecords.clear( );

				current->m_bFirst = outOfDormancy;
				//current->m_bBrokeLC = entry.m_bBrokeLC;

				entry.m_iLastRecordInterval = Interfaces::Globals->iTickCount - entry.m_iLastRecordTick;
				entry.m_iLastRecordTick = Interfaces::Globals->iTickCount;

				entry.m_pRecords.push_back( current );
			}

			entry.m_iLastRecievedTick = Interfaces::Globals->iTickCount;
			updatedPlayers->push_back( &entry );
		}

		for ( auto& entry : *updatedPlayers ) {
			sdk::g_thread_pool->enqueue( [ ]( PlayerEntry* entry ) {
				if ( !entry->m_bRecordAdded ) {
					const auto& side{ entry->m_optPreviousData->m_arrSides.at( entry->m_iResolverSide ) };
					entry->m_pPlayer->SetAbsAngles( { 0, side.m_cAnimState.flAbsYaw, 0 } );
					entry->m_pPlayer->m_flPoseParameter( ) = side.m_flPoseParameter;

					std::memcpy( entry->m_pPlayer->m_AnimationLayers( ), entry->m_optPreviousData->m_pLayers, 0x38 * 13 );

					Features::AnimSys.SetupBonesRebuilt( entry->m_pPlayer, entry->m_matMatrix,
						BONE_USED_BY_SERVER, entry->m_optPreviousData->m_flSimulationTime, true );

					std::memcpy( entry->m_pPlayer->m_CachedBoneData( ).Base( ), entry->m_matMatrix, entry->m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4a_t ) );
				}
				else {
					const auto& record{ entry->m_pRecords.back( ) };
					if ( record->m_bMultiMatrix ) {
						for ( int i{ }; i < 3; ++i ) {
							auto& side{ record->m_cAnimData.m_arrSides.at( i ) };

							entry->m_pPlayer->SetAbsAngles( { 0, side.m_cAnimState.flAbsYaw, 0 } );
							entry->m_pPlayer->m_flPoseParameter( ) = side.m_flPoseParameter;
							*entry->m_pPlayer->m_pAnimState( ) = side.m_cAnimState;

							std::memcpy( entry->m_pPlayer->m_AnimationLayers( ), record->m_cAnimData.m_pLayers, 0x38 * 13 );

							Features::AnimSys.SetupBonesRebuilt( entry->m_pPlayer, side.m_pMatrix,
								BONE_USED_BY_SERVER, record->m_cAnimData.m_flSimulationTime, false );
						}
					}
					else {
						auto& side{ record->m_cAnimData.m_arrSides.at( 0 ) };

						entry->m_pPlayer->SetAbsAngles( { 0, side.m_cAnimState.flAbsYaw, 0 } );
						entry->m_pPlayer->m_flPoseParameter( ) = side.m_flPoseParameter;
						*entry->m_pPlayer->m_pAnimState( ) = side.m_cAnimState;

						std::memcpy( entry->m_pPlayer->m_AnimationLayers( ), record->m_cAnimData.m_pLayers, 0x38 * 13 );

						Features::AnimSys.SetupBonesRebuilt( entry->m_pPlayer, side.m_pMatrix,
							BONE_USED_BY_SERVER, record->m_cAnimData.m_flSimulationTime, false );

						for ( int i{ 1 }; i < 3; ++i )
							std::memcpy( record->m_cAnimData.m_arrSides.at( i ).m_pMatrix, side.m_pMatrix, entry->m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4a_t ) );
					}

					std::memcpy( entry->m_matMatrix, record->m_cAnimData.m_arrSides.at( entry->m_iResolverSide ).m_pMatrix, entry->m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4a_t ) );
					entry->m_pPlayer->ClampBonesInBBOX( entry->m_matMatrix, BONE_USED_BY_SERVER );

					std::memcpy( entry->m_pPlayer->m_CachedBoneData( ).Base( ), entry->m_matMatrix, entry->m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4a_t ) );
				}
			}, std::ref( entry ) );

			sdk::g_thread_pool->wait( );
		}

		delete updatedPlayers;
	}
}

void CAnimationSys::AnimatePlayer( LagRecord_t* current, PlayerEntry& entry ) {
	entry.m_bCanExtrapolate = entry.m_iLastNewCmds == current->m_iNewCmds;
	current->m_iNewCmds = std::clamp( current->m_iNewCmds, 1, 64 );

	entry.m_iLastNewCmds = std::min( Interfaces::Globals->iTickCount - entry.m_iLastRecievedTick, current->m_iNewCmds );

	const auto backupState{ *entry.m_pPlayer->m_pAnimState( ) };

	entry.m_pPlayer->m_iEFlags( ) |= EFL_DIRTY_ABSTRANSFORM;
	entry.m_pPlayer->SetAbsOrigin( current->m_cAnimData.m_vecOrigin );

	/*const auto isShooting{ current->m_cAnimData.m_pWeapon
		&& current->m_cAnimData.m_flSimulationTime >= current->m_cAnimData.m_pWeapon->m_fLastShotTime( ) 
		&& current->m_cAnimData.m_pWeapon->m_fLastShotTime( ) > entry.m_pPlayer->m_flOldSimulationTime( ) };*/

	const auto last{ entry.m_flPreviousYaws.size( ) ? entry.m_flPreviousYaws.back( ).m_flYaw : entry.m_pPlayer->m_angEyeAngles( ).y };

	entry.m_flPreviousYaws.push_back(
		{ Interfaces::Globals->iTickCount, entry.m_pPlayer->m_angEyeAngles( ).y, Math::AngleDiff( last, entry.m_pPlayer->m_angEyeAngles( ).y ) }
	);

	entry.m_flPreviousYaws.erase(
		std::remove_if(
			entry.m_flPreviousYaws.begin( ), entry.m_flPreviousYaws.end( ),
			[ & ]( const PreviousYaw_t& yaw ) -> bool {
				return Interfaces::Globals->iTickCount - yaw.m_iTickCount > 192;// 3 seconds
			}
		),
		entry.m_flPreviousYaws.end( )
				);

	if ( entry.m_optPreviousData.has_value( ) && !entry.m_pPlayer->IsTeammate( )
		&& ( entry.m_optPreviousData->m_vecMins != current->m_cAnimData.m_vecMins
			|| entry.m_optPreviousData->m_vecMaxs != current->m_cAnimData.m_vecMaxs ) ) {
		// rebuild: server.dll/client.dll @ 55 8B EC 8B 45 10 F3 0F 10 81
		entry.m_pPlayer->m_flNewBoundsMaxs( ) = entry.m_pPlayer->m_flUnknownVar( ) + entry.m_optPreviousData->m_vecMaxs.z;

		// we can't be certain... but the margin of error is far less when put at middle
		entry.m_pPlayer->m_flNewBoundsTime( ) = entry.m_pPlayer->m_flSimulationTime( ) /* - TICKS_TO_TIME( current->m_iNewCmds / 2 )*/;
	}


	// used in SetUpMovement rebuild
	const auto backupFlash{ entry.m_pPlayer->m_flNextAttack( ) };
	if ( current->m_bFixJumpFall )
		entry.m_pPlayer->m_flNextAttack( ) = current->m_flLeftGroundTime;
	else
		entry.m_pPlayer->m_flNextAttack( ) = 0.f;

	if ( !entry.m_pPlayer->IsTeammate( )
		&& !entry.m_bBot
		&& entry.m_optPreviousData.has_value( )
		/* && current->m_iNewCmds > 1 */ // smooth decay into 0 mat diff
		&& !ctx.m_pLocal->IsDead( ) ) {
		UpdateSide( entry, current, 1 );

		*entry.m_pPlayer->m_pAnimState( ) = backupState;

		UpdateSide( entry, current, 2 );

		*entry.m_pPlayer->m_pAnimState( ) = backupState;

		current->m_bMultiMatrix = true;
	}

	UpdateSide( entry, current, 0 );
	entry.m_pPlayer->m_flNextAttack( ) = backupFlash;

	if ( !current->m_bMultiMatrix ) {
		for ( int i{ 1 }; i < 3; ++i ) {
			current->m_cAnimData.m_arrSides.at( i ) = current->m_cAnimData.m_arrSides.at( 0 );
			current->m_cAnimData.m_arrSides.at( i ).m_bFilled = false;
		}
	}

	if ( Config::Get<bool>( Vars.RagebotResolver )
		&& current->m_bMultiMatrix
		&& !ctx.m_pLocal->IsDead( ) ) {
		entry.Rezik( current );
		if ( current->m_iResolverSide )
			entry.m_iResolverSide = current->m_iResolverSide;
	}
	else {
		if ( !Config::Get<bool>( Vars.RagebotResolver ) )
			entry.m_iResolverSide = 0;
	}

	entry.m_vecUpdatedOrigin = entry.m_pPlayer->GetAbsOrigin( );
}

void CAnimationSys::UpdateSide( PlayerEntry& entry, LagRecord_t* current, int side ) {
	const auto backupCurtime{ Interfaces::Globals->flCurTime };
	const auto backupFrametime{ Interfaces::Globals->flFrameTime };
	const auto backupHLTV{ Interfaces::ClientState->bIsHLTV };

	Interfaces::ClientState->bIsHLTV = true;
	Interfaces::Globals->flFrameTime = Interfaces::Globals->flIntervalPerTick;

	const auto backupYaw{ entry.m_pPlayer->m_angEyeAngles( ).y };

	if ( side == 1 )
		entry.m_pPlayer->m_angEyeAngles( ).y += 120.f;
	else if ( side == 2 )
		entry.m_pPlayer->m_angEyeAngles( ).y -= 120.f;

	entry.m_pPlayer->m_angEyeAngles( ).y = Math::NormalizeEyeAngles( entry.m_pPlayer->m_angEyeAngles( ).y );

	InterpolateFromLastData( entry.m_pPlayer, current, entry.m_optPreviousData, side );

	entry.m_pPlayer->m_angEyeAngles( ).y = backupYaw;

	auto& sideData{ current->m_cAnimData.m_arrSides.at( side ) }; {
		sideData.m_cAnimState = *entry.m_pPlayer->m_pAnimState( );

		sideData.m_flPoseParameter = entry.m_pPlayer->m_flPoseParameter( );
		std::memcpy( sideData.m_pLayers, entry.m_pPlayer->m_AnimationLayers( ), 0x38 * 13 );

		sideData.m_bFilled = true;
	}

	Interfaces::ClientState->bIsHLTV = backupHLTV;

	/*entry.m_pPlayer->SetAbsAngles( { 0, state->flAbsYaw, 0 } );

	std::memcpy( entry.m_pPlayer->m_AnimationLayers( ), current->m_cAnimData.m_pLayers, 0x38 * 13 );

	Features::AnimSys.SetupBonesRebuilt( entry.m_pPlayer, sideData.m_pMatrix,
		BONE_USED_BY_SERVER, current->m_cAnimData.m_flSimulationTime, false );*/

	Interfaces::Globals->flCurTime = backupCurtime;
	Interfaces::Globals->flFrameTime = backupFrametime;
}

void CAnimationSys::InterpolateFromLastData( CBasePlayer* player, LagRecord_t* current, std::optional < AnimData_t >& from, int side ) {
	const auto state{ player->m_pAnimState( ) };
	if ( !from.has_value( ) ) {
		std::memcpy( player->m_AnimationLayers( ), current->m_cAnimData.m_pLayers, 0x38 * 13 );

		state->m_bLanding = false;
		state->flDurationInAir = 0.f;
		state->flLastUpdateTime = current->m_cAnimData.m_flSimulationTime - Interfaces::Globals->flIntervalPerTick;

		const auto& layer6{ current->m_cAnimData.m_pLayers[ 6 ] };
		const auto& layer7{ current->m_cAnimData.m_pLayers[ 7 ] };
		const auto& layer12{ current->m_cAnimData.m_pLayers[ 12 ] };

		state->flFeetCycle = layer6.flCycle;
		if ( current->m_cAnimData.m_iFlags & FL_ONGROUND
			&& !current->m_cAnimData.m_pLayers[ 5 ].flWeight 
			&& layer6.flWeight > 0 && layer6.flWeight < 1 )
			state->flMoveWeight = layer6.flWeight;

		state->iStrafeSequence = layer7.nSequence;
		state->flStrafeWeight = layer7.flWeight;
		state->flStrafeCycle = layer7.flCycle;

		state->flAccelerationWeight = layer12.flWeight;

		Interfaces::Globals->flCurTime = current->m_cAnimData.m_flSimulationTime;

		/*if ( current->m_bLanded.has_value( ) )
			state->flDurationInAir = std::max( Interfaces::Globals->flCurTime - Interfaces::Globals->flIntervalPerTick - current->m_flOnGroundTime, 0.f );

		if ( side == 1 )
			state->flAbsYaw = std::remainderf( player->m_angEyeAngles( ).y + 120.f, 360.f );
		else if ( side == 2 )
			state->flAbsYaw = std::remainderf( player->m_angEyeAngles( ).y - 120.f, 360.f );*/

		if ( current->m_cAnimData.m_pLayers[ 6 ].flPlaybackRate != 0.f
			&& current->m_cAnimData.m_vecVelocity.Length( ) < 1.f )
			current->m_cAnimData.m_vecVelocity = { 1.1f, 0.f, 0.f };

		player->m_vecAbsVelocity( ) = current->m_cAnimData.m_vecVelocity;

		player->m_iEFlags( ) &= ~EFL_DIRTY_ABSVELOCITY;

		if ( state->iLastUpdateFrame == Interfaces::Globals->iFrameCount )
			state->iLastUpdateFrame = Interfaces::Globals->iFrameCount - 1;

		player->m_angEyeAngles( ).y = current->m_angEyeAngles.y;

		player->m_bClientSideAnimation( ) = ctx.m_bUpdatingAnimations = true;
		player->UpdateClientsideAnimations( );
		player->m_bClientSideAnimation( ) = ctx.m_bUpdatingAnimations = false;

		return;
	}

	/* restore */
	if ( from->m_arrSides.at( side ).m_bFilled )
		*state = from->m_arrSides.at( side ).m_cAnimState;

	const auto& layer6{ from->m_pLayers[ 6 ] };
	const auto& layer7{ from->m_pLayers[ 7 ] };
	const auto& layer12{ from->m_pLayers[ 12 ] };

	state->flFeetCycle = layer6.flCycle;
	if ( from->m_iFlags & FL_ONGROUND && current->m_cAnimData.m_iFlags & FL_ONGROUND
		&& !current->m_cAnimData.m_pLayers[ 5 ].flWeight
		&& !from->m_pLayers[ 5 ].flWeight
		&& layer6.flWeight > 0 && layer6.flWeight < 1 )
		state->flMoveWeight = layer6.flWeight;

	state->iStrafeSequence = layer7.nSequence;
	state->flStrafeWeight = layer7.flWeight;
	state->flStrafeCycle = layer7.flCycle;

	state->flAccelerationWeight = layer12.flWeight;

	std::memcpy( player->m_AnimationLayers( ), from->m_pLayers, 0x38 * 13 );

	/* interp */
	const auto& to{ current->m_cAnimData };

	const auto manualStandVel{ current->m_cAnimData.m_pLayers[ 6 ].flPlaybackRate != 0.f
		&& current->m_cAnimData.m_vecVelocity.Length( ) <= 1.f };

	const auto duckAmountDelta{ to.m_flDuckAmount - from->m_flDuckAmount };
	const auto velocityDelta{ to.m_vecVelocity - from->m_vecVelocity };

	bool landed{ };

	Interfaces::Globals->flCurTime = to.m_flSimulationTime - TICKS_TO_TIME( current->m_iNewCmds );

	/*if ( from->m_iFlags & FL_DUCKING )
		player->m_fFlags( ) |= FL_DUCKING;
	else
		player->m_fFlags( ) &= ~FL_DUCKING;*/

	for ( auto i{ 1 }; i <= current->m_iNewCmds; ++i ) {
		// instead of incrementing at the end, like tickbase does in playerruncommand, do it here because SetSimulationTime is called in CBasePlayer::PostThink, which is before tickbase in incremented (in runcommand), meaning simtime is 1 below the final tickbase (aka its the same as when we animated)
		Interfaces::Globals->flCurTime += Interfaces::Globals->flIntervalPerTick;

		const auto lerp{ i / static_cast< float >( current->m_iNewCmds ) };

		player->m_flDuckAmount( ) = from->m_flDuckAmount + duckAmountDelta * lerp;

		/*if ( player->m_flDuckAmount( ) <= 0.75f && player->m_fFlags( ) & FL_DUCKING )
			player->m_fFlags( ) &= ~FL_DUCKING;
		else if ( !( player->m_fFlags( ) & FL_DUCKING ) && player->m_flDuckAmount( ) == 1.f )
			player->m_fFlags( ) |= FL_DUCKING;*/

		if ( manualStandVel )
			player->m_vecAbsVelocity( ) = { ( i & 1 ) ? 1.1f : -1.1f, 0.f, 0.f };
		else
			player->m_vecAbsVelocity( ) = ( from->m_vecVelocity + ( velocityDelta * lerp ) );

		player->m_iEFlags( ) &= ~EFL_DIRTY_ABSVELOCITY;

		if ( i == current->m_iNewCmds )
			player->m_angEyeAngles( ).y = current->m_angEyeAngles.y;

		if ( current->m_bFixJumpFall ) {
			player->m_fFlags( ) &= ~FL_ONGROUND;
			if ( !landed ) {
				if ( TIME_TO_TICKS( Interfaces::Globals->flCurTime ) >= TIME_TO_TICKS( current->m_flLeftGroundTime ) ) {
					player->m_fFlags( ) |= FL_ONGROUND;
					landed = true;
				}
			}
		}

		if ( state->iLastUpdateFrame == Interfaces::Globals->iFrameCount )
			state->iLastUpdateFrame = Interfaces::Globals->iFrameCount - 1;

		player->m_bClientSideAnimation( ) = ctx.m_bUpdatingAnimations = true;
		player->UpdateClientsideAnimations( );
		player->m_bClientSideAnimation( ) = ctx.m_bUpdatingAnimations = false;
	}
}

void PlayerEntry::Rezik( LagRecord_t* current ) {
	const auto& from{ this->m_optPreviousData };
	if ( !from.has_value( ) )
		return;

	if ( current->m_cAnimData.m_iFlags & FL_ONGROUND
		&& from->m_iFlags & FL_ONGROUND ) {
		const auto speed{ current->m_cAnimData.m_vecVelocity.Length2D( ) };

		if ( this->m_pPlayer->m_flLowerBodyYawTarget( ) != this->m_pPlayer->m_angEyeAngles( ).y
			&& !current->m_cAnimData.m_pLayers[ 6 ].flPlaybackRate ) {
			if ( Math::AngleDiff( this->m_pPlayer->m_flLowerBodyYawTarget( ), this->m_pPlayer->m_angEyeAngles( ).y ) > 0.f )
				current->m_iResolverSide = 1;
			else
				current->m_iResolverSide = 2;

			this->m_bLBYResolved = true;

			if ( this->m_bInvertResolverOrientation )
				current->m_iResolverSide = SWAP_RESIK_SIDE( current->m_iResolverSide );
		}

		if ( speed > 1.f ) {
			const auto accelerating{ static_cast< int >( current->m_cAnimData.m_pLayers[ 6 ].flWeight * 1000.f ) == static_cast< int >( from->m_pLayers[ 6 ].flWeight * 1000.f ) };

			if ( ( !accelerating && !current->m_bAccurateVelocity )
				|| static_cast< int >( current->m_cAnimData.m_pLayers[ 12 ].flWeight * 1000.f ) )
				return;

			const auto leftDelta{ static_cast< int >( std::abs( current->m_cAnimData.m_pLayers[ 6 ].flPlaybackRate - current->m_cAnimData.m_arrSides[ 1 ].m_pLayers[ 6 ].flPlaybackRate ) * 1000.f ) };
			const auto rightDelta{ static_cast< int >( std::abs( current->m_cAnimData.m_pLayers[ 6 ].flPlaybackRate - current->m_cAnimData.m_arrSides[ 2 ].m_pLayers[ 6 ].flPlaybackRate ) * 1000.f ) };
			const auto middleDelta{ static_cast< int >( std::abs( current->m_cAnimData.m_pLayers[ 6 ].flPlaybackRate - current->m_cAnimData.m_arrSides[ 0 ].m_pLayers[ 6 ].flPlaybackRate ) * 1000.f ) };

			// handled in check
			//if ( leftDelta == rightDelta )
			//	return;

			if ( middleDelta < leftDelta || rightDelta <= leftDelta || leftDelta ) {
				if ( middleDelta >= rightDelta && leftDelta > rightDelta && !rightDelta ) {
					current->m_iResolverSide = 2;
					this->m_bLBYResolved = false;
				}
			}
			else {
				current->m_iResolverSide = 1;
				this->m_bLBYResolved = false;
			}

			/*auto lowestDelta{ FLT_MAX };
			int find;
			for ( int i{ 2 }; i >= 0; --i ) {
				const auto delta{ std::abs( current->m_cAnimData.m_pLayers[ 6 ].flWeight - current->m_cAnimData.m_arrSides[ i ].m_pLayers[ 6 ].flWeight ) };

				if ( delta < lowestDelta ) {
					lowestDelta = delta;
					find = i;
				}
			}

			if ( find != current->m_iResolverSide )
				Features::Logger.Log( "differed", false );
			else
				Features::Logger.Log( "same", false );*/
		}
	}
	else if ( current->m_iNewCmds <= 1 )
		current->m_iResolverSide = 0;
}

bool CAnimationSys::SetupBonesRebuilt( CBasePlayer* const player, matrix3x4a_t* bones, const int boneMask, const float time, const bool clampbonesinbbox ) {
	//Interfaces::MDLCache->BeginLock( );
	const auto backupCurTime{ Interfaces::Globals->flCurTime };
	const auto backupFrameTime{ Interfaces::Globals->flFrameTime };

	// idk if frametime will even be used... dont care though
	Interfaces::Globals->flCurTime = time;
	Interfaces::Globals->flFrameTime = Interfaces::Globals->flIntervalPerTick;

	player->m_iEFlags( ) |= EFL_SETTING_UP_BONES;

	auto hdr{ player->m_pStudioHdr( ) };
	if ( !hdr )
		return false;

	alignas( 16 ) Vector pos[ 256 ];
	alignas( 16 ) Quaternion q[ 256 ];

	uint8_t boneComputed[ 256 ]{ };
	std::memset( boneComputed, 0, 256 );

	const auto ik{ new CIKContext( ) };
	const auto backupIk{ player->m_pIk( ) };

	player->m_pIk( ) = ik;

	// cant get m_iIKCounter
	ik->Init( hdr, player->GetAbsAngles( ), player->GetAbsOrigin( ), Interfaces::Globals->flCurTime, Interfaces::Globals->iTickCount, boneMask );
	GetSkeleton( player, hdr, pos, q, boneMask, player->m_pIk( ) );

	player->m_pIk( )->UpdateTargets( pos, q, bones, boneComputed );
	player->CalculateIKLocks( Interfaces::Globals->flCurTime );
	player->m_pIk( )->SolveDependencies( pos, q, bones, boneComputed );

	// bone merge and moveparents are for non player characters

	BuildMatrices( player, hdr, pos, q, bones, boneMask, boneComputed );

	if ( clampbonesinbbox )
		player->ClampBonesInBBOX( bones, boneMask );

	player->m_pIk( ) = backupIk;

	delete ik;

	Interfaces::Globals->flCurTime = backupCurTime;
	Interfaces::Globals->flFrameTime = backupFrameTime;

	//Interfaces::MDLCache->EndLock( );

	player->m_iEFlags( ) &= ~EFL_SETTING_UP_BONES;

	return true; 
}