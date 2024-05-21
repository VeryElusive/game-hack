#include "ragebot.h"

//https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/cstrike15/weapon_knife.cpp
//https://counterstrike.fandom.com/wiki/Knife#ConsecutiveGO (unarmored value)
int ScaleStabDamage( CBasePlayer* player, bool backStab, bool secondary ) {
	const auto firstSwing{ ( ctx.m_pWeapon->m_flNextPrimaryAttack( ) + 0.4 ) < ctx.m_flFixedCurtime };
	const auto armoured{ player->m_bHasHeavyArmor( ) || player->m_ArmorValue( ) };

	if ( !armoured ) {
		if ( secondary ) {
			if ( backStab )
				return 180;
			else
				return 65;
		}
		else {
			if ( backStab )
				return 90;
			else if ( firstSwing )
				// first swing does full damage
				return 40;
			else
				// subsequent swings do less	
				return 25;
		}
	}
	else {
		if ( secondary ) {
			if ( backStab )
				return 153;
			else
				return 55;
		}
		else {
			if ( backStab )
				return 76;
			else if ( firstSwing )
				// first swing does full damage
				return 34;
			else
				// subsequent swings do less	
				return 21;
		}
	}
}

void CRageBot::KnifeBot( CUserCmd& cmd ) {
	auto targets{ new std::vector< KnifeTarget_t> };
	for ( auto i{ 1 }; i < 64; i++ ) {
		auto player{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
		if ( !player
			|| player->IsDormant( )
			|| player->IsDead( )
			|| player->m_bGunGameImmunity( )
			|| player->IsTeammate( ) )
			continue;

		auto& entry{ Features::AnimSys.m_arrEntries.at( i - 1 ) };
		if ( entry.m_pPlayer != player )
			continue;

		if ( !entry.m_pRecords.size( ) )
			continue;

		KnifeTarget_t target{ player };

		const auto backup{ std::make_unique< LagBackup_t >( player ) };

		if ( !Config::Get<bool>( Vars.RagebotLagcompensation ) ) {
			KnifeBotTargetPlayer( cmd, target, entry.m_pRecords.back( ).get( ) );
			if ( target.m_bIsDamageable ) {
				target.m_pRecord = entry.m_pRecords.back( );
				targets->push_back( target );
			}
		}
		else {
			std::shared_ptr< LagRecord_t > bestRecord{ };

			for ( auto it{ entry.m_pRecords.rbegin( ) }; it != entry.m_pRecords.rend( ); it = std::next( it ) ) {
				const auto& record{ *it };

				if ( !record->Validity( ) )
					continue;

				const auto damageablePrev{ target.m_bIsDamageable };
				const auto backstabPrev{ target.m_bIsBackstab };

				KnifeBotTargetPlayer( cmd, target, record.get( ) );
				if ( target.m_bIsDamageable
					&& ( !damageablePrev
						|| ( target.m_bIsBackstab && !backstabPrev ) ) ) {
					bestRecord = record;
				}

				// sweet as
				if ( target.m_bIsDamageable && target.m_bIsBackstab && target.m_bCanHitSecondary )
					break;
			}

			target.m_pRecord = bestRecord;

			if ( target.m_bIsDamageable )
				targets->push_back( target );
		}

		backup->Apply( player );
	}

	KnifeTarget_t* bestTarget{ };
	for ( auto& target : *targets ) {
		if ( !bestTarget ) {
			bestTarget = &target;
			continue;
		}

		if ( ScaleStabDamage( bestTarget->m_pPlayer, target.m_bIsBackstab, target.m_bCanHitSecondary ) >= target.m_pPlayer->m_iHealth( ) )
			bestTarget = &target;
	}

	if ( bestTarget ) {
		const auto backup{ std::make_unique< LagBackup_t >( bestTarget->m_pPlayer ) };

		bestTarget->m_pRecord->Apply( bestTarget->m_pPlayer, bestTarget->m_pRecord->m_iResolverSide );
		bestTarget->m_pPlayer->SetAbsAngles( { 0, bestTarget->m_pRecord->m_cAnimData.m_arrSides.at( bestTarget->m_pRecord->m_iResolverSide ).m_cAnimState.flAbsYaw, 0 } );
		
		bestTarget->m_pPlayer->ClampBonesInBBOX( bestTarget->m_pPlayer->m_CachedBoneData( ).Base( ), BONE_USED_BY_SERVER );
		const auto hitboxSet{ bestTarget->m_pPlayer->m_pStudioHdr( )->pStudioHdr->GetHitboxSet( bestTarget->m_pPlayer->m_nHitboxSet( ) ) };
		const auto hitbox{ hitboxSet->GetHitbox( HITBOX_CHEST ) };

		const auto point{ Math::VectorTransform( ( hitbox->vecBBMin + hitbox->vecBBMax ) / 2.f, bestTarget->m_pPlayer->m_CachedBoneData( ).Base( )[hitbox->iBone] ) };

		QAngle angle{ };
		Math::VectorAngles( point - ctx.m_vecEyePos, angle );

		const auto dmgSwing{ ScaleStabDamage( bestTarget->m_pPlayer, bestTarget->m_bIsBackstab, false ) };
		const auto dmgStab{ ScaleStabDamage( bestTarget->m_pPlayer, bestTarget->m_bIsBackstab, true ) };

		if ( ( dmgSwing >= bestTarget->m_pPlayer->m_iHealth( )
			|| dmgStab < bestTarget->m_pPlayer->m_iHealth( )
			/* || !bestTarget->m_bCanHitSecondary*/ )// TODO: this fucks up back stabbing at full hp
			&& ctx.m_bCanShoot ) {
			cmd.iButtons |= IN_ATTACK;
			cmd.iButtons &= ~IN_ATTACK2;
			cmd.iButtons &= ~IN_USE;

			//bestTarget->m_pPlayer->m_iHealth( ) -= ScaleStabDamage( bestTarget->m_pPlayer, bestTarget->m_bIsBackstab, false );

			cmd.viewAngles = angle;
			cmd.iTickCount = TIME_TO_TICKS( bestTarget->m_pRecord->m_cAnimData.m_flSimulationTime + ctx.m_flLerpTime );

			const auto info{ Interfaces::Engine->GetPlayerInfo( bestTarget->m_pPlayer->Index( ) ) };
			Features::Logger.Log( _( "shivved " ) + std::string( info->szName ) +
				_( " | backtrack: " ) + std::to_string( Interfaces::ClientState->iServerTick - bestTarget->m_pRecord->m_iReceiveTick ) + _( " ticks" ), true );
		}
		else {
			// delay if we have to
			if ( ctx.m_pLocal->CanShoot( true ) && bestTarget->m_bCanHitSecondary ) {
				cmd.iButtons |= IN_ATTACK2;
				cmd.iButtons &= ~IN_ATTACK;
				cmd.iButtons &= ~IN_USE;

				//bestTarget->m_pPlayer->m_iHealth( ) -= ScaleStabDamage( bestTarget->m_pPlayer, bestTarget->m_bIsBackstab, true );

				cmd.viewAngles = angle;
				cmd.iTickCount = TIME_TO_TICKS( bestTarget->m_pRecord->m_cAnimData.m_flSimulationTime + ctx.m_flLerpTime );

				ctx.m_bSendPacket = true;

				const auto info{ Interfaces::Engine->GetPlayerInfo( bestTarget->m_pPlayer->Index( ) ) };
				Features::Logger.Log( _( "shanked " ) + std::string( info->szName ) +
					_( " | backtrack: " ) + std::to_string( Interfaces::ClientState->iServerTick - bestTarget->m_pRecord->m_iReceiveTick ) + _( " ticks" ), true );
			}
		}

		backup->Apply( bestTarget->m_pPlayer );
	}

	delete targets;
}

inline float Vector2DNormalize( Vector2D& v ) {
	if ( !v.IsValid( ) )
		return 0.f;

	float l = v.Length( );
	if ( l != 0.f )
		v /= l;
	else
		v.x = v.y = 0.0f;

	return l;
}

bool IsBackStab( CBasePlayer* player ) {
	Vector vTragetForward;

	Math::AngleVectors( player->GetAbsAngles( ), &vTragetForward );// firebullet is ran before animstate->update

	Vector2D vecLOS = ( player->GetAbsOrigin( ) - ctx.m_pLocal->m_vecOrigin( ) ).ToVector2D( );
	Vector2DNormalize( vecLOS );

	float flDot = vecLOS.Dot( vTragetForward.ToVector2D( ) );

	//Triple the damage if we are stabbing them in the back.
	return flDot > 0.475f;
}

void CRageBot::KnifeBotTargetPlayer( CUserCmd& cmd, KnifeTarget_t& target, LagRecord_t* record ) {
	record->Apply( target.m_pPlayer );
	target.m_pPlayer->SetAbsAngles( { 0,record->m_cAnimData.m_arrSides.at( record->m_iResolverSide ).m_cAnimState.flAbsYaw, 0 } );
	
	const auto hitboxSet{ target.m_pPlayer->m_pStudioHdr( )->pStudioHdr->GetHitboxSet( target.m_pPlayer->m_nHitboxSet( ) ) };
	const auto hitbox{ hitboxSet->GetHitbox( HITBOX_CHEST ) };

	const auto point{ Math::VectorTransform( ( hitbox->vecBBMin + hitbox->vecBBMax ) / 2.f, target.m_pPlayer->m_CachedBoneData( ).Base( )[ hitbox->iBone ] ) };
	const auto dir{ ( point - ctx.m_vecEyePos ).Normalized( ) };

	const auto damageable{ KnifeBotCanHitTarget( cmd, target, dir, false ) };
	if ( damageable )
		target.m_bIsDamageable = true;

	const auto backStab{ IsBackStab( target.m_pPlayer ) };

	if ( backStab )
		target.m_bIsBackstab = true;

	if ( damageable ) {
		if ( KnifeBotCanHitTarget( cmd, target, dir, true ) )
			target.m_bCanHitSecondary = true;
	}
}

bool CRageBot::KnifeBotCanHitTarget( CUserCmd& cmd, KnifeTarget_t& target, Vector dir, bool secondary ) {
	const auto minDistance{ secondary ? 32.f : 48.f };

	const auto end{ ctx.m_vecEyePos + ( dir * minDistance ) };

	CTraceFilter traceFilter{ ctx.m_pLocal };
	CGameTrace trace;

	Interfaces::EngineTrace->TraceRay(
		{ ctx.m_vecEyePos, end }, MASK_SOLID,
		&traceFilter, &trace
	);

	if ( trace.flFraction >= 1.0 )
		return false;

	if ( trace.pHitEntity != target.m_pPlayer )
		return false;

	return true;
}