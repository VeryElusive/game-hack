#include "visuals.h"

// this file is pasted ye who cares

void CExtendedESP::Start( ) {
	if ( !ctx.m_pLocal )
		return;

	// no active sounds
	if ( !utlCurSoundList.Count( ) )
		return;

	// accumulate sounds for esp correction
	for ( int iter = 0; iter < utlCurSoundList.Count( ); iter++ ) {
		SoundInfo_t& sound = utlCurSoundList[ iter ];
		if ( sound.nSoundSource < 1 || // World
			sound.nSoundSource > 64 )   // Most likely invalid
			continue;

		const auto player = static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( sound.nSoundSource ) );

		if ( !player || !sound.vecOrigin || !player->IsPlayer( ) || player->IsTeammate( ) || sound.vecOrigin->IsZero( ) )
			continue;

		if ( !ValidSound( sound ) )
			continue;

		SetupAdjustPlayer( player, sound );

		m_cSoundPlayers[ sound.nSoundSource - 1 ].Override( sound );
	}

	for ( int iter = 1; iter < 64; ++iter ) {
		const auto player = static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( iter ) );
		if ( !player || !player->IsPlayer( ) || player->IsDead( ) )
			continue;

		if ( !player->Dormant( ) )
			m_cSoundPlayers[ player->Index( ) - 1 ].valid = false;
		else
			AdjustPlayer( player );
	}

	m_utlvecSoundBuffer = utlCurSoundList;
}

bool CExtendedESP::ValidSound( SoundInfo_t& sound ) {
	//  We don't want the sound to keep following client's predicted origin.
	for ( int iter = 0; iter < m_utlvecSoundBuffer.Count( ); iter++ ) {
		const SoundInfo_t& cached_sound = m_utlvecSoundBuffer[ iter ];
		if ( cached_sound.nGuid == sound.nGuid ) {
			return false;
		}
	}

	return true;
}

void CExtendedESP::GetActiveSounds( ) {
	utlCurSoundList.RemoveAll( );
	Interfaces::EngineSound->GetActiveSounds( utlCurSoundList );
}

void CExtendedESP::AdjustPlayer( CBasePlayer* player )
{
	if ( !player )
		return;
	auto& sound_player = m_cSoundPlayers[ player->Index( ) - 1];

	if ( Interfaces::Globals->flRealTime - sound_player.m_iReceiveTime > 1.f )
		return;

	if ( !sound_player.valid )
		return;

	player->m_fFlags( ) = sound_player.m_nFlags;
	player->m_vecOrigin( ) = sound_player.m_vecOrigin;
	player->SetAbsOrigin( sound_player.m_vecOrigin );

	Features::Visuals.PlayerESP.Entries.at( player->Index( ) - 1 ).Alpha = 1.f;
}


void CExtendedESP::SetupAdjustPlayer( CBasePlayer* player, SoundInfo_t& sound ) {
	if ( !player )
		return;

	Vector src3D, dst3D;
	CGameTrace tr;
	CTraceFilter filter{ player };

	src3D = ( *sound.vecOrigin ) + Vector( 0, 0, 1 ); // So they dont dig into ground incase shit happens /shrug
	dst3D = src3D - Vector( 0, 0, 500 );
	Ray_t ray{ src3D, dst3D };

	Interfaces::EngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

	/* Corrects origin and important flags. */

	// Player stuck, idk how this happened
	if ( tr.bAllSolid )
		m_cSoundPlayers[ sound.nSoundSource - 1].m_iReceiveTime = -1;

	*sound.vecOrigin = ( ( tr.flFraction < 0.97f ) ? tr.vecEnd : *sound.vecOrigin );
	m_cSoundPlayers[ sound.nSoundSource - 1 ].m_nFlags = player ? player->m_fFlags( ) : 0;
	m_cSoundPlayers[ sound.nSoundSource - 1 ].m_nFlags |= ( tr.flFraction < 0.50f ? FL_DUCKING : 0 ) | ( tr.flFraction != 1.f ? FL_ONGROUND : 0 );   // Turn flags on
	m_cSoundPlayers[ sound.nSoundSource - 1 ].m_nFlags &= ( tr.flFraction > 0.50f ? ~FL_DUCKING : 0 ) | ( tr.flFraction == 1.f ? ~FL_ONGROUND : 0 ); // Turn flags off
}