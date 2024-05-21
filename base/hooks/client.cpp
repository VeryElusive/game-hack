#include "../core/hooks.h"
#include "../core/config.h"
#include "../context.h"
#include "../features/visuals/visuals.h"
#include "../features/misc/engine_prediction.h"
#include "../features/misc/shots.h"
#include "../features/animations/animation.h"
#include "../features/rage/exploits.h"

// The client overwrites the already predicted viewmodel animation data with networked data from the server.
// We store the data before the client overwrites it and restore it after. This will fix our viewmodel animation resetting while fakelagging.
CBaseViewModel* viewmdl = nullptr;
auto cycle = 0.f, animtime = 0.f;

struct ClientHitVerify_t {
	Vector m_vecPos;
	float  m_flTime;
	float  m_flExpires;
};

void draw_server_hitboxes( int index ) {
	//if ( g_Vars.globals.m_iServerType != 8 )
	//	return;

	auto get_player_by_index = [ ]( int index ) -> CBasePlayer* {
		typedef CBasePlayer* ( __fastcall* player_by_index )( int );
		static auto player_index = reinterpret_cast< player_by_index >( MEM::FindPattern( _( "server.dll" ), _( "85 C9 ? ? A1 ? ? ? ? 3B 48 18 7F 28" ) ) );

		if ( !player_index )
			return false;

		return player_index( index );
	};

	static auto fn = MEM::FindPattern( _( "server.dll" ), _( "55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE" ) );
	auto duration = -1.f;
	PVOID entity = nullptr;

	entity = get_player_by_index( index );

	if ( !entity )
		return;

	__asm {
		pushad
		movss xmm1, duration
		push 0 // 0 - colored, 1 - blue
		mov ecx, entity
		call fn
		popad
	}
}

void FASTCALL Hooks::hkFrameStageNotify( IBaseClientDll* thisptr, int edx, EClientFrameStage stage ) {
	static auto oFrameStageNotify = DTR::FrameStageNotify.GetOriginal<decltype( &hkFrameStageNotify )>( );

	//ctx.m_iLastFSNStage = stage;
	
	ctx.GetLocal( );
	static int backupsmokeCount{ };

	if ( Interfaces::ClientState->pNetChannel ) {
		if ( !DTR::SendNetMsg.IsHooked( ) )
			DTR::SendNetMsg.Create( MEM::GetVFunc( Interfaces::ClientState->pNetChannel, VTABLE::SENDNETMSG ), &Hooks::hkSendNetMsg );

		if ( !DTR::SendDatagram.IsHooked( ) )
			DTR::SendDatagram.Create( MEM::GetVFunc( Interfaces::ClientState->pNetChannel, VTABLE::SENDDATAGRAM ), &Hooks::hkSendDatagram );
	}

	if ( stage == FRAME_RENDER_START ) {
		//Features::Visuals.Weather.Main( );

		if ( ctx.m_pLocal ) {
			/*if ( Interfaces::Input->bCameraInThirdPerson ) {
				auto state{ ctx.m_pLocal->m_pAnimState( ) };
				if ( state )
					ctx.m_pLocal->SetAbsAngles( { 0.f, state->flAbsYaw, 0.f } );
			}*/

			if ( Config::Get<bool>( Vars.VisClientBulletImpacts ) ) {
				static int lastcount{ };
				auto& clientImpactsList{ *( CUtlVector < ClientHitVerify_t >* )( ( uintptr_t )ctx.m_pLocal + 0x11C50u ) };


				Color col{ Config::Get<Color>( Vars.VisClientBulletImpactsCol ) };
				for ( auto i = clientImpactsList.Count( ); i > lastcount; --i )
					Interfaces::DebugOverlay->AddBoxOverlay( clientImpactsList[ i - 1 ].m_vecPos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), QAngle( 0, 0, 0 ),
						col.Get<COLOR_R>( ), col.Get<COLOR_G>( ), col.Get<COLOR_B>( ), col.Get<COLOR_A>( ), Displacement::Cvars.sv_showimpacts_time->GetFloat( ) );

				if ( clientImpactsList.Count( ) != lastcount )
					lastcount = clientImpactsList.Count( );
			}

			for ( int i{ 1 }; i < 64; i++ ) {
				const auto player{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
				if ( !player || player->IsDead( ) || !player->IsPlayer( ) || player == ctx.m_pLocal )
					continue;

				auto& varMap{ player->m_pVarMapping( ) };
				for ( int i{ }; i < varMap.m_nInterpolatedEntries; ++i ) {
					//if ( reinterpret_cast< int >( varMap.m_Entries[ i ].data ) - reinterpret_cast< int >( player ) == 71632 )
					//	varMap.m_Entries[ i ].m_bNeedsToInterpolate = false;
					if ( reinterpret_cast< int >( varMap.m_Entries[ i ].data ) - reinterpret_cast< int >( player ) == 172 )
						varMap.m_Entries[ i ].watcher->m_InterpolationAmount = Interfaces::Globals->flIntervalPerTick * Config::Get<int>( Vars.VisPlayerInterpolation );
					else
						varMap.m_Entries[ i ].m_bNeedsToInterpolate = false;
				}

				/* filtered
				[ HAVOC ] 71632
				[ HAVOC ] 264
				[ HAVOC ] 10808
				[ HAVOC ] 10752
				[ HAVOC ] 2580
				[ HAVOC ] 10104
				[ HAVOC ] 2644
				[ HAVOC ] 172


				71632 = eyeangles
				172 = origin
				*/

				/* start up
				[ HAVOC ] 2580
				[ HAVOC ] 2644
				[ HAVOC ] 10104*/
			}

			if ( Config::Get<bool>( Vars.RemovalFlash ) )
				ctx.m_pLocal->m_flFlashDuration( ) = 0.f;
		}

		{

			if ( Displacement::Cvars.r_modelAmbientMin->GetFloat( ) != ( Config::Get<bool>( Vars.VisWorldBloom ) ? Config::Get<int>( Vars.VisWorldBloomAmbience ) / 10.0f : 0.f ) )
				Displacement::Cvars.r_modelAmbientMin->SetValue( Config::Get<bool>( Vars.VisWorldBloom ) ? Config::Get<int>( Vars.VisWorldBloomAmbience ) / 10.0f : 0.f );

			static bool reset = false;
			if ( Config::Get<bool>( Vars.WorldAmbientLighting ) ) {
				reset = false;
				if ( Displacement::Cvars.mat_ambient_light_r->GetFloat( ) != Config::Get<Color>( Vars.WorldAmbientLightingCol ).Get<COLOR_R>( ) / 255.f )
					Displacement::Cvars.mat_ambient_light_r->SetValue( Config::Get<Color>( Vars.WorldAmbientLightingCol ).Get<COLOR_R>( ) / 255.f );

				if ( Displacement::Cvars.mat_ambient_light_g->GetFloat( ) != Config::Get<Color>( Vars.WorldAmbientLightingCol ).Get<COLOR_G>( ) / 255.f )
					Displacement::Cvars.mat_ambient_light_g->SetValue( Config::Get<Color>( Vars.WorldAmbientLightingCol ).Get<COLOR_G>( ) / 255.f );

				if ( Displacement::Cvars.mat_ambient_light_b->GetFloat( ) != Config::Get<Color>( Vars.WorldAmbientLightingCol ).Get<COLOR_B>( ) / 255.f )
					Displacement::Cvars.mat_ambient_light_b->SetValue( Config::Get<Color>( Vars.WorldAmbientLightingCol ).Get<COLOR_B>( ) / 255.f );
			}
			else {
				if ( !reset ) {
					Displacement::Cvars.mat_ambient_light_r->SetValue( 0.f );
					Displacement::Cvars.mat_ambient_light_g->SetValue( 0.f );
					Displacement::Cvars.mat_ambient_light_b->SetValue( 0.f );
					reset = true;
				}
			}

			auto& smokeCount = **reinterpret_cast< int** >( Displacement::Sigs.SmokeCount + 0x1 );
			backupsmokeCount = smokeCount;

			if ( Config::Get<bool>( Vars.RemovalSmoke ) )
				smokeCount = 0;

			**reinterpret_cast< bool** >( Displacement::Sigs.PostProcess + 0x2 ) = ( ctx.m_pLocal && !Config::Get<bool>( Vars.VisWorldBloom ) && Config::Get<bool>( Vars.RemovalPostProcess ) );


			//if ( Displacement::Cvars.r_drawspecificstaticprop->GetBool( ) )
			//	Displacement::Cvars.r_drawspecificstaticprop->SetValue( FALSE );

			//if ( Displacement::Cvars.sv_showimpacts->GetInt( ) != 2 )
			//	Displacement::Cvars.sv_showimpacts->SetValue( 2 );

			if ( !Config::Get<bool>( Vars.VisWorldFog ) )
				Displacement::Cvars.fog_override->SetValue( FALSE );
			else {
				Displacement::Cvars.fog_override->SetValue( TRUE );

				if ( Displacement::Cvars.fog_start->GetInt( ) )
					Displacement::Cvars.fog_start->SetValue( 0 );


				if ( Displacement::Cvars.fog_end->GetInt( ) != Config::Get<int>( Vars.VisWorldFogDistance ) )
					Displacement::Cvars.fog_end->SetValue( Config::Get<int>( Vars.VisWorldFogDistance ) );

				if ( Displacement::Cvars.fog_maxdensity->GetFloat( ) != ( float )Config::Get<int>( Vars.VisWorldFogDensity ) * 0.01f )
					Displacement::Cvars.fog_maxdensity->SetValue( ( float )Config::Get<int>( Vars.VisWorldFogDensity ) * 0.01f );

				if ( Displacement::Cvars.fog_hdrcolorscale->GetFloat( ) != Config::Get<int>( Vars.VisWorldFogHDR ) * 0.01f )
					Displacement::Cvars.fog_hdrcolorscale->SetValue( Config::Get<int>( Vars.VisWorldFogHDR ) * 0.01f );

				const auto& col{ Config::Get<Color>( Vars.VisWorldFogCol ) };

				char bufferColor[ 12 ]{ };
				sprintf_s( bufferColor, 12, "%i %i %i", col.Get<COLOR_R>( ), col.Get<COLOR_G>( ), col.Get<COLOR_B>( ) );

				if ( strcmp( Displacement::Cvars.fog_color->GetString( ), bufferColor ) )
					Displacement::Cvars.fog_color->SetValue( bufferColor );
			}
		}
	}
	else if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START ) {
		if ( ctx.m_pLocal ) {
			if ( !ctx.m_pLocal->IsDead( ) && !ctx.m_pLocal->m_pAnimState( )->bFirstUpdate ) {

				// TODO: trigger this when standing and save ourselves with defnsive LOL
				/*if ( Config::Get<bool>( Vars.AntiaimTrickLBY ) ) {
					const auto speed{ ctx.m_pLocal->m_vecVelocity( ).Length2D( ) };

					if ( speed <= 1.f ) {
						if ( ctx.m_pLocal->m_AnimationLayers( )[ 3 ].flWeight >= .15f && !ctx.m_pAnimationLayers[ 3 ].flCycle ) {
							Features::Antiaim.Invert = !Features::Antiaim.Invert;
							Features::Logger.Log( "fuck", true );
						}
					}
				}*/

				//std::memcpy( &ctx.m_pLocal->m_AnimationLayers( )[ 3 ], &ctx.m_pAnimationLayers[ 3 ], sizeof CAnimationLayer );
				std::memcpy( &ctx.m_pLocal->m_AnimationLayers( )[ 4 ], &ctx.m_pAnimationLayers[ 4 ], sizeof CAnimationLayer );
				std::memcpy( &ctx.m_pLocal->m_AnimationLayers( )[ 5 ], &ctx.m_pAnimationLayers[ 5 ], sizeof CAnimationLayer );

				ctx.m_pLocal->m_AnimationLayers( )[ 6 ].flWeight = ctx.m_pAnimationLayers[ 6 ].flWeight;
				ctx.m_pLocal->m_AnimationLayers( )[ 6 ].flCycle = ctx.m_pAnimationLayers[ 6 ].flCycle;
				ctx.m_pLocal->m_AnimationLayers( )[ 12 ].flWeight = ctx.m_pAnimationLayers[ 12 ].flWeight;
			}

			Features::Visuals.SkyboxChanger( );
			Features::Visuals.ModelChanger( );
		}

	}
	else if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END ) {
		if ( Config::Get<bool>( Vars.RemovalSmoke ) ) {
			static const std::array< IMaterial*, 4u > smokeMaterials{
				Interfaces::MaterialSystem->FindMaterial( _( "particle/vistasmokev1/vistasmokev1_fire" ), nullptr ),
				Interfaces::MaterialSystem->FindMaterial( _( "particle/vistasmokev1/vistasmokev1_smokegrenade" ), nullptr ),
				Interfaces::MaterialSystem->FindMaterial( _( "particle/vistasmokev1/vistasmokev1_emods" ), nullptr ),
				Interfaces::MaterialSystem->FindMaterial( _( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ), nullptr )
			};

			for ( auto& material : smokeMaterials )
				if ( material )
					material->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );
		}
	}

	//Features::Visuals.ModelChanger( );

	if ( stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START ) {
		if ( ctx.m_pLocal && ctx.m_pLocal->m_hViewModel( ) ) {
			if ( const auto viewModel{ static_cast< CBaseViewModel* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( ctx.m_pLocal->m_hViewModel( ) ) ) }; viewModel ) {
				if ( viewModel->m_flAnimTime( ) != animtime )
					ctx.m_flLastAnimTimeUpdate = Interfaces::Globals->flRealTime;
			}
		}
	}

	oFrameStageNotify( thisptr, edx, stage );

	switch ( stage ) {
	case FRAME_RENDER_START: {
		//Features::AnimSys.SetupFakeMatrix( );

		/*for ( int i{ 1 }; i < 64; i++ ) {
			const auto player{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
			if ( !player || player->IsDead( ) || !player->IsPlayer( ) )
				continue;

			//player->SetAbsOrigin( player->m_vecOrigin( ) );

			draw_server_hitboxes( i );
		}*/
	}break;
	case FRAME_NET_UPDATE_START: {
		if ( ctx.m_pLocal && ctx.m_pLocal->m_hViewModel( ) ) {
			if ( const auto viewModel{ static_cast< CBaseViewModel* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( ctx.m_pLocal->m_hViewModel( ) ) ) }; viewModel ) {
				cycle = viewModel->m_flCycle( );
				animtime = viewModel->m_flAnimTime( );
			}
		}
	}break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START: {
		if ( ctx.m_pLocal && ctx.m_pLocal->m_hViewModel( ) ) {
			if ( const auto viewModel{ static_cast< CBaseViewModel* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( ctx.m_pLocal->m_hViewModel( ) ) ) }; viewModel ) {
				viewModel->m_flCycle( ) = cycle;
				viewModel->m_flAnimTime( ) = animtime;
			}
		}
	}break;	
	case FRAME_RENDER_END: {
		**reinterpret_cast< int** >( Displacement::Sigs.SmokeCount + 0x1 ) = backupsmokeCount;

		//Features::Visuals.BulletTracers.Draw( );
	}break;
	case FRAME_NET_UPDATE_END: {
		if ( ctx.m_pLocal ) {

			/*if ( !ctx.m_pLocal->IsDead( ) ) {
				for ( auto i{ Interfaces::ClientState->pEvents }; i; i = Interfaces::ClientState->pEvents->next ) {
					if ( !i->iClassID )
						continue;

					i->flFireDelay = 0.f;
				}
			}

			Interfaces::Engine->FireEvents( );*/

			Features::AnimSys.RunAnimationSystem( );
			//Features::AnimSys.UpdateCommands( );
		}

		Features::Shots.ProcessShots( );
		
		{
			static DWORD* KillFeedTime = nullptr;
			if ( ctx.m_pLocal && !ctx.m_pLocal->IsDead( ) ) {
				if ( !KillFeedTime )
					KillFeedTime = MEM::FindHudElement<DWORD>( _( "CCSGO_HudDeathNotice" ) );

				if ( KillFeedTime ) {
					auto LocalDeathNotice = ( float* )( ( uintptr_t )KillFeedTime + 0x50 );

					if ( LocalDeathNotice )
						*LocalDeathNotice = Config::Get<bool>( Vars.MiscPreserveKillfeed ) ? FLT_MAX : 1.5f;

					if ( ctx.m_bClearKillfeed ) {
						using Fn = void( __thiscall* )( uintptr_t );
						static auto clearNotices = ( Fn )Displacement::Sigs.ClearNotices;

						clearNotices( ( uintptr_t )KillFeedTime - 0x14 );

						ctx.m_bClearKillfeed = false;
					}
				}
			}
			else
				KillFeedTime = 0;
		}
	}break;
	default: break;

	}

	if ( ctx.m_pLocal ) {
		if ( !Interfaces::GameRules )
			Interfaces::GameRules = ( ( **reinterpret_cast< CCSGameRules*** >( MEM::FindPattern( CLIENT_DLL, _( "A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 80 B8 ? ? ? ? ? 74 7A" ) ) + 0x1 ) ) );

		if ( !Interfaces::GameResources ) {
			for ( auto* pClass{ Interfaces::Client->GetAllClasses( ) }; pClass; pClass = pClass->pNext ) {
				if ( !strcmp( pClass->szNetworkName, _( "CPlayerResource" ) ) ) {
					RecvTable_t* pClassTable = pClass->pRecvTable;

					for ( int nIndex = 0; nIndex < pClassTable->nProps; nIndex++ ) {
						RecvProp_t* pProp = &pClassTable->pProps[ nIndex ];

						if ( !pProp || strcmp( pProp->szVarName, _( "m_iTeam" ) ) != 0 )
							continue;

						Interfaces::GameResources = **reinterpret_cast< IGameResources*** >( DWORD( pProp->pDataTable->pProps->oProxyFn ) + 0x10 );

						break;
					}
					break;
				}
			}
		}
	}
	else {
		Interfaces::GameRules = nullptr;
		Interfaces::GameResources = nullptr;
	}
}

bool FASTCALL Hooks::hkWriteUserCmdDeltaToBuffer( void* ecx, void* edx, int slot, bf_write* buf, int from, int to, bool is_new_command ) {
	static auto oWriteUserCmdDeltaToBuffer = DTR::WriteUserCmdDeltaToBuffer.GetOriginal<decltype( &hkWriteUserCmdDeltaToBuffer )>( );

	if ( from == -1 ) {
		const auto moveMsg{ reinterpret_cast< MoveMsg_t* >( *reinterpret_cast< std::uintptr_t* >(
			reinterpret_cast< std::uintptr_t >( _AddressOfReturnAddress( ) ) - sizeof( std::uintptr_t ) ) - 0x58u ) };

		moveMsg->m_iBackupCmds = 7;
		moveMsg->m_iNewCmds = std::min( 16, Interfaces::ClientState->nChokedCommands + 1 );

		if ( ctx.m_iTicksAllowed > 0 ) {
			const auto newCmds{ std::min( Interfaces::ClientState->nChokedCommands + 1 + ctx.m_iTicksAllowed, 16 ) };

			//const auto backup{ ctx.m_flFixedCurtime };
			//ctx.m_flFixedCurtime = TICKS_TO_TIME( Features::Exploits.AdjustTickbase( newCmds ) + Interfaces::ClientState->nChokedCommands + ctx.m_iTicksAllowed );

			const auto& idx{ ctx.m_pWeapon->m_iItemDefinitionIndex( ) };

			const auto dontShift{ ctx.m_bSafeFromDefensive && Features::Exploits.m_bRealCmds 
				&& ( /*( !ctx.m_pLocal->CanShoot( ) && idx != WEAPON_SSG08 && idx != WEAPON_AWP )
					|| */( Features::Misc.AutoPeeking && Config::Get<bool>( Vars.ExploitsDoubletapExtended ) && Config::Get<bool>( Vars.ExploitsDoubletapDefensive ) ) ) };

			//ctx.m_flFixedCurtime = backup;

			if ( dontShift )
				ctx.m_iLastStopTime = Interfaces::Globals->flRealTime;

			if ( Features::Exploits.m_iShiftAmount && !dontShift )
				Features::Exploits.Shift( buf, moveMsg );
			else if ( Features::Exploits.m_bWasDefensiveTick || dontShift )
					Features::Exploits.BreakLC( buf, ctx.m_iTicksAllowed, moveMsg );
			else if ( Features::Exploits.m_iRechargeCmd != Interfaces::ClientState->iLastOutgoingCommand ) {
				const auto newCmds{ std::min( moveMsg->m_iNewCmds + ctx.m_iTicksAllowed, 16 ) };

				const auto maxShiftedCmds{ newCmds - moveMsg->m_iNewCmds };

				ctx.m_iTicksAllowed = std::max( maxShiftedCmds, 0 );
			}
		}
		//else if ( Config::Get<bool>( Vars.ExploitsBreakCBB ) )
		//	Features::Exploits.BreakCBB( buf, moveMsg );

		bool corrected{ };
		auto tickbase{ Features::Exploits.AdjustTickbase( moveMsg->m_iNewCmds, &corrected ) };

		for ( to = Interfaces::ClientState->iLastOutgoingCommand + 1; to <= Interfaces::ClientState->iLastOutgoingCommand + moveMsg->m_iNewCmds; ++to ) {
			auto& localData{ ctx.m_cLocalData.at( to % 150 ) };
			if ( auto command{ Interfaces::Input->GetUserCmd( to ) }; command && command->iTickCount != INT_MAX ) {
				// if we didn't correct this tick, use the regular tickbase as it was corrected if our tickbase was incorrect when we needed to correct it
				if ( corrected ) {
					localData.m_bOverrideTickbase = true;
					localData.m_iAdjustedTickbase = tickbase;
				}
				else
					tickbase = localData.m_iTickbase;

				tickbase++;
			}
		}

		Features::Exploits.m_iServerTickbase = tickbase;

		if ( tickbase > ctx.m_iHighestTickbase ) {
			ctx.m_bSafeFromDefensive = false;
			ctx.m_iHighestTickbase = tickbase;
		}
		else
			ctx.m_bSafeFromDefensive = true;

		auto nextCmdNumber{ Interfaces::ClientState->iLastOutgoingCommand + Interfaces::ClientState->nChokedCommands + 1 };

		if ( ctx.m_pLocal && !ctx.m_pLocal->IsDead( ) ) {
			if ( auto command{ Interfaces::Input->GetUserCmd( nextCmdNumber ) }; command )
				Features::Antiaim.RunLocalModifications( *command, tickbase );
		}

		for ( to = Interfaces::ClientState->iLastOutgoingCommand - 7 + 1; to <= Interfaces::ClientState->iLastOutgoingCommand + moveMsg->m_iNewCmds; ++to ) {
			if ( !oWriteUserCmdDeltaToBuffer( ecx, edx, slot, buf, from, to, to >= Interfaces::ClientState->iLastOutgoingCommand + 1 ) )
				return false;

			from = to;
		}

		ctx.m_cLocalData.at( nextCmdNumber % 150 ).m_iTickCount = Interfaces::Globals->iTickCount;

		ctx.m_iSentCmds.push_back( nextCmdNumber );
	}
	
	return true;
}