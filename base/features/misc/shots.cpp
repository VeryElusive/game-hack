#include "shots.h"

void CShots::ProcessShots( ) {
	if ( m_vecShots.empty( ) )
		return;

	if ( !ctx.m_pLocal )
		return m_vecShots.clear( );

	if ( !ctx.m_pWeapon )
		return;

	m_vecShots.erase(
		std::remove_if(
			m_vecShots.begin( ), m_vecShots.end( ),
			[ & ]( const Shot_t& shot ) -> bool {
				const auto remove{ Interfaces::Globals->flRealTime - shot.m_flFireTime > 1.f };

				if ( !shot.m_iServerProcessTick
					&& remove ) {
					if ( ctx.m_pLocal->IsDead( ) )
						Features::Logger.Log( _( "local player died before server processed shots" ), true );
					else
						Features::Logger.Log( _( "shot did not register" ), true );
				}
					
				return remove;
			}
		),
		m_vecShots.end( )
				);

	// new packet eyeangles is different: eyeangles fix miss
	// max intersects: lagcompensation miss

	if ( m_vecShots.empty( ) )
		return;

	ctx.m_bInCreatemove = true;

	for ( auto it{ m_vecShots.begin( ) }; it != m_vecShots.end( ); it = std::next( it ) ) {
		auto& shot{ *it };
		if ( !shot.m_pPlayer ) {
			NONEXISTANT:
			Features::Logger.Log( _( "player no longer exists" ), true );
			it = m_vecShots.erase( it );
			if ( m_vecShots.empty( ) )
				break;

			continue;
		}

		if ( !shot.m_iServerProcessTick )
			continue;


		const auto playerInfo{ Interfaces::Engine->GetPlayerInfo( shot.m_pPlayer->Index( ) ) };
		if ( !playerInfo.has_value( ) )
			goto NONEXISTANT;// cbf to repaste the same shit

		if ( shot.m_pPlayer->IsDead( ) ) {
			if ( shot.m_bHitPlayer )
				Features::Logger.Log( ( _( "hit " ) + ( std::string )playerInfo->szName 
					+ _( " in the " ) + Features::Ragebot.HitgroupToString( shot.m_iServerHitgroup ) 
					+ _( " for " ) + std::to_string( shot.m_iServerDamage ).c_str( ) ) + _( " damage" ), true );
			else
				Features::Logger.Log( _( "target died before server processed shot" ), true );

			it = m_vecShots.erase( it );
			if ( m_vecShots.empty( ) )
				break;

			continue;
		}

		LagBackup_t backup{ shot.m_pPlayer };
		CGameTrace trace{ };

		shot.m_pRecord->Apply( shot.m_pPlayer );
		std::memcpy( shot.m_pPlayer->m_CachedBoneData( ).Base( ), shot.m_pMatrix, shot.m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

		Interfaces::EngineTrace->ClipRayToEntity(
			{ shot.m_vecStart, shot.m_vecServerEnd },
			MASK_SHOT_PLAYER, shot.m_pPlayer, &trace
		);

		/*if ( shot.m_flYaw != -1
			&& std::abs( shot.m_pPlayer->m_angEyeAngles( ).y - shot.m_flYaw ) > 5.f ) {
			Interfaces::ConVar->ConsoleColorPrintf( ACCENT, _( "[ HAVOC ] " ) );
			const auto& yawRecords{ Features::AnimSys.m_arrEntries.at( shot.m_pPlayer->Index( ) - 1 ).m_flPreviousYaws };

			Interfaces::ConVar->ConsoleColorPrintf( Color( 250, 107, 110 ), _( "Eyeangles differed | " ) );
			Interfaces::ConVar->ConsoleColorPrintf( Color( 250, 107, 110 ),
				( _( "pred: " ) + std::to_string( int( shot.m_flYaw ) ) + _( " actual: " ) + std::to_string( int( shot.m_pPlayer->m_angEyeAngles( ).y ) )
					+ "\n" ).c_str( ) );
		}*/

		auto& entry{ Features::AnimSys.m_arrEntries.at( shot.m_pPlayer->Index( ) - 1 ) };

		if ( shot.m_bHitPlayer ) {
			if ( shot.m_iServerHitgroup != shot.m_iHitgroup ) {
				// we misresolved but still hit the player
				if ( trace.iHitGroup == shot.m_iHitgroup
					&& trace.pHitEntity == shot.m_pPlayer ) {

					std::memcpy( shot.m_pPlayer->m_CachedBoneData( ).Base( ), shot.m_pRecord->m_cAnimData.m_arrSides.at( shot.m_iResolverSide ).m_pMatrix, shot.m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

					const auto backupCurtime{ Interfaces::Globals->flCurTime };
					Interfaces::Globals->flCurTime = shot.m_flCurTime;
					shot.m_pPlayer->ClampBonesInBBOX( shot.m_pPlayer->m_CachedBoneData( ).Base( ), BONE_USED_BY_SERVER );
					Interfaces::Globals->flCurTime = backupCurtime;

					Interfaces::EngineTrace->ClipRayToEntity(
						{ shot.m_vecStart, shot.m_vecServerEnd },
						MASK_SHOT_PLAYER, shot.m_pPlayer, &trace
					);

					if ( trace.pHitEntity != shot.m_pPlayer
						|| trace.iHitGroup != shot.m_iServerHitgroup ) {
						std::string log{ _( "hit incorrect hitbox due to inconsistency" ) };
						if ( shot.m_flYaw != shot.m_pPlayer->m_angEyeAngles( ).y )
							log += " | YAW " + std::to_string( int( shot.m_flYaw ) ) + " - " + std::to_string( int( shot.m_pPlayer->m_angEyeAngles( ).y ) );

						if ( shot.m_cCBBShit.m_flCBBMaxz != shot.m_pPlayer->m_flNewBoundsMaxs( ) )
							log += " | MAXS " + std::to_string( shot.m_cCBBShit.m_flCBBMaxz ) + " - " + std::to_string( shot.m_pPlayer->m_flNewBoundsMaxs( ) );

						if ( shot.m_cCBBShit.m_flCBBTime != shot.m_pPlayer->m_flNewBoundsTime( ) )
							log += " | TIME " + std::to_string( shot.m_cCBBShit.m_flCBBTime ) + " - " + std::to_string( shot.m_pPlayer->m_flNewBoundsTime( ) );

						if ( shot.m_cCBBShit.m_flViewOffsetZ != shot.m_pPlayer->m_vecViewOffset( ).z )
							log += " | VIEW " + std::to_string( shot.m_cCBBShit.m_flViewOffsetZ ) + " - " + std::to_string( shot.m_pPlayer->m_vecViewOffset( ).z );

						if ( ( shot.m_cCBBShit.m_hGroundEntity != INVALID_EHANDLE_INDEX ) != ( shot.m_pPlayer->m_hGroundEntity( ) != INVALID_EHANDLE_INDEX ) )
							log += " | GENT " + std::to_string( ( shot.m_cCBBShit.m_hGroundEntity != INVALID_EHANDLE_INDEX ) ) + " - " + std::to_string( ( shot.m_pPlayer->m_hGroundEntity( ) != INVALID_EHANDLE_INDEX ) );

						Features::Logger.Log( log, false );
					}
					else {
						std::memcpy( shot.m_pPlayer->m_CachedBoneData( ).Base( ), shot.m_pRecord->m_cAnimData.m_arrSides.at( SWAP_RESIK_SIDE( shot.m_iResolverSide ) ).m_pMatrix, shot.m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

						Interfaces::Globals->flCurTime = shot.m_flCurTime;
						shot.m_pPlayer->ClampBonesInBBOX( shot.m_pPlayer->m_CachedBoneData( ).Base( ), BONE_USED_BY_SERVER );
						Interfaces::Globals->flCurTime = backupCurtime;

						Interfaces::EngineTrace->ClipRayToEntity(
							{ shot.m_vecStart, shot.m_vecServerEnd },
							MASK_SHOT_PLAYER, shot.m_pPlayer, &trace
						);

						if ( trace.pHitEntity == shot.m_pPlayer
							&& trace.iHitGroup == shot.m_iServerHitgroup ) {
							Features::Logger.Log( _( "hit incorrect hitbox due to resolver" ), false );
							entry.m_iResolverSide = SWAP_RESIK_SIDE( shot.m_iResolverSide );

							if ( entry.m_bLBYResolved )
								entry.m_bInvertResolverOrientation = !entry.m_bInvertResolverOrientation;

							entry.m_bLBYResolved = false;
						}
						else {
							const auto lastSide{ shot.m_iResolverSide == 0 ? 1 : 0 };

							std::memcpy( shot.m_pPlayer->m_CachedBoneData( ).Base( ), shot.m_pRecord->m_cAnimData.m_arrSides.at( lastSide ).m_pMatrix, shot.m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

							Interfaces::Globals->flCurTime = shot.m_flCurTime;
							shot.m_pPlayer->ClampBonesInBBOX( shot.m_pPlayer->m_CachedBoneData( ).Base( ), BONE_USED_BY_SERVER );
							Interfaces::Globals->flCurTime = backupCurtime;

							Interfaces::EngineTrace->ClipRayToEntity(
								{ shot.m_vecStart, shot.m_vecServerEnd },
								MASK_SHOT_PLAYER, shot.m_pPlayer, &trace
							);

							if ( trace.pHitEntity == shot.m_pPlayer
								&& trace.iHitGroup == shot.m_iServerHitgroup )
								Features::Logger.Log( _( "hit incorrect hitbox due to unknown" ), false );
							else {
								Features::Logger.Log( _( "hit incorrect hitbox due to resolver" ), false );
								entry.m_iResolverSide = lastSide;

								if ( entry.m_bLBYResolved )
									entry.m_bInvertResolverOrientation = !entry.m_bInvertResolverOrientation;

								entry.m_bLBYResolved = false;
							}
						}
					}
				}
				else
					Features::Logger.Log( _( "hit incorrect hitbox due to spread" ), false );
			}

			Features::Logger.Log( ( _( "hit " ) + ( std::string )playerInfo->szName
				+ _( " in " ) + Features::Ragebot.HitgroupToString( shot.m_iServerHitgroup )
				+ _( " for " ) + std::to_string( shot.m_iServerDamage ).c_str( ) ) + _( " damage" ), true );
		}
		else {
			if ( trace.pHitEntity == shot.m_pPlayer ) {
				// new eye yaw was networked by now
				std::memcpy( shot.m_pPlayer->m_CachedBoneData( ).Base( ), shot.m_pRecord->m_cAnimData.m_arrSides.at( shot.m_iResolverSide ).m_pMatrix, shot.m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

				const auto backupCurtime{ Interfaces::Globals->flCurTime };
				Interfaces::Globals->flCurTime = shot.m_flCurTime;
				shot.m_pPlayer->ClampBonesInBBOX( shot.m_pPlayer->m_CachedBoneData( ).Base( ), BONE_USED_BY_SERVER );
				Interfaces::Globals->flCurTime = backupCurtime;

				Interfaces::EngineTrace->ClipRayToEntity(
					{ shot.m_vecStart, shot.m_vecServerEnd },
					MASK_SHOT_PLAYER, shot.m_pPlayer, &trace
				);

				if ( trace.pHitEntity != shot.m_pPlayer ) {
					std::string log{ _( "missed shot due to inconsistency" ) };
					if ( shot.m_flYaw != shot.m_pPlayer->m_angEyeAngles( ).y )
						log += " | YAW " + std::to_string( int( shot.m_flYaw ) ) + " - " + std::to_string( int( shot.m_pPlayer->m_angEyeAngles( ).y ) );

					if ( shot.m_cCBBShit.m_flCBBMaxz != shot.m_pPlayer->m_flNewBoundsMaxs( ) )
						log += " | MAXS " + std::to_string( shot.m_cCBBShit.m_flCBBMaxz ) + " - " + std::to_string( shot.m_pPlayer->m_flNewBoundsMaxs( ) );

					if ( shot.m_cCBBShit.m_flCBBTime != shot.m_pPlayer->m_flNewBoundsTime( ) )
						log += " | TIME " + std::to_string( shot.m_cCBBShit.m_flCBBTime ) + " - " + std::to_string( shot.m_pPlayer->m_flNewBoundsTime( ) );

					if ( shot.m_cCBBShit.m_flViewOffsetZ != shot.m_pPlayer->m_vecViewOffset( ).z )
						log += " | VIEW " + std::to_string( shot.m_cCBBShit.m_flViewOffsetZ ) + " - " + std::to_string( shot.m_pPlayer->m_vecViewOffset( ).z );

					if ( ( shot.m_cCBBShit.m_hGroundEntity != INVALID_EHANDLE_INDEX ) != ( shot.m_pPlayer->m_hGroundEntity( ) != INVALID_EHANDLE_INDEX ) )
						log += " | GENT " + std::to_string( ( shot.m_cCBBShit.m_hGroundEntity != INVALID_EHANDLE_INDEX ) ) + " - " + std::to_string( ( shot.m_pPlayer->m_hGroundEntity( ) != INVALID_EHANDLE_INDEX ) );

					Features::Logger.Log( log, true );
				}
				else {
					std::memcpy( shot.m_pPlayer->m_CachedBoneData( ).Base( ), shot.m_pRecord->m_cAnimData.m_arrSides.at( SWAP_RESIK_SIDE( shot.m_iResolverSide ) ).m_pMatrix, shot.m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

					Interfaces::Globals->flCurTime = shot.m_flCurTime;
					shot.m_pPlayer->ClampBonesInBBOX( shot.m_pPlayer->m_CachedBoneData( ).Base( ), BONE_USED_BY_SERVER );
					Interfaces::Globals->flCurTime = backupCurtime;

					Interfaces::EngineTrace->ClipRayToEntity(
						{ shot.m_vecStart, shot.m_vecServerEnd },
						MASK_SHOT_PLAYER, shot.m_pPlayer, &trace
					);

					if ( trace.pHitEntity != shot.m_pPlayer ) {
						Features::Logger.Log( _( "missed shot due to resolver" ), true );
						entry.m_iResolverSide = SWAP_RESIK_SIDE( shot.m_iResolverSide );

						if ( entry.m_bLBYResolved )
							entry.m_bInvertResolverOrientation = !entry.m_bInvertResolverOrientation;

						entry.m_bLBYResolved = false;
					}
					else {
						const auto lastSide{ shot.m_iResolverSide == 0 ? 1 : 0 };

						// TODO: decipher if it was an extrapolation issue that caused us to miss by shifting the bones to the networked origin
						if ( shot.m_bExtrapolated 
							&& ( ( !Config::Get<bool>( Vars.RagebotLagcompensation ) && int( shot.m_pRecord->m_cAnimData.m_vecOrigin != shot.m_pPlayer->m_vecOrigin( ) ) )
								|| ( Config::Get<bool>( Vars.RagebotLagcompensation ) && entry.m_pRecords.size( ) && int( shot.m_pRecord->m_cAnimData.m_vecOrigin != entry.m_pRecords.back( )->m_cAnimData.m_vecOrigin ) ) ) ) {
							Interfaces::ConVar->ConsoleColorPrintf( ACCENT, _( "[ HAVOC ] " ) );
							Interfaces::ConVar->ConsoleColorPrintf( Color( 250, 107, 110 ), _( "Extrapolation origin differed | " ) );

							const auto origin{ Config::Get<bool>( Vars.RagebotLagcompensation ) ? entry.m_pRecords.back( )->m_cAnimData.m_vecOrigin : shot.m_pPlayer->m_vecOrigin( ) };

							Interfaces::ConVar->ConsoleColorPrintf( Color( 250, 107, 110 ),
								( _( "difference: " ) + std::to_string( int( ( shot.m_pRecord->m_cAnimData.m_vecOrigin - origin ).Length( ) ) )
									+ "\n" ).c_str( ) );
						}

						std::memcpy( shot.m_pPlayer->m_CachedBoneData( ).Base( ), shot.m_pRecord->m_cAnimData.m_arrSides.at( lastSide ).m_pMatrix, shot.m_pPlayer->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

						Interfaces::Globals->flCurTime = shot.m_flCurTime;
						shot.m_pPlayer->ClampBonesInBBOX( shot.m_pPlayer->m_CachedBoneData( ).Base( ), BONE_USED_BY_SERVER );
						Interfaces::Globals->flCurTime = backupCurtime;

						Interfaces::EngineTrace->ClipRayToEntity(
							{ shot.m_vecStart, shot.m_vecServerEnd },
							MASK_SHOT_PLAYER, shot.m_pPlayer, &trace
						);

						if ( trace.pHitEntity == shot.m_pPlayer ) {
							const auto& records{ Features::AnimSys.m_arrEntries.at( shot.m_pPlayer->Index( ) - 1 ).m_pRecords };
							if ( shot.m_bExtrapolated )
								Features::Logger.Log( _( "missed shot due to extrapolation error" ), true );
							else if ( records.size( ) && records.front( )->m_iReceiveTick > shot.m_pRecord->m_iReceiveTick )
								Features::Logger.Log( _( "missed shot due to lagcompensation" ), true );
							else
								Features::Logger.Log( _( "missed shot due to unknown" ), true );
						}
						else {
							Features::Logger.Log( _( "missed shot due to resolver" ), true );
							entry.m_iResolverSide = lastSide;

							if ( entry.m_bLBYResolved )
								entry.m_bInvertResolverOrientation = !entry.m_bInvertResolverOrientation;

							entry.m_bLBYResolved = false;
						}
					}
				}

				Features::AnimSys.m_arrEntries.at( shot.m_pPlayer->Index( ) - 1 ).m_iMissedShots++;
			}
			else {
				const auto dir{ ( shot.m_vecServerEnd - shot.m_vecStart ).Normalized( ) };

				const auto vecEnd{ shot.m_vecStart + ( dir * 8192.f ) };

				Interfaces::EngineTrace->ClipRayToEntity(
					{ shot.m_vecStart, vecEnd },
					MASK_SHOT_PLAYER, shot.m_pPlayer, &trace
				);

				// cheeky curb the crash with a spread miss
				if ( trace.pHitEntity == shot.m_pPlayer
					&& ctx.m_pWeaponData ) {
					const auto data{ Features::Autowall.FireBullet( ctx.m_pLocal, shot.m_pPlayer, ctx.m_pWeaponData,
						ctx.m_pWeapon->m_iItemDefinitionIndex( ) == WEAPON_TASER,
						shot.m_vecStart, vecEnd, true ) };

					if ( data.dmg >= 1 )
						Features::Logger.Log( _( "missed shot due to autowall issue" ), true );
					else
						Features::Logger.Log( _( "missed shot due to spread" ), true );
				}
				else
					Features::Logger.Log( _( "missed shot due to spread" ), true );
			}
		}

		backup.Apply( shot.m_pPlayer );

		it = m_vecShots.erase( it );

		if ( m_vecShots.empty( ) )
			break;
	}

	ctx.m_bInCreatemove = false;
}