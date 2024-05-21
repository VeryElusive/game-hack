#include "visuals.h"
#include "../animations/animation.h"

void CVisuals::Main( ) {
	if ( !ctx.m_pLocal )
		return;

	DormantESP.Start( );

	if ( Interfaces::GameResources )
		ctx.m_iBombCarrier = Interfaces::GameResources->GetC4Carrier( );
	else
		ctx.m_iBombCarrier = -1;

	for ( int i{ }; i <= Interfaces::ClientEntityList->GetHighestEntityIndex( ); i++ ) {
		const auto ent{ static_cast< CBaseEntity* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
		if ( !ent )
			continue;

		if ( ent->IsPlayer( ) )
			PlayerESP.Main( static_cast< CBasePlayer* >( ent ) );
		else {
			EntModulate( ent );
			OtherEntities( ent );
		}
	}

	auto& w{ ctx.m_ve2ScreenSize.x };
	auto& h{ ctx.m_ve2ScreenSize.y };

	if ( Config::Get<bool>( Vars.RemovalScope ) && ctx.m_pLocal->m_bIsScoped( ) 
		&& ctx.m_pWeaponData && ctx.m_pWeaponData->nWeaponType == WEAPONTYPE_SNIPER ) {
		Render::Line( Vector2D( 0, h / 2 ), Vector2D( w, h / 2 ), Color( 0, 0, 0 ) );
		Render::Line( Vector2D( w / 2, 0 ), Vector2D( w / 2, h ), Color( 0, 0, 0 ) );
	}

	ManageHitmarkers( );
	AutoPeekIndicator( );
	BulletTracers.Draw( );

	if ( !ctx.m_pLocal->IsDead( ) ) {
		if ( Config::Get<bool>( Vars.VisPenetrationCrosshair ) ) {
			const auto center = ctx.m_ve2ScreenSize / 2;
			const Color color = ctx.m_bCanPenetrate ? Color( 0, 255, 0, 155 ) : Color( 255, 0, 0, 155 );

			int add{ 2 };
			static auto aa{ Interfaces::ConVar->FindVar( "mat_antialias" ) };
			if ( aa ) {
				if ( aa->GetInt( ) )
					add = 1;
			}

			Render::Line( center - Vector2D( 1, 0 ), center + Vector2D( add, 0 ), color );
			Render::Line( center - Vector2D( 0, 1 ), center + Vector2D( 0, add ), color );
		}

		if ( Config::Get<bool>( Vars.AntiAimManualDirInd ) ) {
			const auto& col{ Config::Get<Color>( Vars.AntiaimManualCol ) };

			Vector2D
				p1{ w / 2 - 55, h / 2 + 10 },
				p2{ w / 2 - 75, h / 2 },
				p3{ w / 2 - 55, h / 2 - 10 };

			Render::Triangle( p1, p2, p3, Features::Antiaim.ManualSide == 1 ? col : Color( 125, 125, 125, 150 ) );

			p1 = { w / 2 + 55, h / 2 - 10 };
			p2 = { w / 2 + 75, h / 2 };
			p3 = { w / 2 + 55, h / 2 + 10 };

			Render::Triangle( p1, p2, p3, Features::Antiaim.ManualSide == 2 ? col : Color( 125, 125, 125, 150 ) );
		}
	}

	//GrenadePrediction.Paint( );

	GrenadePrediction.View( );
	GrenadePrediction.PaintLocal( );
}

const char* modelsForChange[ ] = {
	_( "models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl" ),
	_( "models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl" ),
	_( "models/player/custom_player/legacy/tm_jumpsuit_variantc.mdl" ),
	_( "models/player/custom_player/legacy/ctm_st6_variantm.mdl" ),
	_( "models/player/custom_player/legacy/ctm_st6_variantg.mdl" ),
	_( "models/player/custom_player/legacy/ctm_st6_variante.mdl" ),
	_( "models/player/custom_player/legacy/tm_balkan_varianth.mdl" ),
	_( "models/player/custom_player/legacy/tm_phoenix_varianth.mdl" ),
	_( "models/player/custom_player/legacy/tm_leet_variantf.mdl" ),
	_( "models/player/custom_player/legacy/tm_leet_variantg.mdl" ),
	_( "models/player/custom_player/legacy/ctm_sas_variantf.mdl" ),
	_( "models/player/custom_player/legacy/tm_anarchist.mdl" ),
	_( "models/player/custom_player/legacy/tm_anarchist_varianta.mdl" ),
};

void CVisuals::ModelChanger( ) {
	static int lastModelIndex{ };

	if ( Config::Get<bool>( Vars.MiscCustomModelChanger ) ) {
		int iModelIndex = 0;
		iModelIndex = Interfaces::ModelInfo->GetModelIndex( ( "models/player/custom_player/legacy/" + ( Config::Get<std::string>( Vars.MiscCustomModelChangerString ) ) + ".mdl" ).c_str( ) );
		if ( iModelIndex != 0
			&& lastModelIndex != iModelIndex )
			ctx.m_pLocal->SetModelIndex( iModelIndex );

		lastModelIndex = iModelIndex;
		return;
	}

	if ( ctx.m_pLocal->m_nModelIndex( ) != lastModelIndex ) {
		lastModelIndex = 0;
	}


	const auto team{ ctx.m_pLocal->m_iTeamNum( ) };

	int modelIndex{ };
	if ( team == TEAM_TT )
		modelIndex = Interfaces::ModelInfo->GetModelIndex( modelsForChange[ Config::Get<int>( Vars.MiscPlayerModelT ) ] );
	else if ( team == TEAM_CT )
		modelIndex = Interfaces::ModelInfo->GetModelIndex( modelsForChange[ Config::Get<int>( Vars.MiscPlayerModelCT ) ] );

	if ( modelIndex == -1 ) {
		if ( team == TEAM_TT ) {
			PrecacheModel( modelsForChange[ Config::Get<int>( Vars.MiscPlayerModelT ) ] );
			modelIndex = Interfaces::ModelInfo->GetModelIndex( modelsForChange[ Config::Get<int>( Vars.MiscPlayerModelT ) ] );
		}
		else if ( team == TEAM_CT ) {
			PrecacheModel( modelsForChange[ Config::Get<int>( Vars.MiscPlayerModelCT ) ] );
			modelIndex = Interfaces::ModelInfo->GetModelIndex( modelsForChange[ Config::Get<int>( Vars.MiscPlayerModelCT ) ] );
		}
	}

	if ( modelIndex != -1
		&& lastModelIndex != modelIndex ) {
		ctx.m_pLocal->SetModelIndex( modelIndex );
		lastModelIndex = modelIndex;
	}
}

void CVisuals::SkyboxChanger( ) {
	std::string sv_skyname{ Displacement::Cvars.sv_skyname->GetString( ) };

	switch ( Config::Get<int>( Vars.VisWorldSkybox ) )
	{
	case 1:
		sv_skyname = _( "cs_tibet" );
		break;
	case 2:
		sv_skyname = _( "cs_baggage_kybox_" );
		break;
	case 3:
		sv_skyname = _( "italy" );
		break;
	case 4:
		sv_skyname = _( "jungle" );
		break;
	case 5:
		sv_skyname = _( "office" );
		break;
	case 6:
		sv_skyname = _( "sky_cs15_daylight01_hdr" );
		break;
	case 7:
		sv_skyname = _( "sky_cs15_daylight02_hdr" );
		break;
	case 8:
		sv_skyname = _( "vertigoblue_hdr" );
		break;
	case 9:
		sv_skyname = _( "vertigo" );
		break;
	case 10:
		sv_skyname = _( "sky_day02_05_hdr" );
		break;
	case 11:
		sv_skyname = _( "nukeblank" );
		break;
	case 12:
		sv_skyname = _( "sky_venice" );
		break;
	case 13:
		sv_skyname = _( "sky_cs15_daylight03_hdr" );
		break;
	case 14:
		sv_skyname = _( "sky_cs15_daylight04_hdr" );
		break;
	case 15:
		sv_skyname = _( "sky_csgo_cloudy01" );
		break;
	case 16:
		sv_skyname = _( "sky_csgo_night02" );
		break;
	case 17:
		sv_skyname = _( "sky_csgo_night02b" );
		break;
	case 18:
		sv_skyname = _( "sky_csgo_night_flat" );
		break;
	case 19:
		sv_skyname = _( "sky_dust" );
		break;
	case 20:
		sv_skyname = _( "vietnam" );
		break;
	case 21:
		sv_skyname = Config::Get<std::string>( Vars.VisWorldSkyboxCustom );
		break;
	}

	static std::string last{ };
	if ( last == sv_skyname )
		return;

	last = sv_skyname;

	if ( Config::Get<int>( Vars.VisWorldSkybox ) <= 0 ) {
		if ( !Displacement::Cvars.r_3dsky->GetBool( ) )
			Displacement::Cvars.r_3dsky->SetValue( true );
	}
	else if ( Displacement::Cvars.r_3dsky->GetBool( ) )
		Displacement::Cvars.r_3dsky->SetValue( false );

	static auto fnLoadNamedSkys{ ( void( __fastcall* )( const char* ) )Displacement::Sigs.LoadNamedSkys };

	return fnLoadNamedSkys( sv_skyname.c_str( ) );
}

void CVisuals::Watermark( ) {
	const auto name = _( "Havoc " );
	const auto pstr = _( "Early access" );

	const auto name_size = Render::GetTextSize( name, Fonts::Menu );
	const auto ping_size = Render::GetTextSize( pstr, Fonts::Menu );

	const auto size = Vector2D( name_size.x + ping_size.x + 37, 20 );
	const auto pos = Vector2D( ctx.m_ve2ScreenSize.x - size.x - 20, 15 );

	if ( ctx.m_pLocal && !ctx.m_pLocal->IsDead( ) ) {
		int i{ 1 };
		for ( auto& dbg : ctx.m_strDbgLogs ) {
			Render::Text( Fonts::Menu, pos + Vector2D( 0, 20 * i ), Color( 255, 255, 255 ), 0, dbg->c_str( ) );
			++i;
		}

		if ( ctx.m_bSafeFromDefensive )
			Render::Text( Fonts::Menu, pos + Vector2D( 0, 20 * i ), Color( 255, 255, 255 ), 0, "SAFE" );


		/*for ( int i{ 1 }; i < 64; i++ ) {
			const auto player{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
			if ( !player || player->IsDead( ) || !player->IsPlayer( ) )
				continue;
			auto& entry{ Features::AnimSys.m_arrEntries.at( i - 1 ) };

			Render::Text( Fonts::Menu, pos + Vector2D( 0, 30 * i ), Color( 255, 255, 255 ), 0, std::to_string( entry.m_iLastNewCmds ).c_str( ) );
		}*/
	}

	if ( !Config::Get<bool>( Vars.MiscWatermark ) )
		return;

	Render::FilledRoundedBox( pos - Vector2D( 1, 1 ), size + 2, 5, 5, Color( 10, 10, 10 ) );
	Render::FilledRoundedBox( pos, size, 5, 5, OUTLINE_LIGHT );
	Render::FilledRoundedBox( pos + 1, size - Vector2D( 2, 2 ), 5, 5, Color( 10, 10, 10 ) );
	Render::FilledRoundedBox( pos + 2, size - Vector2D( 4, 4 ), 5, 5, BACKGROUND );

	for ( int i{ }; i < 3; ++i ) {
		Render::Line( pos + Vector2D( name_size.x + 8 + 19 + i, 2 ), pos + Vector2D( name_size.x + 8 + 4 + i, 17 ), ACCENT2 );
		Render::Line( pos + Vector2D( name_size.x + 8 + 12 + i, 2 ), pos + Vector2D( name_size.x + 8 - 3 + i, 17 ), ACCENT );
	}

	Render::Text( Fonts::Menu, pos + Vector2D( 6, 3 ), ACCENT, 0, name );
	Render::Text( Fonts::Menu, pos + Vector2D( name_size.x + 33, 3 ), ACCENT2, 0, pstr );
	/*if ( ctx.m_pLocal ) {
		Render::Text( Fonts::Menu, pos + Vector2D( 0, 30 ), Color( 255, 255, 255 ), 0, std::to_string( ctx.m_pLocal->m_pAnimState( )->flMaxBodyYaw ).c_str( ) );
		Render::Text( Fonts::Menu, pos + Vector2D( 0, 40 ), Color( 255, 255, 255 ), 0, std::to_string( ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flWeight ).c_str( ) );
		Render::Text( Fonts::Menu, pos + Vector2D( 0, 50 ), Color( 255, 255, 255 ), 0, std::to_string( ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flCycle ).c_str( ) );
	}*/

	//Render::Text( Fonts::Menu, pos + Vector2D( 0, 30 ), Color( 255, 255, 255 ), 0, std::to_string( ctx.m_iRealOutLatencyTicks ).c_str( ) );
}

void CVisuals::OtherEntities( CBaseEntity* ent ) {
	if ( !ent )
		return;

	auto client_class = ent->GetClientClass( );
	if ( !client_class )
		return;

	const auto& class_id = client_class->nClassID;


	if ( class_id == EClassIndex::CPlantedC4 ) {
		const auto owner{ static_cast< CBaseEntity* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( ent->m_hOwnerEntity( ) ) ) };
		if ( owner )
			ctx.m_iBombCarrier = owner->Index( );

		if ( strstr( client_class->szNetworkName, _( "Planted" ) ) ) {
			ctx.m_iBombCarrier = -1;
			if ( Config::Get<bool>( Vars.VisBomb ) ) {
				Vector2D world;
					if ( Math::WorldToScreen( ent->m_vecOrigin( ), world ) )
						Render::Text( Fonts::HealthESP, world, Color( 255, 255, 255 ), FONT_CENTER, _( "BOMB" ) );
			}
		}
	}

	const auto distTo = ent->GetAbsOrigin( ).DistTo( ctx.m_pLocal->GetAbsOrigin( ) );
	if ( distTo > 1500.f )
		return;

	float maxAlpha{ std::min( distTo < 1245.f ? 255.f : 1500.f - distTo, 255.f ) };

	if ( !maxAlpha )
		return;

	if ( class_id != EClassIndex::CBaseWeaponWorldModel && ( strstr( client_class->szNetworkName, _( "Weapon" ) ) ) ) {
		if ( Config::Get<bool>( Vars.VisDroppedWeapon ) ) {
			const auto& origin{ ent->m_vecOrigin( ) };
			if ( origin.IsZero( ) )
				return;

			Vector2D world;
			if ( Math::WorldToScreen( origin, world ) )
				Render::Text( Fonts::HealthESP, world,
					Config::Get<Color>( Vars.VisDroppedWeaponCol ).Set<COLOR_A>( maxAlpha ),
					FONT_CENTER, static_cast< CWeaponCSBase* >( ent )->GetGunName( ).c_str( ) );
		}
	}
	else {
		const auto owner{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientNetworkableFromHandle( ent->m_hOwnerEntity( ) ) ) };
		if ( !owner || !owner->IsPlayer( ) )
			return;

		if ( strstr( client_class->szNetworkName, _( "Projectile" ) ) ) {
			if ( ( ( owner->IsTeammate( ) && Config::Get<bool>( Vars.VisGrenadesTeam ) )
				|| ( !owner->IsTeammate( ) && Config::Get<bool>( Vars.VisGrenadesEnemy ) ) ) )
				DrawGrenade( ent, maxAlpha );

			//if ( class_id == EClassIndex::CMolotovProjectile || class_id == EClassIndex::CBaseCSGrenadeProjectile || class_id == EClassIndex::CIncendiaryGrenade )
			//	GrenadePrediction.SimulateThrownGrenade( owner, reinterpret_cast< CWeaponCSBase*>( ent ) );
		}

		if ( ( ( owner->IsTeammate( ) && Config::Get<bool>( Vars.VisGrenadesTeam ) )
			|| ( !owner->IsTeammate( ) && Config::Get<bool>( Vars.VisGrenadesEnemy ) ) )
			&& class_id == EClassIndex::CInferno ) {
			Vector min, max;
			ent->GetClientRenderable( )->GetRenderBounds( min, max );

			const auto radius = ( max - min ).Length2D( ) * 0.5f;

			DrawWrappingRing( ent, 7, _( "FIRE" ), static_cast< CBaseCSGrenadeProjectile* >( ent )->m_flSpawnTime( ), radius, maxAlpha );
		}
	}
}

void CVisuals::AutoPeekIndicator( ) {
	if ( ctx.m_pLocal->IsDead( ) )
		return;

	if ( !Config::Get<bool>( Vars.MiscAutoPeek ) )
		return;

	if ( !ctx.m_pWeapon 
		|| ctx.m_pLocal->IsDead( )
		|| ctx.m_pWeapon->IsKnife( ) || ctx.m_pWeapon->IsGrenade( ) )
		return;

	if ( !Features::Misc.AutoPeeking && !m_flAutoPeekSize )
		return;

	const float multiplier = static_cast< float >( 20.f * Interfaces::Globals->flFrameTime );

	if ( Config::Get<keybind_t>( Vars.MiscAutoPeekKey ).enabled )
		m_flAutoPeekSize += multiplier * ( 1.0f - m_flAutoPeekSize );
	else {
		if ( m_flAutoPeekSize > 0.01f )
			m_flAutoPeekSize += multiplier * -m_flAutoPeekSize;
		else
			m_flAutoPeekSize = 0.0f;
	}

	m_flAutoPeekSize = std::clamp<float>( m_flAutoPeekSize, 0.0f, 1.f );

	const auto& col = Config::Get<Color>( Vars.MiscAutoPeekCol );

	Render::WorldCircle( Features::Misc.OldOrigin, static_cast<int>( m_flAutoPeekSize * 20.0f ), col, col.Set<COLOR_A>( col.Get<COLOR_A>( ) / 5.f ) );
}

void CVisuals::DrawGrenade( CBaseEntity* ent, int maxAlpha ) {
	const auto model = ent->GetModel( );
	if ( !model ) 
		return;

	const auto hdr = Interfaces::ModelInfo->GetStudioModel( model );
	if ( !hdr ) 
		return;

	const char* model_name = hdr->szName;

	if ( !strstr( model_name, _( "thrown" ) ) && !strstr( model_name, _( "dropped" ) ) )
		return;

	const auto grenade = ( CBaseCSGrenadeProjectile* )ent;
	const char* name = _( "HE GRENADE" );

	if ( strstr( hdr->szName, _( "flash" ) ) )
		name = _( "FLASH" );
	else if ( strstr( hdr->szName, _( "smoke" ) ) ) {
		name = _( "SMOKE" );
		DrawWrappingRing( ent, 18.f, name, TICKS_TO_TIME( ( ( CWeaponCSBase* )ent )->m_nSmokeEffectTickBegin( ) ), 120.f, maxAlpha );
		return;
	}
	else if ( strstr( hdr->szName, _( "decoy" ) ) ) {
		name = _( "DECOY" );
		if ( grenade->m_nExplodeEffectTickBegin( ) )
			return;
	}
	else if ( strstr( hdr->szName, _( "incendiary" ) ) || strstr( hdr->szName, _( "molotov" ) ) )
		name = _( "FIRE" );
	else {
		if ( grenade->m_nExplodeEffectTickBegin( ) )
			return;
	}

	Vector2D world;
	if ( Math::WorldToScreen( ent->m_vecOrigin( ), world ) )
		Render::Text( Fonts::HealthESP, world, Color( 255, 255, 255, maxAlpha ), FONT_CENTER, name );
}

void CVisuals::DrawWrappingRing( CBaseEntity* entity, float seconds, const char* name, float spawntime, float radius, int maxAlpha ) {
	const auto fadeAlpha = std::min( std::max( seconds - ( Interfaces::Globals->flCurTime - spawntime ), 0.f ) * 2, 1.f );

	const auto owner = static_cast< CBasePlayer * >( Interfaces::ClientEntityList->GetClientEntityFromHandle( entity->m_hOwnerEntity( ) ) );
	if ( !owner || !owner->IsPlayer( ) )
		return;

	const auto color{ owner->IsTeammate() && owner != ctx.m_pLocal ? Config::Get<Color>( Vars.VisGrenadesTeamCol ) : Config::Get<Color>( Vars.VisGrenadesEnemyCol ) };

	Vector text_pos = entity->m_vecOrigin( );
	text_pos.z += 5.f;

	const float delta = Interfaces::Globals->flCurTime - spawntime;
	Vector2D world;
	if ( Math::WorldToScreen( entity->GetAbsOrigin( ), world ) ) {
		Vector last_pos;

		const float fill = ( seconds - delta ) / ( seconds ) * 180.f;

		for ( float rot = -fill; rot <= fill; rot += 3.f ) {
			auto rotation = rot + delta * 90.f;

			while ( rotation > 360.f )
				rotation -= 360.f;

			Vector rotated_pos = entity->m_vecOrigin( );

			rotated_pos.z -= 5.f;

			rotated_pos.x += std::cos( DEG2RAD( rotation ) ) * radius;
			rotated_pos.y += std::sin( DEG2RAD( rotation ) ) * radius;

			if ( rot != -fill ) {
				Vector2D w2s_new, w2s_old;

				if ( Math::WorldToScreen( rotated_pos, w2s_new ) && Math::WorldToScreen( last_pos, w2s_old ) ) {
					auto alpha = 1.f - ( std::abs( rot ) / fill );

					float threshold = seconds * 0.2f;

					if ( seconds - delta < threshold ) {
						float diff = ( seconds - delta ) / ( seconds ) * 5.f;

						alpha *= diff;
					}

					Vertex_t v[ ] = {
						{ world },
						{ w2s_old },
						{ w2s_new }
					};
					Color col = color;
					col[ 3 ] *= alpha;

					Render::Line( Vector2D( w2s_old.x, w2s_old.y ), Vector2D( w2s_new.x, w2s_new.y ), col.Set<COLOR_A>( std::min( maxAlpha, static_cast< int >( col[ 3 ] * fadeAlpha ) ) ) );
				}
			}

			last_pos = rotated_pos;
		}

		Render::Text( Fonts::HealthESP, world, Color( 255, 255, 255, static_cast<int>( maxAlpha * fadeAlpha ) ), FONT_CENTER, name );//color.Set<COLOR_A>( static_cast<int>( maxAlpha * fadeAlpha ) )
	}
}

void CVisuals::EntModulate( CBaseEntity* ent ) {
	if ( !ent || ent->IsDormant( ) )
		return;

	const auto client_class = ent->GetClientClass( );
	if ( !client_class )
		return;

	auto rgb_to_int = [ ]( int red, int green, int blue ) -> int {
		int r;
		int g;
		int b;

		r = red & 0xFF;
		g = green & 0xFF;
		b = blue & 0xFF;
		return ( r << 16 | g << 8 | b );
	};

	// bloom
	if ( client_class->nClassID == EClassIndex::CEnvTonemapController ) {
		if ( Config::Get<bool>( Vars.VisWorldBloom ) ) {
			*( bool* )( uintptr_t( ent ) + Displacement::Netvars->m_bUseCustomAutoExposureMin ) = true;
			*( bool* )( uintptr_t( ent ) + Displacement::Netvars->m_bUseCustomAutoExposureMax ) = true;
			*( bool* )( uintptr_t( ent ) + Displacement::Netvars->m_bUseCustomBloomScale ) = true;

			*( float* )( uintptr_t( ent ) + Displacement::Netvars->m_flCustomAutoExposureMin ) = float( Config::Get<int>( Vars.VisWorldBloomExposure ) ) / 50.f;
			*( float* )( uintptr_t( ent ) + Displacement::Netvars->m_flCustomAutoExposureMax ) = float( Config::Get<int>( Vars.VisWorldBloomExposure ) ) / 50.f;
			*( float* )( uintptr_t( ent ) + Displacement::Netvars->m_flCustomBloomScale ) = float( Config::Get<int>( Vars.VisWorldBloomScale ) ) / 50.f;
		}
		else {
			*( bool* )( uintptr_t( ent ) + Displacement::Netvars->m_bUseCustomAutoExposureMin ) = true;
			*( bool* )( uintptr_t( ent ) + Displacement::Netvars->m_bUseCustomAutoExposureMax ) = true;
			*( bool* )( uintptr_t( ent ) + Displacement::Netvars->m_bUseCustomBloomScale ) = true;
			*( float* )( uintptr_t( ent ) + Displacement::Netvars->m_flCustomAutoExposureMin ) = 0.f;
			*( float* )( uintptr_t( ent ) + Displacement::Netvars->m_flCustomAutoExposureMax ) = 0.f;
			*( float* )( uintptr_t( ent ) + Displacement::Netvars->m_flCustomBloomScale ) = 0.f;
		}
	}

	// fog
	/*if ( client_class->nClassID == EClassIndex::CFogController ) {
		*( byte* )( ( uintptr_t )ent + Displacement::Netvars->m_fog_enable ) = Config::Get<bool>( Vars.VisWorldFog );				// m_fog.enable

		*( bool* )( uintptr_t( ent ) + 0xA1D ) = Config::Get<bool>( Vars.VisWorldFog );					// m_fog.blend
		*( float* )( uintptr_t( ent ) + 0x9F8 ) = 0;
		*( float* )( uintptr_t( ent ) + 0x9FC ) = Config::Get<int>( Vars.VisWorldFogDistance );			// m_fog.start
		*( float* )( uintptr_t( ent ) + 0xA04 ) = Config::Get<int>( Vars.VisWorldFogDensity ) * 0.01f;	// m_fog.maxdensity
		*( float* )( uintptr_t( ent ) + 0xA24 ) = Config::Get<int>( Vars.VisWorldFogHDR ) * 0.01f;		// fog_hdrcolorscale

		const auto& col = Config::Get<Color>( Vars.VisWorldFogCol );
		*( int* )( uintptr_t( ent ) + 0x9E8 ) = rgb_to_int( ( int )( col.Get<COLOR_R>( ) ), ( int )( col.Get<COLOR_G>( ) ), ( int )( col.Get<COLOR_R>( ) ) ); // m_fog.colorPrimary
		*( int* )( uintptr_t( ent ) + 0x9EC ) = rgb_to_int( ( int )( col.Get<COLOR_B>( ) ), ( int )( col.Get<COLOR_G>( ) ), ( int )( col.Get<COLOR_R>( ) ) ); // m_fog.colorSecondary
	}*/
}

// so embarrassed about this func 
void CVisuals::KeybindsList( ) {
	auto addBind = [ & ]( const char* name, float& newXSize, std::vector<BindInfo_t>& binds, int mode ) {
		if ( mode == EKeyMode::AlwaysOn )
			return;

		const auto& b = binds.emplace_back( name, mode );

		if ( b.m_iTextLength + 40 > newXSize )
			newXSize = b.m_iTextLength + 40;
	};

	if ( !Config::Get<bool>( Vars.MiscKeybindList ) ) {
		m_vec2KeyBindAbsSize = { 120, 20 };
		return;
	}

	// gimme setup
	const bool topBarHovered = Inputsys::hovered( m_vec2KeyBindPos, Vector2D( std::max( m_vec2KeyBindAbsSize.x, 100.f ), 20 ) );
	Vector2D keyBindSize = { 120, 20 };
	std::vector<BindInfo_t> binds;

	// gimme binds
	if ( Config::Get<bool>( Vars.ExploitsDoubletap ) && Config::Get<keybind_t>( Vars.ExploitsDoubletapKey ).enabled )
		addBind( _( "Doubletap" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.ExploitsDoubletapKey ).mode );

	if ( Config::Get<bool>( Vars.ExploitsHideshots ) && Config::Get<keybind_t>( Vars.ExploitsHideshotsKey ).enabled )
		addBind( _( "Hideshots" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.ExploitsHideshotsKey ).mode );

	if ( Config::Get<bool>( Vars.RagebotDamageOverride ) && Config::Get<keybind_t>( Vars.RagebotDamageOverrideKey ).enabled )
		addBind( _( "Damage override" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.RagebotDamageOverrideKey ).mode );

	//if ( Config::Get<keybind_t>( Vars.DBGKeybind ).enabled )
	//	addBind( _( "Air stuck" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.DBGKeybind ).mode );

	if ( Config::Get<keybind_t>( Vars.RagebotForceBaimKey ).enabled )
		addBind( _( "Force baim" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.RagebotForceBaimKey ).mode );	
	
	if ( Config::Get<keybind_t>( Vars.RagebotForceSafePointKey ).enabled )
		addBind( _( "Force safe point" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.RagebotForceSafePointKey ).mode );	

	if ( Config::Get<keybind_t>( Vars.RagebotForceYawSafetyKey ).enabled )
		addBind( _( "Force safe yaw" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.RagebotForceYawSafetyKey ).mode );
	
	if ( !Config::Get<bool>( Vars.AntiaimConstantInvertFlick ) && Config::Get<keybind_t>( Vars.AntiaimFlickInvert ).enabled )
		addBind( _( "Flick invert" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.AntiaimFlickInvert ).mode );

	if ( !Config::Get<bool>( Vars.AntiaimConstantInvert ) && Config::Get<keybind_t>( Vars.AntiaimInvert ).enabled )
		addBind( _( "Invert desync" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.AntiaimInvert ).mode );

	if ( Config::Get<bool>( Vars.MiscSlowWalk ) && Config::Get<keybind_t>( Vars.MiscSlowWalkKey ).enabled )
		addBind( _( "Slow walk" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.MiscSlowWalkKey ).mode );

	if ( Config::Get<bool>( Vars.MiscAutoPeek ) && Config::Get<keybind_t>( Vars.MiscAutoPeekKey ).enabled )
		addBind( _( "Auto peek" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.MiscAutoPeekKey ).mode );

	if ( Config::Get<int>( Vars.AntiaimFreestanding ) && Config::Get<keybind_t>( Vars.AntiaimFreestandingKey ).enabled )
		addBind( _( "Freestanding" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.AntiaimFreestandingKey ).mode );

	if ( Config::Get<bool>( Vars.MiscFakeDuck ) && Config::Get<keybind_t>( Vars.MiscFakeDuckKey ).enabled )
		addBind( _( "Fake duck" ), keyBindSize.x, binds, Config::Get<keybind_t>( Vars.MiscFakeDuckKey ).mode );	

	constexpr auto appendLength{ 17 };

	// gimme extra room
	if ( !binds.empty( ) || Menu::m_flAlpha )
		keyBindSize.y += 9 + binds.size( ) * appendLength;

	// gimme that lerped size
	m_vec2KeyBindAbsSize.x = Math::Interpolate( m_vec2KeyBindAbsSize.x, keyBindSize.x, 10.f * Interfaces::Globals->flFrameTime );
	m_vec2KeyBindAbsSize.y = Math::Interpolate( m_vec2KeyBindAbsSize.y, keyBindSize.y, 10.f * Interfaces::Globals->flFrameTime );

	// gimme alpha
	auto alpha_mod = 1.f;
	if ( ( !ctx.m_pLocal || ctx.m_pLocal->IsDead( ) ) && Menu::m_flAlpha < 1.f )
		alpha_mod = Menu::m_flAlpha;

	if ( alpha_mod < 0.001f )
		return;

	// gimme drag
	if ( !m_bKeybindDragging && Inputsys::pressed( VK_LBUTTON ) && topBarHovered && Menu::m_flAlpha )
		m_bKeybindDragging = true;
	else if ( m_bKeybindDragging && Inputsys::down( VK_LBUTTON ) ) {
		Config::Get<int>( Vars.MiscKeybindPosX ) -= Inputsys::MouseDelta.x;
		Config::Get<int>( Vars.MiscKeybindPosY ) -= Inputsys::MouseDelta.y;
	}
	else if ( m_bKeybindDragging && !Inputsys::down( VK_LBUTTON ) )
		m_bKeybindDragging = false;

	if ( m_vec2KeyBindAbsSize.y < 1 )
		return;

	m_vec2KeyBindPos = { static_cast< float >( Config::Get<int>( Vars.MiscKeybindPosX ) ), static_cast< float >( Config::Get<int>( Vars.MiscKeybindPosY ) ) };

	auto& pos = m_vec2KeyBindPos;
	auto& size = m_vec2KeyBindAbsSize;


	{
		Render::FilledRoundedBox( pos - Vector2D( 1, 1 ), size + 2, 5, 5, Color( 10, 10, 10, static_cast< int >( 255 * alpha_mod ) ) );
		Render::FilledRoundedBox( pos, size, 5, 5, OUTLINE_LIGHT.Set<COLOR_A>( 255 * alpha_mod ) );
		Render::FilledRoundedBox( pos + 1, size - Vector2D( 2, 2 ), 5, 5, Color( 10, 10, 10, static_cast< int >( 255 * alpha_mod ) ) );
		Render::FilledRoundedBox( pos + 2, size - Vector2D( 4, 4 ), 5, 5, BACKGROUND.Set<COLOR_A>( 255 * alpha_mod ) );

		for ( int i{ }; i < 3; ++i ) {
			Render::Line( pos + Vector2D( size.x - 19 - i, 2 ), pos + Vector2D( size.x - 4 - i, 17 ), ACCENT.Set<COLOR_A>( 255 * alpha_mod ) );
			Render::Line( pos + Vector2D( size.x - 12 - i, 2 ), pos + Vector2D( size.x - 2, 12 + i ), ACCENT2.Set<COLOR_A>( 255 * alpha_mod ) );

			Render::Line( pos + Vector2D( 19 + i, 2 ), pos + Vector2D( 4 + i, 17 ), ACCENT.Set<COLOR_A>( 255 * alpha_mod ) );
			Render::Line( pos + Vector2D( 11 + i, 2 ), pos + Vector2D( 1, 12 + i ), ACCENT2.Set<COLOR_A>( 255 * alpha_mod ) );
		}

		if ( size.y > 20 ) {
			Render::Line( pos + Vector2D( 1, 17 ), pos + Vector2D( size.x - 2, 17 ), Color( 10, 10, 10, static_cast< int >( 255 * alpha_mod ) ) );
			Render::Line( pos + Vector2D( 1, 18 ), pos + Vector2D( size.x - 2, 18 ), OUTLINE_LIGHT.Set<COLOR_A>( 255 * alpha_mod ) );
			Render::Line( pos + Vector2D( 1, 19 ), pos + Vector2D( size.x - 2, 19 ), Color( 10, 10, 10, static_cast< int >( 255 * alpha_mod ) ) );
		}

		/*18.0000000 key
		25.0000000 binds*/

		Render::Text( Fonts::Menu, pos + Vector2D( size.x / 2 - 21, 3 ), Color( 255, 255, 255, static_cast< int >( 255 * alpha_mod ) ), FONT_LEFT, _( "Key" ) );
		Render::Text( Fonts::Menu, pos + Vector2D( size.x / 2 - 3, 3 ), ACCENT.Set<COLOR_A>( 255 * alpha_mod ), FONT_LEFT, _( "binds" ) );

		Interfaces::Surface->SetClipRect( pos.x + 2, pos.y + 2, size.x - 4, size.y - 4 );

		// gimme text
		for ( auto i{ 0 }; i < binds.size( ); i++ ) {
			const auto& bind = binds.at( i );
			Render::Text( Fonts::Menu, pos + Vector2D( 20, 25 + appendLength * i + 1 ), Color( 255, 255, 255, static_cast< int >( 255 * alpha_mod ) ), FONT_LEFT, bind.m_szName );

			Render::Text( Fonts::Menu, pos + Vector2D( size.x - 20, 25 + appendLength * i ), ACCENT.Set<COLOR_A>( 255 * alpha_mod ), FONT_RIGHT, bind.m_szMode );
		}
	}
	Interfaces::Surface->SetClipRect( 0, 0, ctx.m_ve2ScreenSize.x, ctx.m_ve2ScreenSize.y );
}