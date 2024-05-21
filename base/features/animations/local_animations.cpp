#include "animation.h"

void CAnimationSys::UpdateLocal( QAngle viewAngles, const bool onlyAnimState, CUserCmd& cmd ) {
	const auto state{ ctx.m_pLocal->m_pAnimState( ) };
	if ( !state )
		return;

	const auto backupCurTime{ Interfaces::Globals->flCurTime };
	const auto backupFrameTime{ Interfaces::Globals->flFrameTime };
	const auto backupHLTV{ Interfaces::ClientState->bIsHLTV };

	Interfaces::Globals->flCurTime = TICKS_TO_TIME( ctx.m_pLocal->m_nTickBase( ) );
	Interfaces::Globals->flFrameTime = Interfaces::Globals->flIntervalPerTick;
	Interfaces::ClientState->bIsHLTV = true;

	Interfaces::Prediction->SetLocalViewAngles( viewAngles );

	UpdateServerLayers( cmd );

	//ctx.m_pLocal->SetAbsVelocity( ctx.m_pLocal->m_vecVelocity( ) );

	state->iLastUpdateFrame = Interfaces::Globals->iFrameCount - 1;

	ctx.m_pLocal->m_bClientSideAnimation( ) = ctx.m_bUpdatingAnimations = true;
	ctx.m_pLocal->UpdateClientsideAnimations( );
	ctx.m_pLocal->m_bClientSideAnimation( ) = ctx.m_bUpdatingAnimations = false;

	Interfaces::Globals->flCurTime = backupCurTime;
	Interfaces::Globals->flFrameTime = backupFrameTime;
	Interfaces::ClientState->bIsHLTV = backupHLTV;
}

constexpr float CSGO_ANIM_LOWER_REALIGN_DELAY{ 1.1f };

void CAnimationSys::UpdateServerLayers( CUserCmd& cmd ) {
	const auto state{ ctx.m_pLocal->m_pAnimState( ) };

	const bool onGround{ static_cast<bool>( ctx.m_pLocal->m_fFlags( ) & FL_ONGROUND ) };
	const auto onLadder{ !onGround && ctx.m_pLocal->m_MoveType( ) == MOVETYPE_LADDER };

	float flMaxSpeedRun = std::max( ctx.m_pLocal->m_flMaxSpeed( ), 0.001f );

	const auto speedAsPortionOfWalkTopSpeed{ ctx.m_pLocal->m_vecVelocity( ).Length2D( ) / ( flMaxSpeedRun * 0.52f ) };


	const auto startedLadderingThisFrame = !state->bOnLadder && onLadder;
	const auto stoppedLadderingThisFrame = state->bOnLadder && !onLadder;
	const auto landedOnGroundThisFrame{ state->bOnGround != onGround && onGround };
	const auto leftGroundThisFrame{ state->bOnGround != onGround && !onGround };
	static bool landing{ };

	bool stoppedMovingThisFrame{ };
	if ( ctx.m_pLocal->m_vecAbsVelocity( ).y > 0 )
		stoppedMovingThisFrame = ( state->flDurationMoving <= 0 );
	else
		stoppedMovingThisFrame = ( state->flDurationStill <= 0 );

	state->flLastUpdateIncrement = Interfaces::Globals->flIntervalPerTick;

	/* CCSGOPlayerAnimState::SetUpVelocity */
	/*auto& ANIMATION_LAYER_ADJUST{ ctx.m_pLocal->m_AnimationLayers( )[ 3 ] };

	if ( !state->m_bAdjustStarted && stoppedMovingThisFrame && onGround && !onLadder && !landing && state->m_flStutterStep < 50 ) {
		state->m_bAdjustStarted = true;
		state->SetLayerSequence( &ANIMATION_LAYER_ADJUST, ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING );
	}

	if ( ctx.m_pLocal->GetSequenceActivity( ANIMATION_LAYER_ADJUST.nSequence ) == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING ||
		ctx.m_pLocal->GetSequenceActivity( ANIMATION_LAYER_ADJUST.nSequence ) == ACT_CSGO_IDLE_TURN_BALANCEADJUST ) {
		auto prevWeight{ ANIMATION_LAYER_ADJUST.flWeight };
		if ( state->m_bAdjustStarted && speedAsPortionOfWalkTopSpeed <= 0.25f ) {
			state->IncrementLayerCycle( &ANIMATION_LAYER_ADJUST, false );
			ANIMATION_LAYER_ADJUST.flWeight = state->GetLayerIdealWeightFromSeqCycle( 3 );
			state->m_bAdjustStarted = !( ANIMATION_LAYER_ADJUST.flCycle + Interfaces::Globals->flIntervalPerTick * ANIMATION_LAYER_ADJUST.flPlaybackRate >= 1.f );
		}
		else {
			state->m_bAdjustStarted = false;
			ANIMATION_LAYER_ADJUST.flWeight = std::clamp( Math::Approach( 0.f, prevWeight, Interfaces::Globals->flIntervalPerTick * 5.f ), 0.f, 1.f );
		}

		ANIMATION_LAYER_ADJUST.flWeightDeltaRate = ( ANIMATION_LAYER_ADJUST.flWeight - prevWeight ) / Interfaces::Globals->flIntervalPerTick;
	}*/



	if ( onGround ) {
		if ( ctx.m_pLocal->m_vecAbsVelocity( ).Length2D( ) > 0.1f ) {
			m_flLowerBodyRealignTimer = Interfaces::Globals->flCurTime + ( CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f );
			ctx.m_pLocal->m_flLowerBodyYawTarget( ) = cmd.viewAngles.y;
		}
		else {
			if ( Interfaces::Globals->flCurTime > m_flLowerBodyRealignTimer && std::abs( Math::AngleDiff( state->flAbsYaw, cmd.viewAngles.y ) ) > 35.0f ) {
				m_flLowerBodyRealignTimer = Interfaces::Globals->flCurTime + CSGO_ANIM_LOWER_REALIGN_DELAY;
				ctx.m_pLocal->m_flLowerBodyYawTarget( ) = cmd.viewAngles.y;
			}
		}
	}

	/* do weapon */

	/* CCSGOPlayerAnimState::SetUpMovement */
	auto& MOVEMENT_LAND_OR_CLIMB{ ctx.m_pLocal->m_AnimationLayers( )[ 5 ] };
	auto& MOVEMENT_JUMP_OR_FALL{ ctx.m_pLocal->m_AnimationLayers( )[ 4 ] };

	// this will be off by 1 tick (it doesnt really make much of a difference)
	if ( state->m_flLadderWeight > 0 || onLadder ) {
		if ( startedLadderingThisFrame )
			state->SetLayerSequence( &MOVEMENT_LAND_OR_CLIMB, ACT_CSGO_CLIMB_LADDER );
	}


	if ( onGround ) {
		if ( !landing && ( landedOnGroundThisFrame || stoppedLadderingThisFrame ) ) {
			state->SetLayerSequence( &MOVEMENT_LAND_OR_CLIMB, state->flDurationInAir > 1.f ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT );
			landing = true;
		}

		if ( landing && ctx.m_pLocal->GetSequenceActivity( MOVEMENT_LAND_OR_CLIMB.nSequence ) != ACT_CSGO_CLIMB_LADDER ) {
			m_bJumping = false;

			if ( MOVEMENT_JUMP_OR_FALL.flCycle + Interfaces::Globals->flFrameTime * MOVEMENT_JUMP_OR_FALL.flPlaybackRate >= 1.f )
				landing = false;
		}

		// this uses new m_bLanding
		// ladder weight not set leading to 1 tick difference
		if ( !landing && !m_bJumping && state->m_flLadderWeight <= 0 )
			MOVEMENT_LAND_OR_CLIMB.flWeight = 0;
	}
	else if ( !onLadder ) {
		landing = false;

		if ( leftGroundThisFrame || stoppedLadderingThisFrame ) {
			if ( !m_bJumping )
				state->SetLayerSequence( &MOVEMENT_JUMP_OR_FALL, ACT_CSGO_FALL );
		}
	}

	/* do SetUpFlinch/SetUpWeaponAction */

	/* SetUpLean */
	//ctx.m_pLocal->m_AnimationLayers( )[ 12 ].nSequence = 7;

	if ( cmd.iButtons & IN_JUMP && ( onGround || onLadder ) ) {
		state->SetLayerSequence( &MOVEMENT_JUMP_OR_FALL, ACT_CSGO_JUMP );
		m_bJumping = true;
	}
}