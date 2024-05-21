#include "ragebot.h"

bool CRageBot::ExtrapolatePlayer( PlayerEntry& entry, float yaw, int resolverSide, int amount, Vector previousVelocity ) {
	if ( yaw == -1 ) {
		if ( Config::Get<bool>( Vars.RagebotLagcompensation ) ) {
			yaw = ExtrapolateYawFromRecords( entry.m_pRecords, amount );

			if ( yaw == -1 )
				entry.m_pRecords.back( )->m_angEyeAngles.y;
		}
		else
			yaw = entry.m_flPreviousYaws.back( ).m_flYaw;
	}

	PreviousExtrapolationData_t previous{ entry.m_pPlayer->m_flDuckAmount( ), entry.m_pPlayer->m_fFlags( ) };
	auto angles{ entry.m_pRecords.back( )->m_angEyeAngles };
	angles.y = yaw;

	float assumedSpeed{ };

	const auto dir{ std::remainder( RAD2DEG(
		std::atan2( entry.m_optPreviousData->m_vecVelocity.y, entry.m_optPreviousData->m_vecVelocity.x ) ), 360.f ) };

	const auto prevDir{ std::remainder( RAD2DEG(
		std::atan2( previousVelocity.y, previousVelocity.x ) ), 360.f ) };

	auto direction{ std::remainder( RAD2DEG(
		std::atan2( ( previousVelocity - entry.m_optPreviousData->m_vecVelocity ).y, ( previousVelocity - entry.m_optPreviousData->m_vecVelocity ).x ) ), 360.f ) };

	if ( static_cast< int >( direction ) % 90 > 10
		&& static_cast< int >( direction ) % 90 < 80 ) {
		return false;
	}
	else {
		if ( std::abs( entry.m_optPreviousData->m_vecVelocity.Length2D( ) - previousVelocity.Length2D( ) ) < 10.f )
			assumedSpeed = entry.m_optPreviousData->m_vecVelocity.Length2D( );

		if ( direction <= 20 )
			direction = dir;
	}

	if ( entry.m_optPreviousData->m_vecVelocity.Length2D( ) < 5.f && entry.m_optPreviousData->m_iFlags & FL_ONGROUND )
		assumedSpeed = 1.1f;

	ctx.m_bExtrapolating = true;
	for ( int i{ 1 }; i <= amount; ++i ) {
		SimulatePlayer( entry.m_pPlayer, entry.m_pPlayer->m_flSimulationTime( ) + TICKS_TO_TIME( i ),
			angles, entry.m_iResolverSide, i == amount, previous, 
			direction, assumedSpeed, false );

		previous = { entry.m_pPlayer->m_flDuckAmount( ), entry.m_pPlayer->m_fFlags( ) };
	}
	ctx.m_bExtrapolating = false;

	return true;
}

bool CRageBot::ExtrapolatePlayer( CBasePlayer* player, float baseTime, int amount, QAngle angles, Vector previousVelocity, bool local ) {
	const auto backupJumping{ Features::AnimSys.m_bJumping };
	const auto backupLowerBodyRealignTimer{ Features::AnimSys.m_flLowerBodyRealignTimer };

	PreviousExtrapolationData_t previous{ player->m_flDuckAmount( ), player->m_fFlags( ) };

	float assumedSpeed{ };

	const auto dir = std::remainder( RAD2DEG(
		atan2( player->m_vecVelocity( ).y, player->m_vecVelocity( ).x ) ), 360.f );
	const auto prev_dir = std::remainder( RAD2DEG(
		atan2( previousVelocity.y, previousVelocity.x ) ), 360.f );

	if ( abs( dir - prev_dir ) >= 5.f
		&& !local )
		return false;

	if ( std::abs( player->m_vecVelocity( ).Length2D( ) - previousVelocity.Length2D( ) ) < 10.f )
		assumedSpeed = player->m_vecVelocity( ).Length2D( );

	const auto direction = dir;

	for ( int i{ 1 }; i <= amount; ++i ) {
		SimulatePlayer( player, baseTime + TICKS_TO_TIME( i ),
			angles, 0, i == amount, previous, 
			direction, assumedSpeed, local );

		previous = { player->m_flDuckAmount( ), player->m_fFlags( ) };
	}

	if ( local ) {
		Features::AnimSys.m_bJumping = backupJumping;
		Features::AnimSys.m_flLowerBodyRealignTimer = backupLowerBodyRealignTimer;
	}

	return true;
}

void CRageBot::SimulatePlayer( CBasePlayer* player, float time, QAngle angles, int resolverSide, bool last, PreviousExtrapolationData_t& previous, float direction, float assumedSpeed, bool local ) {
	CUserCmd cmd;
	CMoveData moveData;
	memset( &moveData, 0, sizeof( CMoveData ) );
	memset( &cmd, 0, sizeof( CUserCmd ) );
	cmd.viewAngles = angles;
	cmd.iTickCount = TICKS_TO_TIME( time );
	cmd.iCommandNumber = Interfaces::ClientState->nChokedCommands + Interfaces::ClientState->iLastOutgoingCommand + 1;
	const auto& curFlags{ player->m_fFlags( ) };

	const auto backupAttack{ player->m_flNextAttack( ) };

	if ( curFlags & FL_ONGROUND && !( previous.m_iFlags & FL_ONGROUND ) ) {
		if ( !local )
			player->m_flNextAttack( ) = time;

		cmd.iButtons |= IN_JUMP;
	}
	else if ( !local )
		player->m_flNextAttack( ) = 0.f;

	if ( ( player->m_flDuckAmount( ) > previous.m_flDuckAmount )
		|| curFlags & FL_DUCKING )
		cmd.iButtons |= IN_DUCK;

	// walking
	if ( player->m_iMoveState( ) == 1 )
		cmd.iButtons |= IN_SPEED;


	// not accelerating/decelerating- assume same pace
	if ( assumedSpeed )
		cmd.flForwardMove = assumedSpeed;
	else
		cmd.flForwardMove = 450.f;

	// assume they will stop
	// TODO: more logic?
	if ( !local && player->m_fFlags( ) & FL_ONGROUND ) {
		const auto hitboxSet{ ctx.m_pLocal->m_pStudioHdr( )->pStudioHdr->GetHitboxSet( ctx.m_pLocal->m_nHitboxSet( ) ) };

		auto enemyShootPos{ player->m_vecOrigin( ) };
		enemyShootPos.z += ( player->m_fFlags( ) & FL_DUCKING ) ? 46.f : 64.f;

		const auto hitbox{ hitboxSet->GetHitbox( HITBOX_CHEST ) };
		const auto center{ Math::VectorTransform( ( hitbox->vecBBMax + hitbox->vecBBMin ) * 0.5f,
					ctx.m_pLocal->m_CachedBoneData( ).Base( )[ hitbox->iBone ] ) };

		const auto weapon{ player->GetWeapon( ) };
		if ( !weapon )
			return;

		if ( !weapon->GetCSWeaponData( ) )
			return;

		const auto data{ Features::Autowall.FireBullet( player, ctx.m_pLocal, weapon->GetCSWeaponData( ),
			weapon->m_iItemDefinitionIndex( ) == WEAPON_TASER, enemyShootPos, center, true ) };

		if ( data.dmg > 0 ) {
			const auto maxWeaponSpeed{ player->m_flMaxSpeed( ) };
			auto optSpeed{ maxWeaponSpeed / 3.f };

			Features::Misc.LimitSpeed( cmd, optSpeed, player );
		}
	}

	auto moveDir{ cmd.viewAngles };

	moveDir.y = direction;
	Features::Misc.MoveMINTFix( cmd, moveDir, player->m_fFlags( ), player->m_MoveType( ) );

	if ( !last ) {
		if ( resolverSide == 1 )
			cmd.viewAngles.y += 120.f;
		else if ( resolverSide == 2 )
			cmd.viewAngles.y -= 120.f;

		cmd.viewAngles.Normalize( );
	}

	const auto originalplayercommand{ !player->CurrentCommand( ) ? CUserCmd( ) : *player->CurrentCommand( ) };
	const auto pPredictionPlayer{ *( *reinterpret_cast< CBasePlayer*** >( Displacement::Sigs.pPredictionPlayer ) ) };
	const auto originalrandomseed{ *( *reinterpret_cast< unsigned int** >( Displacement::Sigs.uPredictionRandomSeed ) ) };

	const auto backupMaxsZ{ player->m_vecMaxs( ).z };
	const auto backupDucking{ ( player->m_fFlags( ) & FL_DUCKING ) };
	const auto backupTickbase{ player->m_nTickBase( ) };

	const auto backupInPrediction{ Interfaces::Prediction->bInPrediction };
	const auto backupFrametime{ Interfaces::Globals->flFrameTime };
	const auto backupCurtime{ Interfaces::Globals->flCurTime };

	Interfaces::Prediction->bInPrediction = true;
	Interfaces::MoveHelper->SetHost( player );

	if ( player->CurrentCommand( ) )
		player->CurrentCommand( ) = &cmd;

	*( *reinterpret_cast< unsigned int** >( Displacement::Sigs.uPredictionRandomSeed ) ) = ( ( int( __thiscall* )( int ) )Displacement::Sigs.MD5PseudoRandom )( cmd.iCommandNumber ) & 0x7fffffff;
	*( *reinterpret_cast< CBasePlayer*** >( Displacement::Sigs.pPredictionPlayer ) ) = player;

	Interfaces::Globals->flFrameTime = Interfaces::Prediction->bEnginePaused ? 0.0f : Interfaces::Globals->flIntervalPerTick;
	Interfaces::Globals->flCurTime = time;

	ctx.m_bProhibitSounds = true;
	Interfaces::Prediction->CheckMovingGround( player, Interfaces::Globals->flFrameTime );
	Interfaces::Prediction->SetupMove( player, &cmd, Interfaces::MoveHelper, &moveData );
	Interfaces::GameMovement->ProcessMovement( player, &moveData );
	Interfaces::Prediction->FinishMove( player, &cmd, &moveData );
	Interfaces::MoveHelper->SetHost( nullptr );
	ctx.m_bProhibitSounds = false;

	/*if ( player->m_fFlags( ) & FL_DUCKING ) {
		player->SetCollisionBounds( { -16.f, -16.f, 0.f }, { 16.f, 16.f, 54.f } );
		player->m_vecViewOffset( ).z = 46.f;
	}
	else {
		player->SetCollisionBounds( { -16.f, -16.f, 0.f }, { 16.f, 16.f, 72.f } );
		player->m_vecViewOffset( ).z = 64.f;
	}

	if ( ( player->m_fFlags( ) & FL_DUCKING ) != backupDucking ) {
		// rebuild: server.dll/client.dll @ 55 8B EC 8B 45 10 F3 0F 10 81
		player->m_flNewBoundsMaxs( ) = player->m_flUnknownVar( ) + backupMaxsZ;
		player->m_flNewBoundsTime( ) = time;
	}*/

	player->m_vecAbsVelocity( ) = player->m_vecVelocity( );
	player->SetAbsOrigin( player->m_vecOrigin( ) );
	player->m_iEFlags( ) &= ~EFL_DIRTY_ABSVELOCITY;

	Interfaces::Prediction->bInPrediction = backupInPrediction;

	// animation
	if ( !local ) {
		const auto state{ player->m_pAnimState( ) };

		if ( state->iLastUpdateFrame == Interfaces::Globals->iFrameCount )
			state->iLastUpdateFrame = Interfaces::Globals->iFrameCount - 1;

		player->m_angEyeAngles( ) = cmd.viewAngles;

		player->m_bClientSideAnimation( ) = ctx.m_bUpdatingAnimations = true;
		player->UpdateClientsideAnimations( );
		player->m_bClientSideAnimation( ) = ctx.m_bUpdatingAnimations = false;
	}
	else {
		player->m_nTickBase( ) = TIME_TO_TICKS( Interfaces::Globals->flCurTime );
		Features::AnimSys.UpdateLocal( cmd.viewAngles, false, cmd );
	}

	if ( player->CurrentCommand( ) )
		*player->CurrentCommand( ) = originalplayercommand;
	*( *reinterpret_cast< unsigned int** >( Displacement::Sigs.uPredictionRandomSeed ) ) = originalrandomseed;
	*( *reinterpret_cast< CBasePlayer*** >( Displacement::Sigs.pPredictionPlayer ) ) = pPredictionPlayer;

	player->m_flNextAttack( ) = backupAttack;
	player->m_nTickBase( ) = backupTickbase;

	Interfaces::Globals->flCurTime = backupCurtime;
	Interfaces::Globals->flFrameTime = backupFrametime;
}

void CRageBot::AdjustDuckingVars( PlayerEntry& entry, int extrapolationAmount ) {
	auto extrapolationCapped{ static_cast< int >( ( extrapolationAmount ) / entry.m_iLastNewCmds ) * entry.m_iLastNewCmds };

	if ( entry.m_pPlayer->m_flDuckAmount( ) <= 0 || entry.m_pPlayer->m_flDuckAmount( ) >= 1 || !entry.m_optPreviousData.has_value( ) )
		return;

	auto predictedDuckAmount{ entry.m_pPlayer->m_flDuckAmount( ) }, predictedDuckSpeed{ entry.m_pPlayer->m_flDuckSpeed( ) };

	if ( entry.m_pPlayer->m_flDuckAmount( ) > entry.m_optPreviousData->m_flDuckAmount ) {
		if ( entry.m_pPlayer->m_fFlags( ) & FL_DUCKING )
			return;

		for ( int i{ 1 }; i <= extrapolationCapped; ++i ) {
			predictedDuckSpeed = Math::Approach( 8.f, predictedDuckSpeed, Interfaces::Globals->flIntervalPerTick * 3.0f );//8.f = CS_PLAYER_DUCK_SPEED_IDEAL

			auto duckSpeed{ entry.m_pPlayer->m_flDuckSpeed( ) * 0.8f };

			if ( entry.m_pPlayer->m_bIsDefusing( ) )
				duckSpeed *= 0.4f;

			predictedDuckAmount = Math::Approach( 1.f, predictedDuckAmount, Interfaces::Globals->flIntervalPerTick * duckSpeed );

			// finish ducking immediately if duck time is over [ or not on ground ] <- handled above
			if ( predictedDuckAmount >= 1.f ) {
				entry.m_pPlayer->m_vecViewOffset( ).z = 46.f;

				entry.m_pPlayer->m_flNewBoundsMaxs( ) = entry.m_pPlayer->m_flUnknownVar( ) + entry.m_pPlayer->m_vecMaxs( ).z;
				entry.m_pPlayer->m_flNewBoundsTime( ) = entry.m_pPlayer->m_flSimulationTime( ) + TICKS_TO_TIME( i );
				break;
			}
		}
	}
	else {
		if ( !( entry.m_pPlayer->m_fFlags( ) & FL_DUCKING ) )
			return;

		for ( int i{ 1 }; i <= extrapolationCapped; ++i ) {
			predictedDuckSpeed = Math::Approach( 8.f, predictedDuckSpeed, Interfaces::Globals->flIntervalPerTick * 3.0f );//8.f = CS_PLAYER_DUCK_SPEED_IDEAL

			// Always unduck at at least 1.5 to prevent advantageous semi-ducked positions
			auto duckSpeed{ std::max( 1.5f, predictedDuckSpeed ) };

			// Reduce crouch/uncrouch speed significantly while defusing
			if ( entry.m_pPlayer->m_bIsDefusing( ) )
				duckSpeed *= 0.4f;

			predictedDuckAmount = Math::Approach( 0.f, predictedDuckAmount, Interfaces::Globals->flIntervalPerTick * duckSpeed );

			// Remove the ducked flags if we're not fully ducked anymore.
			// REI: This is inconsistent with the documentation for these flags, but I'm not sure why the code
			//      is doing this.  It does mean you lose your ducking accuracy bonus very early in the un-duck,
			//      which is certainly important.
			if ( predictedDuckAmount <= 0.75f && entry.m_pPlayer->m_fFlags( ) & ( FL_ANIMDUCKING | FL_DUCKING ) ) {
				entry.m_pPlayer->m_vecViewOffset( ).z = 64.f;
				entry.m_pPlayer->m_flNewBoundsMaxs( ) = entry.m_pPlayer->m_flUnknownVar( ) + entry.m_pPlayer->m_vecMaxs( ).z;
				entry.m_pPlayer->m_flNewBoundsTime( ) = entry.m_pPlayer->m_flSimulationTime( ) + TICKS_TO_TIME( i );
				break;
			}
		}
	}
}

float CRageBot::ExtrapolateYaw( std::vector<PreviousYaw_t >& pattern, int extrapolationAmount ) {
	const auto& begin{ pattern.rbegin( ) };
	auto positionDetected{ begin };
	int bestMatches{ 4 };

	for ( auto it{ begin + 1 }; it != pattern.rend( ); ++it ) {
		if ( it == pattern.rend( ) )
			break;

		if ( begin->m_iTickCount - it->m_iTickCount < extrapolationAmount )
			continue;

		int matches{ };
		//int tolerance{ 1 };
		for ( int i{ }; i < pattern.size( ); ++i ) {
			if ( ( it + i + 1 ) == pattern.rend( ) )
				break;

			if ( std::abs( ( it + i )->m_flAngleDifference - ( begin + i )->m_flAngleDifference ) < 20.f )
				++matches;
			else {
				//if ( !tolerance )
					break;

				//--tolerance;
			}

			if ( matches >= 30 )
				break;
		}

		if ( matches > bestMatches ) {
			bestMatches = matches;
			positionDetected = it;
			if ( bestMatches >= 30 )
				break;
		}
	}

	float ret = positionDetected->m_flYaw;
	if ( positionDetected != begin ) {
		const auto posDetectedTime = positionDetected->m_iTickCount;
		for ( auto it{ positionDetected }; it != begin; --it ) {
			if ( posDetectedTime + extrapolationAmount < it->m_iTickCount )
				break;
			ret = it->m_flYaw;
		}
		ret += Math::AngleDiff( positionDetected->m_flYaw, begin->m_flYaw );
	}
	else
		return -1;

	return Math::NormalizeEyeAngles( ret );
}

float CRageBot::ExtrapolateYawFromRecords( std::vector<std::shared_ptr<LagRecord_t>>& records, int extrapolationAmount ) {
	const auto& begin{ records.rbegin( ) };
	auto positionDetected{ begin };
	int bestMatches{ 1 };

	for ( auto it{ begin + 1 }; it != records.rend( ); ++it ) {
		auto record{ it->get( ) };

		if ( it == records.rend( ) )
			break;

		if ( begin->get( )->m_iReceiveTick - record->m_iReceiveTick < extrapolationAmount )
			continue;

		int matches{ };
		for ( int i{ }; i < records.size( ); ++i ) {
			if ( ( it + i + 1 ) == records.rend( ) )
				break;

			const auto angleDiffIt{ Math::AngleDiff( ( it + i + 1 )->get( )->m_angEyeAngles.y, ( it + i )->get( )->m_angEyeAngles.y ) };
			const auto angleDiffStart{ Math::AngleDiff( ( begin + i + 1 )->get( )->m_angEyeAngles.y, ( begin + i )->get( )->m_angEyeAngles.y ) };

			if ( std::abs( angleDiffIt - angleDiffStart < 20.f ) )
				++matches;
			else
				break;

			if ( matches >= 3 )
				break;
		}

		if ( matches > bestMatches ) {
			bestMatches = matches;
			positionDetected = it;
			if ( bestMatches >= 3 )
				break;
		}
	}

	float ret = positionDetected->get( )->m_angEyeAngles.y;
	if ( positionDetected != begin ) {
		const auto posDetectedTime = positionDetected->get( )->m_iReceiveTick;
		for ( auto it{ positionDetected }; it != begin; --it ) {
			if ( posDetectedTime + extrapolationAmount < it->get( )->m_iReceiveTick )
				break;
			ret = it->get( )->m_angEyeAngles.y;
		}
		ret += Math::AngleDiff( positionDetected->get( )->m_angEyeAngles.y, begin->get( )->m_angEyeAngles.y );
	}
	else
		return -1;

	return Math::NormalizeEyeAngles( ret );
}