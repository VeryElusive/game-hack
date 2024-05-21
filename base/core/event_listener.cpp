#include "event_listener.h"
#include "../utils/xorstr.h"
// sequence:
// weapon_fire -> bullet_impact -> player_hurt

void CEventListener::Setup( const std::deque<const char*>& arrEvents ) {
	if ( arrEvents.empty( ) )
		return;

	for ( auto szEvent : arrEvents ) {
		Interfaces::GameEvent->AddListener( this, szEvent, false );
	}
}

void CEventListener::Destroy( ) {
	Interfaces::GameEvent->RemoveListener( this );
}

void BulletImpact( IGameEvent* pEvent ) {
	if ( !pEvent || !ctx.m_pLocal )
		return;

	Vector pos{ pEvent->GetFloat( _( "x" ) ), pEvent->GetFloat( _( "y" ) ), pEvent->GetFloat( _( "z" ) ) };
	auto ent{ static_cast<CBasePlayer*>( Interfaces::ClientEntityList->GetClientEntity( Interfaces::Engine->GetPlayerForUserID( pEvent->GetInt( _( "userid" ) ) ) ) ) };

	if ( ent != ctx.m_pLocal ) {
		if ( ent->IsTeammate( ) )
			return;

		const auto& col = Config::Get<Color>( Vars.VisServerBulletImpactsCol );
		if ( Config::Get<bool>( Vars.VisServerBulletImpacts ) ) {
			Interfaces::DebugOverlay->AddBoxOverlay( pos, Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ), QAngle( 0.f, -0.f, 0.f ),
				col.Get<COLOR_R>( ), col.Get<COLOR_G>( ), col.Get<COLOR_B>( ), col.Get<COLOR_A>( ), 4.0f );
		}

		if ( Config::Get<bool>( Vars.VisOtherBulletTracers ) )
			Features::Visuals.BulletTracers.AddTracer( ent->GetEyePosition( ent->m_angEyeAngles( ).y , ent->m_angEyeAngles( ).x ), pos, ent );
			
		return;
	}

	if ( const auto shot = Features::Shots.LastUnprocessed( ) ) {
		shot->m_vecServerEnd = pos;

		if ( Config::Get<bool>( Vars.VisLocalBulletTracers ) )
			Features::Visuals.BulletTracers.AddTracer( shot->m_vecStart, pos, ent );
	}
	else 	if ( Config::Get<bool>( Vars.VisLocalBulletTracers ) )
		Features::Visuals.BulletTracers.AddTracer( ctx.m_vecEyePos, pos, ent );

	const auto col = Config::Get<Color>( Vars.VisLocalBulletImpactsCol );

	if ( Config::Get<bool>( Vars.VisLocalBulletImpacts ) )
		Interfaces::DebugOverlay->AddBoxOverlay( pos, Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ), QAngle( 0.f, -0.f, 0.f ),
			col.Get<COLOR_R>( ), col.Get<COLOR_G>( ), col.Get<COLOR_B>( ), col.Get<COLOR_A>( ), 4.0f );
}

void RoundStart( IGameEvent* pEvent ) {
	if ( !pEvent || !ctx.m_pLocal )
		return;

	ctx.m_bClearKillfeed = true;
	ctx.m_iBombCarrier = -1;
	ctx.m_iLastShotNumber = 0;

	for ( int i{ }; i < 64; i++ ) {
		Features::Visuals.DormantESP.m_cSoundPlayers[ i ].valid = false;
		Features::Visuals.PlayerESP.Entries.at( i ).Alpha = 0;
		Features::Visuals.PlayerESP.Entries.at( i ).health = 100;
		Features::AnimSys.m_arrEntries.at( i ).OnNewRound( );
	}

	Features::Visuals.GrenadePrediction.m_vecGrenadeSimulations.clear( );

	if ( Config::Get<bool>( Vars.MiscBuyBot ) ) {
		std::string buy;

		switch ( Config::Get<int>( Vars.MiscBuyBotPrimary ) ) {
		case 1:
			buy += "buy scar20; ";
			break;
		case 2:
			buy += "buy ssg08; ";
			break;
		case 3:
			buy += "buy awp; ";
			break;
		case 4:
			buy += "buy negev; ";
			break;		
		case 5:
			buy += "buy ak-47; ";
			break;
		}

		switch ( Config::Get<int>( Vars.MiscBuyBotSecondary ) ) {
		case 1:
			buy += "buy deagle; ";
			break;
		case 2:
			buy += "buy elite; ";
			break;
		case 3:
			buy += "buy glock; ";
			break;		
		
		case 4:
			buy += "buy tec9; ";
			break;
		}

		if ( Config::Get<bool>( Vars.MiscBuyBotOtherArmor ) ) {
			buy += "buy vesthelm; ";
			buy += "buy vest; ";
		}

		if ( Config::Get<bool>( Vars.MiscBuyBotOtherKit ) )
			buy += "buy defuser; ";

		if ( Config::Get<bool>( Vars.MiscBuyBotOtherSmoke ) )
			buy += "buy smokegrenade; ";

		if ( Config::Get<bool>( Vars.MiscBuyBotOtherNade ) )
			buy += "buy hegrenade; ";


		if ( Config::Get<bool>( Vars.MiscBuyBotOtherMolotov ) )
			buy += "buy molotov; ";

		if ( Config::Get<bool>( Vars.MiscBuyBotOtherFlashbang ) )
			buy += "buy flashbang; ";

		if ( Config::Get<bool>( Vars.MiscBuyBotOtherTaser ) )
			buy += "buy taser; ";

		if ( !buy.empty( ) )
			Interfaces::Engine->ClientCmdUnrestricted( buy.c_str( ) );
	}

}

void WeaponFire( IGameEvent* pEvent ) {
	if ( !pEvent || !ctx.m_pLocal )
		return;

	if ( Interfaces::ClientEntityList->GetClientEntity( Interfaces::Engine->GetPlayerForUserID( pEvent->GetInt( _( "userid" ) ) ) ) != ctx.m_pLocal )
		return;

	if ( Features::Shots.m_vecShots.empty( ) )
		return;

	const auto shot{ std::find_if(
		Features::Shots.m_vecShots.begin( ), Features::Shots.m_vecShots.end( ),
		[ ]( const Shot_t& shot ) {
			return !shot.m_iServerProcessTick;
		}
	) };

	if ( shot == Features::Shots.m_vecShots.end( ) )
		return;

	shot->m_iServerProcessTick = Interfaces::ClientState->iServerTick;
}

void PlayerDeath( IGameEvent* pEvent ) {
	const auto user{ pEvent->GetInt( _( "userid" ) ) };
	ctx.m_iLastID = Interfaces::Engine->GetPlayerForUserID( user );
}
void PlayerHurt( IGameEvent* pEvent ) {
	static auto GetHitboxByHitGroup = [ ]( int hitgroup ) -> int
	{
		switch ( hitgroup )
		{
		case HITGROUP_HEAD:
			return HITBOX_HEAD;
		case HITGROUP_CHEST:
			return HITBOX_CHEST;
		case HITGROUP_STOMACH:
			return HITBOX_STOMACH;
		case HITGROUP_LEFTARM:
			return HITBOX_LEFT_HAND;
		case HITGROUP_RIGHTARM:
			return HITBOX_RIGHT_HAND;
		case HITGROUP_LEFTLEG:
			return HITBOX_RIGHT_CALF;
		case HITGROUP_RIGHTLEG:
			return HITBOX_LEFT_CALF;
		default:
			return HITBOX_PELVIS;
		}
	};


	if ( !pEvent || !ctx.m_pLocal )
		return;

	const auto attacker = pEvent->GetInt( _( "attacker" ) );
	const auto user = pEvent->GetInt( _( "userid" ) );
	const auto damage = pEvent->GetInt( _( "dmg_health" ) );
	const auto hitgroup = pEvent->GetInt( _( "hitgroup" ) );

	const auto attacker_id = Interfaces::Engine->GetPlayerForUserID( attacker );
	if ( attacker_id != Interfaces::Engine->GetLocalPlayer( ) )
		return;

	const auto user_id = Interfaces::Engine->GetPlayerForUserID( user );
	const auto victim = static_cast<CBasePlayer*>( Interfaces::ClientEntityList->GetClientEntity( user_id ) );
	if ( !victim )
		return;

	switch ( Config::Get<int>( Vars.MiscHitSound ) )
	{
	case 1:
		Interfaces::Surface->PlaySoundSurface( _( "buttons/arena_switch_press_02.wav" ) );
		break;
	case 2:
		Interfaces::Surface->PlaySoundSurface( ( Config::Get<std::string>( Vars.MiscCustomHitSound ) + _(".wav" ) ).c_str( ) );
		break;
	default:
		break;
	}

	const auto shot = Features::Shots.LastUnprocessed( );

	if ( !shot
		|| ( shot->m_pPlayer != victim ) ) {
		if ( victim ) {
			const auto hdr = Interfaces::ModelInfo->GetStudioModel( victim->GetModel( ) );
			if ( !hdr )
				return;

			const auto HitboxSet = hdr->GetHitboxSet( victim->m_nHitboxSet( ) );
			if ( !HitboxSet )
				return;

			const auto hitbox = HitboxSet->GetHitbox( GetHitboxByHitGroup( hitgroup ) );
			if ( !hitbox )
				return;

			const auto min = Math::VectorTransform( hitbox->vecBBMin, victim->m_CachedBoneData( ).Base( )[ hitbox->iBone ] );
			const auto max = Math::VectorTransform( hitbox->vecBBMax, victim->m_CachedBoneData( ).Base( )[ hitbox->iBone ] );

			const auto point = ( min + max ) * 0.5f;

			Features::Visuals.AddHit( { point, damage, Interfaces::Globals->flRealTime } );
		}
		return;
	}

	Features::Visuals.AddHit( { shot->m_vecPredEnd, damage, Interfaces::Globals->flRealTime } );

	shot->m_iServerHitgroup = hitgroup;
	shot->m_iServerDamage = damage;
	shot->m_bHitPlayer = true;
}

void CEventListener::FireGameEvent( IGameEvent* pEvent ) {
	if ( pEvent == nullptr )
		return;

	const FNV1A_t uNameHash = FNV1A::Hash( pEvent->GetName( ) );

	switch ( uNameHash ) {
	case FNV1A::HashConst( "bullet_impact" ): BulletImpact( pEvent ); break;
	case FNV1A::HashConst( "round_start" ): RoundStart( pEvent ); break;
	case FNV1A::HashConst( "player_hurt" ): PlayerHurt( pEvent ); break;
	case FNV1A::HashConst( "player_death" ): PlayerDeath( pEvent ); break;
	case FNV1A::HashConst( "weapon_fire" ): WeaponFire( pEvent ); break;
	default: break;
	}
}