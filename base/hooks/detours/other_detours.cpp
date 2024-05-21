#include "../../core/hooks.h"
#include "../../context.h"
#include "../../features/rage/exploits.h"
#include "../../features/visuals/visuals.h"
#include <intrin.h>

void FASTCALL Hooks::hkModifyEyePosition( CCSGOPlayerAnimState* ecx, void* edx, Vector& pos ) {
	static auto oModifyEyePosition = DTR::ModifyEyePosition.GetOriginal<decltype( &hkModifyEyePosition )>( );

	const auto BackupMoveWeightSmoothed = ecx->flMoveWeightSmoothed;
	const auto BackupCameraSmoothHeight = ecx->flCameraSmoothHeight;

	ecx->m_bJumping = false;

	auto& viewOffset{ ecx->m_pPlayer->m_vecViewOffset( ) };
	viewOffset.z -= viewOffset.z - std::floor( viewOffset.z );

	oModifyEyePosition( ecx, edx, pos );

	ecx->flMoveWeightSmoothed = BackupMoveWeightSmoothed;
	ecx->flCameraSmoothHeight = BackupCameraSmoothHeight;
}

void FASTCALL Hooks::hkCalcView( CBasePlayer* pPlayer, void* edx, Vector& vecEyeOrigin, QAngle& angEyeAngles, float& flZNear, float& flZFar, float& flFov ) {
	static auto oCalcView = DTR::CalcView.GetOriginal<decltype( &hkCalcView )>( );
	if ( !pPlayer || pPlayer != ctx.m_pLocal )
		return oCalcView( pPlayer, edx, vecEyeOrigin, angEyeAngles, flZNear, flZFar, flFov );

	const bool backupUseNewAnimstateBackup = pPlayer->m_bUseNewAnimstate( );
	const auto backupAimPunch = pPlayer->m_aimPunchAngle( );
	const auto backupViewPunch = pPlayer->m_viewPunchAngle( );

	pPlayer->m_bUseNewAnimstate( ) = false;

	if ( Config::Get<bool>( Vars.RemovalPunch ) )
		ctx.m_pLocal->m_viewPunchAngle( ) = pPlayer->m_aimPunchAngle( ) = { };

	oCalcView( pPlayer, edx, vecEyeOrigin, angEyeAngles, flZNear, flZFar, flFov );

	pPlayer->m_bUseNewAnimstate( ) = backupUseNewAnimstateBackup;
	ctx.m_pLocal->m_viewPunchAngle( ) = backupViewPunch;
	pPlayer->m_aimPunchAngle( ) = backupAimPunch;
}

float FASTCALL Hooks::hkCalcViewmodelBob( CWeaponCSBase* pWeapon, void* edx ) {
	return 0.f;
}

void** FASTCALL Hooks::hkCHudScopePaint( void* ecx, int edx ) {
	static auto oCHudScopePaint = DTR::CHudScopePaint.GetOriginal<decltype( &hkCHudScopePaint )>( );

	if ( !Config::Get<bool>( Vars.RemovalScope ) )
		oCHudScopePaint( ecx, edx );

	return nullptr;
}

bool FASTCALL Hooks::hkShouldInterpolate( CBasePlayer* ecx, const std::uintptr_t edx ) {
	static auto oShouldInterpolate = DTR::ShouldInterpolate.GetOriginal<decltype( &hkShouldInterpolate )>( );
	if ( ecx == ctx.m_pLocal
		&& Features::Exploits.m_iRechargeCmd == Interfaces::ClientState->iLastOutgoingCommand ) {

		return false;
	}

	return oShouldInterpolate( ecx, edx );
}


bool CDECL Hooks::hkGlowEffectSpectator( CBasePlayer* const player, CBasePlayer* const local, int& style,
	Vector& clr, float& alpha_from, float& alpha_to,
	float& time_from, float& time_to, bool& animate ) {
	static auto oGlowEffectSpectator = DTR::GlowEffectSpectator.GetOriginal<decltype( &hkGlowEffectSpectator )>( );

	int type{ ENEMY };
	if ( player == ctx.m_pLocal )
		type = LOCAL;
	else if ( player->IsTeammate( ) )
		type = TEAM;

	Color col{ };
	GetPlayerColorFig( type, VisGlowCol, col );

	CheckIfPlayer( VisGlow, type ) {
		style = 0;

		clr.x = col.Get<COLOR_R>( ) / 255.f;
		clr.y = col.Get<COLOR_G>( ) / 255.f;
		clr.z = col.Get<COLOR_B>( ) / 255.f;

		alpha_to = col.Get<COLOR_A>( ) / 255.f;

		return true;
	}

	return oGlowEffectSpectator( player, local, style, clr, alpha_from, alpha_to, time_from, time_to, animate );
}

void FASTCALL Hooks::hkGetColorModulation( IMaterial* const ecx, const std::uintptr_t edx, float* const r, 
	float* const g, float* const b ) {
	static auto oGetColorModulation = DTR::GetColorModulation.GetOriginal<decltype( &hkGetColorModulation )>( );

	oGetColorModulation( ecx, edx, r, g, b );

	const auto textureGroupName = ecx->GetTextureGroupName( );

	// https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/public/texture_group_names.h
	
	// *reinterpret_cast< const std::uint32_t* >( "SkyBox textures" )
	if ( *reinterpret_cast< const std::uint32_t* >( textureGroupName ) == 0x42796B53u ) {
		if ( Config::Get<bool>( Vars.VisWorldSkyboxMod ) ) {
			const auto& col = Config::Get<Color>( Vars.WorldSkyboxCol );

			*r = col.Get<COLOR_R>( ) / 255.f;
			*g = col.Get<COLOR_G>( ) / 255.f;
			*b = col.Get<COLOR_B>( ) / 255.f;
		}

		return;
	}	

	// *reinterpret_cast< const std::uint32_t* >( "StaticProp textures" )
	if ( *reinterpret_cast< const std::uint32_t* >( textureGroupName ) == 0x74617453u ) {
		if ( Config::Get<bool>( Vars.VisWorldPropMod ) ) {
			const auto& col = Config::Get<Color>( Vars.VisWorldPropCol );

			*r = col.Get<COLOR_R>( ) / 255.f;
			*g = col.Get<COLOR_G>( ) / 255.f;
			*b = col.Get<COLOR_B>( ) / 255.f;
		}

		return;
	}

	// *reinterpret_cast< const std::uint32_t* >( "World textures" )
	if ( *reinterpret_cast< const std::uint32_t* >( textureGroupName ) == 0x6c726f57u ) {
		if ( Config::Get<bool>( Vars.WorldModulation ) ) {
			const auto& col = Config::Get<Color>( Vars.WorldModulationCol );

			*r = col.Get<COLOR_R>( ) / 255.f;
			*g = col.Get<COLOR_G>( ) / 255.f;
			*b = col.Get<COLOR_B>( ) / 255.f;
		}

		return;
	}
}

float FASTCALL Hooks::hkGetAlphaModulation( IMaterial* ecx, uint32_t ebx ) {
	static auto oGetAlphaModulation = DTR::GetAlphaModulation.GetOriginal<decltype( &hkGetAlphaModulation )>( );

	const auto textureGroupName = ecx->GetTextureGroupName( );

	if ( *reinterpret_cast< const std::uint32_t* >( textureGroupName ) == 0x74617453u
		&& Config::Get<bool>( Vars.VisWorldPropMod ) ) {
		const auto& col = Config::Get<Color>( Vars.VisWorldPropCol );

		return col.Get<COLOR_A>( ) / 255.f;
	}

	return oGetAlphaModulation( ecx, ebx );
}

void FASTCALL Hooks::hkUpdatePostProcessingEffects( void* ecx, int edx ) {
	static auto oUpdatePostProcessingEffects = DTR::UpdatePostProcessingEffects.GetOriginal<decltype( &hkUpdatePostProcessingEffects )>( );
	if ( !ctx.m_pLocal )
		return oUpdatePostProcessingEffects( ecx, edx );

	const auto backupScoped{ ctx.m_pLocal->m_bIsScoped( ) };
	ctx.m_pLocal->m_bIsScoped( ) = false;

	oUpdatePostProcessingEffects( ecx, edx );

	ctx.m_pLocal->m_bIsScoped( ) = backupScoped;
}


int FASTCALL Hooks::hkGetWeaponType( void* ecx, int edx ) {
	static auto oGetWeaponType = DTR::GetWeaponType.GetOriginal<decltype( &hkGetWeaponType )>( );

	if ( reinterpret_cast< uintptr_t >( _ReturnAddress( ) ) == Displacement::Sigs.ReturnToWantReticleShown
		|| reinterpret_cast< uintptr_t >( _ReturnAddress( ) ) == Displacement::Sigs.ReturnToDrawCrosshair ) {
		if ( Config::Get<bool>( Vars.MiscForceCrosshair ) && ctx.m_pLocal && !ctx.m_pLocal->m_bIsScoped( ) )
			return 0xFADED;
	}

	return oGetWeaponType( ecx, edx );
}

bool ReadPackets{};
void __vectorcall Hooks::hk_Host_RunFrame_Input( float accumulated_extra_samples, bool bFinalTick ) {
	static auto o_Host_RunFrame_Input = DTR::_Host_RunFrame_Input.GetOriginal<decltype( &hk_Host_RunFrame_Input )>( );
	static auto o_Host_RunFrame_Client = DTR::_Host_RunFrame_Client.GetOriginal<decltype( &hk_Host_RunFrame_Client )>( );

	ctx.GetLocal( );

	if ( !ctx.m_pLocal || ctx.m_pLocal->IsDead( ) ) {
		ReadPackets = true;
		return o_Host_RunFrame_Input( accumulated_extra_samples, bFinalTick );
	}

	ReadPackets = false;
	o_Host_RunFrame_Client( bFinalTick );

	o_Host_RunFrame_Input( accumulated_extra_samples, bFinalTick );
}

void FASTCALL Hooks::hk_Host_RunFrame_Client( bool bFinalTick ) {
	static auto o_Host_RunFrame_Client = DTR::_Host_RunFrame_Client.GetOriginal<decltype( &hk_Host_RunFrame_Client )>( );
	static auto o_Host_RunFrame_Input = DTR::_Host_RunFrame_Input.GetOriginal<decltype( &hk_Host_RunFrame_Input )>( );

	ctx.GetLocal( );

	if ( ReadPackets )
		o_Host_RunFrame_Client( bFinalTick );
}

void __vectorcall Hooks::hkCL_Move( float accumulated_extra_samples, bool bFinalTick ) {
	static auto oCL_Move = DTR::CL_Move.GetOriginal<decltype( &hkCL_Move )>( );

	ctx.GetLocal( );


	oCL_Move( accumulated_extra_samples, bFinalTick );
}

void FASTCALL Hooks::hkInterpolatedVarArrayBase_Reset( void* ecx, int edx, float a2 ) {

	//Features::Logger.Log( std::to_string( reinterpret_cast< uintptr_t >( *( void** ) ( reinterpret_cast< uintptr_t >( ecx ) + 4 ) ) ), false );
	return;
}

/*void FASTCALL Hooks::hkCL_ReadPackets( bool bFinalTick ) {
	static auto oCL_ReadPackets = DTR::CL_ReadPackets.GetOriginal<decltype( &hkCL_ReadPackets )>( );
	static auto oCL_Move = DTR::CL_Move.GetOriginal<decltype( &hkCL_Move )>( );

	oCL_ReadPackets( bFinalTick );
	oCL_Move( accumulatedExtraSamples, bFinalTick );
}*/

bool FASTCALL Hooks::hkisBoneAvailableForRead( void* ecx, int edx, int a2 ) {
	static auto oisBoneAvailableForRead = DTR::isBoneAvailableForRead.GetOriginal<decltype( &hkisBoneAvailableForRead )>( );

	if ( reinterpret_cast< uintptr_t >( _ReturnAddress( ) ) == Displacement::Sigs.ReturnToProcessInputIsBoneAvailableForRead )
		return false;
	else
		return oisBoneAvailableForRead( ecx, edx, a2 );
}

Vector temp{ 0, 0, 20 };
Vector* FASTCALL Hooks::hkGetAbsOrigin( void* ecx, int edx ) {
	static auto oGetAbsOrigin = DTR::GetAbsOrigin.GetOriginal<decltype( &hkGetAbsOrigin )>( );

	if ( reinterpret_cast< uintptr_t >( _ReturnAddress( ) ) == Displacement::Sigs.ReturnToProcessInputGetAbsOrigin )
		return &temp;
	else
		return oGetAbsOrigin( ecx, edx );
}

void FASTCALL Hooks::hkGetExposureRange( float* pflAutoExposureMin, float* pflAutoExposureMax ) {
	static auto ohkGetExposureRange = DTR::GetExposureRange.GetOriginal<decltype( &hkGetExposureRange )>( );

	*pflAutoExposureMin = 1.f;
	*pflAutoExposureMax = 1.f;

	ohkGetExposureRange( pflAutoExposureMin, pflAutoExposureMax );
}