#include "antiaim.h"
#include "../animations/animation.h"
#include "../rage/exploits.h"
// TODO: antiresolver!
// check if we are able to be resolved from animlayers, and balance adjust then just flip desync
// also force balance adjust to be triggered when stopping or randomly when we are standing

void CAntiAim::Pitch( CUserCmd& cmd ) {
	if ( Condition( cmd, true ) )
		return;

	if ( Interfaces::ClientState->nChokedCommands < 1 && Config::Get<bool>( Vars.AntiaimDesync ) )
		ctx.m_bSendPacket = false;

	const auto cond{ ctx.m_bSafeFromDefensive && Features::Exploits.m_bWasDefensiveTick ? Config::Get<int>( Vars.AntiaimSafePitch ) : Config::Get<int>( Vars.AntiaimPitch ) };

	switch ( cond ) {
	case 1: cmd.viewAngles.x = -89.f; break;// up
	case 2: cmd.viewAngles.x =  89.f; break;// down
	case 3: cmd.viewAngles.x =  0.f;  break;// zero
	case 4: cmd.viewAngles.x = Math::RandomFloat( -89.f, 89 );  break;// random
	default: break;
	}
}

void CAntiAim::PickYaw( float& yaw ) {
	static bool invert{ };
	static int rotatedYaw{ };

	const int& yawRange{ Config::Get<int>( Vars.AntiaimYawRange ) };

	switch ( Config::Get<int>( Vars.AntiaimYaw ) ) {
	case 0: yaw += 0.f; break;// forward
	case 1: yaw += 180.f; break;// backward
	case 2: yaw += 90.f; break;// left
	case 3: yaw -= 90.f; break;// right
	}

	switch ( Config::Get<int>( Vars.AntiaimYawAdd ) ) {
	case 1:// jitter 
		yaw += yawRange * ( ChokeCycleJitter ? 0.5f : -0.5f );
		break;
	case 2: {// rotate
		//if ( Interfaces::ClientState->nChokedCommands )
		//	break;

		rotatedYaw -= invert ? Config::Get<int>( Vars.AntiaimYawSpeed ) : -Config::Get<int>( Vars.AntiaimYawSpeed );

		if ( rotatedYaw < yawRange * -0.5f )
			invert = false;
		else if ( rotatedYaw > yawRange * 0.5f )
			invert = true;

		rotatedYaw = std::clamp<int>( rotatedYaw, yawRange * -0.5f, yawRange * 0.5f );
		
		yaw += rotatedYaw;
	}break;
	case 3: {// spin
		rotatedYaw += Config::Get<int>( Vars.AntiaimYawSpeed );
		rotatedYaw = std::remainderf( rotatedYaw, 360.f );
		yaw = rotatedYaw;
		break;
	}
	case 4: {// random
		yaw += Math::RandomFloat( -yawRange / 2.f, yawRange / 2.f );
		break;
	}
	default: break;
	}

	if ( ctx.m_bSafeFromDefensive && Features::Exploits.m_bWasDefensiveTick ) {
		const auto amount{ Config::Get<int>( Vars.AntiaimSafeYawRandomisation ) / 100.f };
		yaw += Math::RandomFloat( -180 * amount, 180 * amount );
	}
}

int CAntiAim::Freestanding( ) {
	if ( !Config::Get<int>( Vars.AntiaimFreestanding )
		|| !Config::Get<keybind_t>( Vars.AntiaimFreestandingKey ).enabled )
		return 0;

	Vector forward, right;
	Math::AngleVectors( ctx.m_angOriginalViewangles, &forward, &right );

	CGameTrace tr;

	// middle
	Interfaces::EngineTrace->TraceRay(
		{ ctx.m_vecEyePos, ctx.m_vecEyePos + forward * 100.0f }, MASK_PLAYERSOLID,
		nullptr, &tr
	);
	const auto middleDist{ ( tr.vecEnd - tr.vecStart ).Length( ) };

	// right
	Interfaces::EngineTrace->TraceRay(
		{ ctx.m_vecEyePos + right * 35.0f, ( ctx.m_vecEyePos + forward * 100.0f ) + right * 35.0f }, MASK_PLAYERSOLID,
		nullptr, &tr
	);
	const auto rightDist{  ( tr.vecEnd - tr.vecStart ).Length( ) };

	// left
	Interfaces::EngineTrace->TraceRay(
		{ ctx.m_vecEyePos - right * 35.0f, ( ctx.m_vecEyePos + forward * 100.0f ) - right * 35.0f }, MASK_PLAYERSOLID,
		nullptr, &tr
	);
	const auto leftDist{ ( tr.vecEnd - tr.vecStart ).Length( ) };

	if ( rightDist > leftDist + 20.f ) {
		if ( rightDist > middleDist + 20.f )
			return 1;
	}

	if ( leftDist > rightDist + 20.f ) {
		if ( leftDist > middleDist + 20.f )
			return 2;
	}

	return 0;
}

float CAntiAim::BaseYaw( CUserCmd& cmd ) {
	m_bAntiBackstab = false;
	//if ( !Interfaces::ClientState->nChokedCommands )
	ChokeCycleJitter = !ChokeCycleJitter;

	if ( Config::Get<bool>( Vars.AntiaimConstantInvert ) )
		Invert = !Invert;
	else {
		static auto old{ Config::Get<keybind_t>( Vars.AntiaimInvert ).enabled };
		if ( old != Config::Get<keybind_t>( Vars.AntiaimInvert ).enabled )
			Invert = old = Config::Get<keybind_t>( Vars.AntiaimInvert ).enabled;
	}

	auto yaw = cmd.viewAngles.y;

	const auto side{ Freestanding( ) };

	if ( Config::Get<bool>( Vars.AntiAimManualDir ) ) {
		if ( ManualSide == 1 ) {
			yaw += 90.f;
			return yaw;
		}
		else if ( ManualSide == 2 ) {
			yaw -= 90.f;
			return yaw;
		}
	}

	if ( side && Config::Get<int>( Vars.AntiaimFreestanding ) == 1 ) {
		if ( side == 1 ) {
			yaw += 90.f;
			return yaw;
		}
		else if ( side == 2 ) {
			yaw -= 90.f;
			return yaw;
		}
	}
	else if ( Config::Get<int>( Vars.AntiaimFreestanding ) == 2 || Config::Get<int>( Vars.AntiaimFreestanding ) == 3 ) {
		if ( side == 1 )
			Invert = Config::Get<int>( Vars.AntiaimFreestanding ) == 2;
		else if ( side == 2 )
			Invert = Config::Get<int>( Vars.AntiaimFreestanding ) != 2;
	}

	AtTarget( yaw );

	if ( m_bFlickNow ) {
		if ( Config::Get<bool>( Vars.AntiaimConstantInvertFlick ) )
			m_bInvertFlick = !m_bInvertFlick;
		else
			m_bInvertFlick = Config::Get<keybind_t>( Vars.AntiaimFlickInvert ).enabled;

		yaw += ( m_bInvertFlick ? -Config::Get<int>( Vars.AntiaimFlickAdd ) : Config::Get<int>( Vars.AntiaimFlickAdd ) );
	}

	if ( m_bAntiBackstab )
		return yaw;

	PickYaw( yaw );
	//AutoDirection( yaw );

	return yaw;
}

void CAntiAim::AtTarget( float& yaw ) {
	CBasePlayer* bestPlayer{ nullptr };
	auto bestValue = INT_MAX;

	if ( !Config::Get<int>( Vars.AntiaimAtTargets ) && !Config::Get<bool>( Vars.AntiaimAntiBackStab ) )
		return;

	for ( auto i = 1; i < 64; ++i ) {
		const auto player{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
		if ( !player
			|| !player->IsPlayer( )
			|| player->Dormant( )
			|| player->IsDead( )
			|| player->IsTeammate( ) )
			continue;

		const auto dist{ ( ctx.m_pLocal->m_vecOrigin( ) - player->m_vecOrigin( ) ).Length( ) };
		if ( Config::Get<bool>( Vars.AntiaimAntiBackStab ) ) {
			if ( player->GetWeapon( )->IsKnife( ) ) {
				if ( dist < 250 ) {
					bestPlayer = player;
					m_bAntiBackstab = true;
					break;
				}
			}
		}

		switch ( Config::Get<int>( Vars.AntiaimAtTargets ) ) {
		case 1: {// FOV/closest to crosshair
			const auto fov = Math::GetFov( ctx.m_angOriginalViewangles, Math::CalcAngle( ctx.m_vecEyePos, player->GetAbsOrigin( ) ) );
			if ( fov > bestValue )
				continue;

			bestValue = fov;
			bestPlayer = player;
		}break;
		case 2: {// Distance
			if ( dist > bestValue )
				continue;

			bestValue = dist;
			bestPlayer = player;
		} break;
		default: break;
		}
	}

	if ( !bestPlayer )
		return;

	const auto x{ bestPlayer->m_vecOrigin( ).x - ctx.m_pLocal->m_vecOrigin( ).x };
	const auto y{ bestPlayer->m_vecOrigin( ).y - ctx.m_pLocal->m_vecOrigin( ).y };

	yaw = x == 0.f && y == 0.f ? 0.f : RAD2DEG( std::atan2( y, x ) );
}

bool CAntiAim::Condition( CUserCmd& cmd, bool checkCmd ) {
	if ( !Config::Get<bool>( Vars.AntiaimEnable ) )
		return true;

	if ( ctx.m_pLocal->m_MoveType( ) == MOVETYPE_NOCLIP || ctx.m_pLocal->m_MoveType( ) == MOVETYPE_LADDER )
		return true;

	if ( !ctx.m_pWeapon )
		return true;

	if ( Interfaces::GameRules && Interfaces::GameRules->IsFreezeTime( ) )
		return true;

	if ( ctx.m_pWeapon->IsGrenade( ) && ctx.m_pWeapon->m_fThrowTime( ) )
		return true;

	if ( !checkCmd )
		return false;

	if ( cmd.iButtons & IN_ATTACK && !ctx.m_pWeapon->IsGrenade( )
		&& ( ctx.m_pWeapon->m_iItemDefinitionIndex( ) != WEAPON_REVOLVER || ctx.m_bRevolverCanShoot ) )
		return true;

	// e
	if ( cmd.iButtons & IN_USE )
		return true;

	// right click
	if ( cmd.iButtons & IN_ATTACK2 && ctx.m_pWeapon->IsKnife( ) /*&& ctx.can_shoot*/ )
		return true;

	return false;
}

void CAntiAim::FakeLag( int cmdNum ) {
	if ( Interfaces::GameRules && Interfaces::GameRules->IsFreezeTime( ) ) {
		ctx.m_bSendPacket = true;
		return;
	}

	static int maxChoke = Config::Get<int>( Vars.AntiaimFakeLagLimit );

	const int& max = Config::Get<int>( Vars.AntiaimFakeLagLimit );

	if ( !Interfaces::ClientState->nChokedCommands )
		maxChoke = Math::RandomInt( static_cast< int >( max * ( 1.f - ( static_cast< float >( Config::Get<int>( Vars.AntiaimFakeLagVariance ) ) / 100.f ) ) ), max );

	if ( Config::Get<bool>( Vars.AntiaimDesync ) )
		maxChoke = std::max( maxChoke, 1 );

	const auto& localData = ctx.m_cLocalData.at( Interfaces::ClientState->iLastOutgoingCommand % 150 );

	if ( Interfaces::ClientState->nChokedCommands >= maxChoke )
		ctx.m_bSendPacket = true;

	if ( Config::Get<bool>( Vars.AntiaimFakeLagBreakLC )
		&& ( ctx.m_pLocal->m_vecOrigin( ) - localData.PredictedNetvars.m_vecOrigin ).LengthSqr( ) > 4096.f )
		ctx.m_bSendPacket = true;

	if ( Config::Get<bool>( Vars.AntiaimFakeLagInPeek ) ) {
		
		if ( cmdNum - ctx.m_iLastPeekCmdNum < 15
			&& ctx.m_bInPeek ) {
			ctx.m_bSendPacket = false;
			//return;
		}
		else if ( ctx.m_bInPeek )
			ctx.m_bSendPacket = true;
	}

	if ( m_bFlickNow )
		m_bFlickNow = false;

	if ( Config::Get<bool>( Vars.AntiaimFlickHead ) && !ctx.m_bFakeDucking ) {
		if ( ++m_iFlickTimer > Config::Get<int>( Vars.AntiaimFlickSpeed ) ) {
			m_bFlickNow = ctx.m_bSendPacket = true;
			m_iFlickTimer = 0;
		}
	}
}

void CAntiAim::RunLocalModifications( CUserCmd& cmd, int tickbase ) {

	const auto animstate{ ctx.m_pLocal->m_pAnimState( ) };
	const auto totalCmds{ Interfaces::ClientState->nChokedCommands + 1 };
	if ( totalCmds < 1
		|| !animstate )
		return;

	auto yaw{ std::remainderf( BaseYaw( cmd ), 360.f ) };

	if ( ctx.m_bSafeFromDefensive && Features::Exploits.m_bWasDefensiveTick ) {
		const auto amount{ Config::Get<int>( Vars.AntiaimSafeYawRandomisation ) / 100.f };
		yaw += Math::RandomFloat( -180 * amount, 180 * amount );
	}

	const auto inShot{ ctx.m_iLastShotNumber > Interfaces::ClientState->iLastOutgoingCommand
		&& ctx.m_iLastShotNumber <= ( Interfaces::ClientState->iLastOutgoingCommand + Interfaces::ClientState->nChokedCommands + 1 ) };

	if ( animstate->bFirstUpdate )
		ctx.m_cFakeData.init = false;

	const auto backupState{ *animstate };
	CAnimationLayer backupLayers[ 13 ]{ };
	std::memcpy( backupLayers, ctx.m_pLocal->m_AnimationLayers( ), 13 * sizeof CAnimationLayer );

	bool did{ };
	const auto oldLBY{ ctx.m_pLocal->m_flLowerBodyYawTarget( ) };

	QAngle* holdAngle{ };

	// avoid overlap
	if ( Config::Get<bool>( Vars.AntiaimTrickLBY ) )
		Invert = Math::AngleDiff( oldLBY, yaw ) <= 0;

	// use brain: this can cause avoid overlap to be wrong... what do i do here...
	if ( Invert )
		yaw += Config::Get<int>( Vars.AntiaimInvertedYawAdd );
	else
		yaw += Config::Get<int>( Vars.AntiaimNormalYawAdd );

	for ( auto i{ 1 }; i <= totalCmds; ++i ) {
		const auto j{ ( Interfaces::ClientState->iLastOutgoingCommand + i ) % 150 };

		auto& curUserCmd{ Interfaces::Input->pCommands[ j ] };
		auto& curLocalData{ ctx.m_cLocalData.at( j ) };

		const auto lastCmd{ i == totalCmds };

		if ( curLocalData.m_flSpawnTime != ctx.m_pLocal->m_flSpawnTime( ) )
			continue;

		if ( curUserCmd.iTickCount == INT_MAX )
			continue;

		if ( curLocalData.PredictedNetvars.m_MoveType != MOVETYPE_LADDER
			&& curLocalData.m_MoveType != MOVETYPE_LADDER ) {
			if ( curLocalData.m_bCanAA ) {
				const auto oldViewAngles{ curUserCmd.viewAngles };

				curUserCmd.viewAngles.y = yaw;

				if ( !lastCmd && !inShot && Config::Get<bool>( Vars.AntiaimDesync ) )
					curUserCmd.viewAngles.y += ( Invert ? -120.f : 120.f );

				Features::Misc.MoveMINTFix(
					curUserCmd, oldViewAngles,
					curLocalData.PredictedNetvars.m_iFlags,
					curLocalData.PredictedNetvars.m_MoveType
				);

			}

			Features::Misc.NormalizeMovement( curUserCmd );
		}

		if ( lastCmd ) {
			if ( ctx.m_pLocal->m_MoveType( ) == MOVETYPE_WALK ) {
				cmd.iButtons &= ~( IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT );

				if ( cmd.flForwardMove != 0.f )
					cmd.iButtons |=
					( Config::Get<bool>( Vars.MiscSlideWalk ) ? cmd.flForwardMove < 0.f : cmd.flForwardMove > 0.f )
					? IN_FORWARD : IN_BACK;

				if ( cmd.flSideMove ) {
					cmd.iButtons |=
						( Config::Get<bool>( Vars.MiscSlideWalk ) ? cmd.flSideMove < 0.f : cmd.flSideMove > 0.f )
						? IN_MOVERIGHT : IN_MOVELEFT;
				}
			}
			Pitch( curUserCmd );
		}

		if ( curUserCmd.iButtons & IN_ATTACK )
			holdAngle = &curUserCmd.viewAngles;

		ctx.m_pLocal->m_nTickBase( ) = tickbase - ( totalCmds - i );
		ctx.m_pLocal->m_fFlags( ) = curLocalData.PredictedNetvars.m_iFlags;
		ctx.m_pLocal->m_vecAbsVelocity( ) = curLocalData.PredictedNetvars.m_vecVelocity;
		ctx.m_pLocal->m_flDuckAmount( ) = curLocalData.PredictedNetvars.m_flDuckAmount;

		Features::AnimSys.UpdateLocal( holdAngle ? *holdAngle : curUserCmd.viewAngles, lastCmd, curUserCmd );

		Interfaces::Input->pVerifiedCommands[ j ].userCmd = curUserCmd;
		Interfaces::Input->pVerifiedCommands[ j ].uHashCRC = curUserCmd.GetChecksum( );
	}

	ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flWeight = ctx.m_pAnimationLayers[ 3 ].flWeight;
	ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flCycle = ctx.m_pAnimationLayers[ 3 ].flCycle;
	std::memcpy( ctx.m_pAnimationLayers, ctx.m_pLocal->m_AnimationLayers( ), 13 * sizeof CAnimationLayer );

	const auto backupCBBTime{ ctx.m_pLocal->m_flNewBoundsTime( ) };

	if ( !Config::Get<bool>( Vars.VisRecordAnims ) || !ctx.m_bSafeFromDefensive ) {
		ctx.m_pLocal->SetAbsAngles( { 0.f, animstate->flAbsYaw, 0.f } );

		static auto lookupBone{ *reinterpret_cast< int( __thiscall* )( void*, const char* ) >( Displacement::Sigs.LookupBone ) };
		const auto boneIndex{ lookupBone( ctx.m_pLocal, _( "lean_root" ) ) };

		if ( ctx.m_pLocal->m_pStudioHdr( )->vecBoneFlags[ boneIndex ] != BONE_USED_BY_SERVER )
			ctx.m_pLocal->m_pStudioHdr( )->vecBoneFlags[ boneIndex ] = BONE_USED_BY_SERVER;

		const auto backup12Weight{ ctx.m_pLocal->m_AnimationLayers( )[ 12 ].flWeight };
		const auto backup3Weight{ ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flWeight };
		const auto backup3Cycle{ ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flCycle };

		ctx.m_pLocal->m_AnimationLayers( )[ 12 ].flWeight *= float( Config::Get<int>( Vars.MiscLocalAnimSway ) / 10.f );
		ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flCycle = 0.f;
		ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flWeight = 0.f;

		ctx.m_pLocal->m_flNewBoundsTime( ) = 0;

		if ( Config::Get<bool>( Vars.ExploitsBreakCBB ) && !ctx.m_iTicksAllowed ) {
			QAngle angle{ 0, 0, 0 };
			Interfaces::Prediction->SetLocalViewAngles( angle );
		}

		Features::AnimSys.SetupBonesRebuilt( ctx.m_pLocal, ctx.m_matRealLocalBones, BONE_USED_BY_SERVER,
			Interfaces::Globals->flCurTime, true );

		ctx.m_pLocal->m_flNewBoundsTime( ) = backupCBBTime;

		std::memcpy( ctx.m_pLocal->m_CachedBoneData( ).Base( ), ctx.m_matRealLocalBones, ctx.m_pLocal->m_iBoneCount( ) * sizeof( matrix3x4a_t ) );

		//ctx.m_pLocal->m_AnimationLayers( )[ 12 ].flWeight = backup12Weight;
		ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flCycle = backup3Cycle;
		ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flWeight = backup3Weight;

		ctx.m_vecSetupBonesOrigin = ctx.m_pLocal->GetAbsOrigin( );
	}

	/* fake matrix */
	if ( !Config::Get<bool>( Vars.ChamDesync ) )
		return;

	const auto backupState2{ *animstate };

	if ( ctx.m_cFakeData.init )
		*animstate = ctx.m_cFakeData.m_sState;
	else
		*animstate = backupState;

	std::memcpy( ctx.m_pLocal->m_AnimationLayers( ), backupLayers, 13 * sizeof CAnimationLayer );

	for ( auto i{ 1 }; i <= totalCmds; ++i ) {
		const auto j{ ( Interfaces::ClientState->iLastOutgoingCommand + i ) % 150 };

		auto& curUserCmd{ Interfaces::Input->pCommands[ j ] };
		auto& curLocalData{ ctx.m_cLocalData.at( j ) };

		ctx.m_pLocal->m_nTickBase( ) = tickbase - ( totalCmds - i );
		ctx.m_pLocal->m_fFlags( ) = curLocalData.PredictedNetvars.m_iFlags;
		ctx.m_pLocal->m_vecAbsVelocity( ) = curLocalData.PredictedNetvars.m_vecVelocity;

		Features::AnimSys.UpdateLocal( holdAngle ? *holdAngle : cmd.viewAngles, true, curUserCmd );
	}

	if ( !Config::Get<bool>( Vars.VisRecordAnims ) || !ctx.m_bSafeFromDefensive ) {

		ctx.m_pLocal->SetAbsAngles( { 0.f, animstate->flAbsYaw, 0.f } );

		// enemy gets the real layers networked to them
		std::memcpy( ctx.m_pLocal->m_AnimationLayers( ), ctx.m_pAnimationLayers, 13 * sizeof CAnimationLayer );

		//const auto backupWeight{ ctx.m_pLocal->m_AnimationLayers( )[ 12 ].flWeight };
		//ctx.m_pLocal->m_AnimationLayers( )[ 12 ].flWeight *= 2.f;

		ctx.m_pLocal->m_flNewBoundsTime( ) = 0;

		//Features::Logger.Log( std::to_string( cmd.viewAngles.y ), false );

		Features::AnimSys.SetupBonesRebuilt( ctx.m_pLocal, ctx.m_cFakeData.m_matMatrix, BONE_USED_BY_SERVER,
			Interfaces::Globals->flCurTime, true );

		ctx.m_pLocal->m_flNewBoundsTime( ) = backupCBBTime;
		//ctx.m_pLocal->m_AnimationLayers( )[ 12 ].flWeight = backupWeight;

		ctx.m_cFakeData.m_sState = *animstate;
		ctx.m_cFakeData.init = true;
		*animstate = backupState2;

		std::memcpy( ctx.m_pLocal->m_CachedBoneData( ).Base( ), ctx.m_matRealLocalBones, ctx.m_pLocal->m_CachedBoneData( ).Size( ) * sizeof( matrix3x4_t ) );
	}
}

// pasta reis courtesy of slazy
/*
bool CAntiAim::AutoDirection( float& yaw ) {
	if ( !Config::Get<int>( Vars.AntiaimFreestand ) )
		return false;

	CBasePlayer* best_player{ };
	auto best_fov = std::numeric_limits< float >::max( );

	const auto view_angles = ctx.m_angOriginalViewangles;

	for ( auto i = 1; i < 64; ++i ) {
		const auto player = static_cast< CBasePlayer* >(
			Interfaces::ClientEntityList->GetClientEntity( i )
			);
		if ( !player
			|| player->Dormant( )
			|| player->IsDead( )
			|| player->IsTeammate( ) )
			continue;

		const auto fov = Math::GetFov( view_angles, Math::CalcAngle( ctx.m_vecEyePos, player->WorldSpaceCenter( ) ) );
		if ( fov >= best_fov )
			continue;

		best_fov = fov;
		best_player = player;
	}

	if ( !best_player )
		return false;

	struct angle_data_t {
		__forceinline constexpr angle_data_t( ) = default;

		__forceinline angle_data_t( const float yaw ) : m_yaw{ yaw } {}

		int		m_dmg{ };
		float	m_yaw{ }, m_dist{ };
		bool	m_can_do_dmg{ };
	};

	std::array< angle_data_t, 3u > angles{
		{
			{ std::remainder( yaw, 360.f ) },
		{ std::remainder( yaw + 90.f, 360.f ) },
		{ std::remainder( yaw - 90.f, 360.f ) }
		}
	};

	constexpr auto k_range = 30.f;

	auto enemy_shoot_pos = best_player->m_vecOrigin( );

	enemy_shoot_pos.z += 64.f;

	bool valid{ };

	const auto& local_shoot_pos = ctx.m_vecEyePos;
	for ( auto& angle : angles ) {
		const auto rad_yaw = DEG2RAD( angle.m_yaw );

		const auto pen_data = Features::Autowall.FireEmulated(
			best_player, ctx.m_pLocal, enemy_shoot_pos,
			{
				local_shoot_pos.x + std::cos( rad_yaw ) * k_range,
				local_shoot_pos.y + std::sin( rad_yaw ) * k_range,
				local_shoot_pos.z
			}
		);

		if ( pen_data.dmg < 1 )
			continue;

		angle.m_dmg = pen_data.dmg;

		angle.m_can_do_dmg = angle.m_dmg > 0;

		if ( !angle.m_can_do_dmg )
			continue;

		valid = true;
	}

	if ( valid ) {
		float best_dmg{ };
		std::size_t best_index{ };

		for ( std::size_t i{ }; i < angles.size( ); ++i ) {
			const auto& angle = angles.at( i );
			if ( !angle.m_can_do_dmg
				|| angle.m_dmg <= best_dmg )
				continue;

			best_dmg = angle.m_dmg;
			best_index = i;
		}

		const auto& best_angle = angles.at( best_index );

		if ( Config::Get<int>( Vars.AntiaimFreestand ) == 2 ) {
			yaw = best_angle.m_yaw;
			return true;
		}
		else {
			const auto diff = Math::AngleDiff( yaw, best_angle.m_yaw );

			Invert = diff >= 0.f;
		}

		return false;
	}

	valid = false;

	constexpr auto k_step = 4.f;

	for ( auto& angle : angles ) {
		const auto rad_yaw = DEG2RAD( angle.m_yaw );

		const Vector dst{
			local_shoot_pos.x + std::cos( rad_yaw ) * k_range,
			local_shoot_pos.y + std::sin( rad_yaw ) * k_range,
			local_shoot_pos.z
		};

		auto dir = dst - enemy_shoot_pos;

		const auto len = dir.NormalizeInPlace( );
		if ( len <= 0.f )
			continue;

		for ( float i{ }; i < len; i += k_step ) {
			const auto contents = Interfaces::EngineTrace->GetPointContents( local_shoot_pos + dir * i, MASK_SHOT_HULL );
			if ( !( contents & MASK_SHOT_HULL ) )
				continue;

			auto mult = 1.f;

			if ( i > ( len * 0.5f ) )
				mult = 1.25f;

			if ( i > ( len * 0.75f ) )
				mult = 1.25f;

			if ( i > ( len * 0.9f ) )
				mult = 2.f;

			angle.m_dist += k_step * mult;

			valid = true;
		}
	}

	if ( !valid )
		return false;

	if ( std::abs( angles.at( 0u ).m_dist - angles.at( 1u ).m_dist ) >= 10.f
		|| std::abs( angles.at( 0u ).m_dist - angles.at( 2u ).m_dist ) >= 10.f ) {
		std::sort(
			angles.begin( ), angles.end( ),
			[ ]( const angle_data_t& a, const angle_data_t& b ) {
				return a.m_dist > b.m_dist;
			}
		);

		const auto& best_angle = angles.front( );
		if ( best_angle.m_dist > 400.f )
			return false;

		if ( Config::Get<int>( Vars.AntiaimFreestand ) == 2 ) {
			yaw = best_angle.m_yaw;
			return true;
		}
		else {
			const auto diff = Math::AngleDiff( yaw, best_angle.m_yaw );

			Invert = diff >= 0.f;
		}

		return false;
	}

	return false;
}*/