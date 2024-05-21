#include "../core/hooks.h"
#include "../core/config.h"
#include "../context.h"
#include "../features/misc/misc.h"
#include "../features/rage/exploits.h"
#include "../features/visuals/visuals.h"

void FASTCALL Hooks::hkOverrideView( IClientModeShared* thisptr, int edx, CViewSetup* pSetup ) {
	static auto oOverrideView = DTR::OverrideView.GetOriginal<decltype( &hkOverrideView )>( );
	
	ctx.GetLocal( );

	if ( !ctx.m_pLocal || ctx.m_pLocal->IsDead( ) ) {
		Interfaces::Input->bCameraInThirdPerson = false;
		return oOverrideView( thisptr, edx, pSetup );
	}

	//Features::Visuals.GrenadePrediction.View( );

	if ( Config::Get<bool>( Vars.RemovalZoom ) ) {
		pSetup->flFOV = 90.f + Config::Get<int>( Vars.MiscFOV );
		if ( ctx.m_pWeapon ) {
			if ( ctx.m_pWeaponData->nWeaponType == WEAPONTYPE_SNIPER && ctx.m_pWeapon->m_zoomLevel( ) == 2 && ctx.m_pLocal->m_bIsScoped( ) )
				pSetup->flFOV = static_cast< float >( 1.f - std::clamp<float>( Config::Get<int>( Vars.SecondZoomAmt ), 1, 99 ) / 100.f ) * pSetup->flFOV;
		}
	}
	else {
		if ( !ctx.m_pLocal->m_bIsScoped( ) )
			pSetup->flFOV = 90.f + Config::Get<int>( Vars.MiscFOV );
	}

	/*		if ( Config::Get<bool>( Vars.RemovalScope ) ) {
		const auto addAmt{ 15.f * Interfaces::Globals->flFrameTime };

		static float fov{ pSetup->flFOV };

		if ( ctx.m_pWeapon ) {
			if ( ctx.m_pLocal->m_bIsScoped( ) ) {
				if ( !ctx.m_pWeapon->m_zoomLevel( ) )
					fov = Math::Interpolate( fov, 90.f + Config::Get<int>( Vars.MiscFOV ), addAmt );
				else if ( ctx.m_pWeapon->m_zoomLevel( ) == 1 && !Config::Get<bool>( Vars.RemovalZoom ) )
					fov = Math::Interpolate( fov, 70.f + Config::Get<int>( Vars.MiscFOV ), addAmt );
				else if ( ctx.m_pWeapon->m_zoomLevel( ) == 2 ) {
					if ( Config::Get<bool>( Vars.RemovalZoom ) ) {
						if ( ctx.m_pWeapon ) {
							if ( ctx.m_pWeaponData->nWeaponType == WEAPONTYPE_SNIPER )
								fov = Math::Interpolate( fov, static_cast< float >( 1.f - std::clamp<float>( Config::Get<int>( Vars.SecondZoomAmt ), 1, 99 ) / 100.f ) * ( 90.f + Config::Get<int>( Vars.MiscFOV ) ), addAmt );
						}
					}
					else
						fov = Math::Interpolate( fov, 50.f + Config::Get<int>( Vars.MiscFOV ), addAmt );
				}
				else
					fov = Math::Interpolate( fov, 90.f + Config::Get<int>( Vars.MiscFOV ), addAmt );
			}
			else
				fov = Math::Interpolate( fov, 90.f + Config::Get<int>( Vars.MiscFOV ), addAmt );
		}

		pSetup->flFOV = fov;
	}
	*/

	if ( Config::Get<bool>( Vars.VisThirdPerson ) && Config::Get<keybind_t>( Vars.VisThirdPersonKey ).enabled )
		Features::Misc.Thirdperson( );
	else {
		Interfaces::Input->vecCameraOffset = { 0, 0, 0 };
		Interfaces::Input->bCameraInThirdPerson = Features::Misc.TPFrac = 0;
	}


	if ( ctx.m_bFakeDucking && !ctx.m_pLocal->IsDead( ) )
		pSetup->vecOrigin = ctx.m_pLocal->GetAbsOrigin( ) + Vector( 0, 0, Interfaces::GameMovement->GetPlayerViewOffset( false ).z );
	else if( Interfaces::Input->bCameraInThirdPerson )
		pSetup->vecOrigin = ctx.m_pLocal->GetAbsOrigin( ) + ctx.m_pLocal->m_vecViewOffset( );

	oOverrideView( thisptr, edx, pSetup );

	// remove scope edge blur
	if ( Config::Get<bool>( Vars.RemovalScope ) ) {
		if ( ctx.m_pLocal && ctx.m_pLocal->m_bIsScoped( ) )
			pSetup->bOffCenter = 0;
	}
}

bool STDCALL Hooks::hkCMCreateMove( float input_sample_frametime, CUserCmd* cmd ) {
	static auto oCMCreateMove = DTR::CMCreateMove.GetOriginal<decltype( &hkCMCreateMove )>( );

	if ( Features::Exploits.m_iRechargeCmd == Interfaces::ClientState->iLastOutgoingCommand ) {
		const auto backupInterp{ Interfaces::Globals->flInterpolationAmount };
		oCMCreateMove( input_sample_frametime, cmd );
		Interfaces::Globals->flInterpolationAmount = backupInterp;
		return false;
	}

	return oCMCreateMove( input_sample_frametime, cmd );
}

bool FASTCALL Hooks::hkShouldDrawViewModel( void* ) {
	return true;
}