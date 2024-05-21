#include <intrin.h>
#include <array>
#include <thread>
#include <windows.h>
#include "hooks.h"
// used: global variables
#include "../context.h"
#include "../features/misc/logger.h"
#include "prop_manager.h"
/*void FASTCALL newcollisionServer( CBasePlayer* ecx, uint32_t edx, Vector* oldMins, Vector* newMins, Vector* oldMaxs, Vector* newMaxs ) {
	static auto oOnNewCollisionBounds = DTR::CMCreateMove.GetOriginal<decltype( &newcollisionServer )>( );

	oOnNewCollisionBounds( ecx, edx, oldMins, newMins, oldMaxs, newMaxs );

	Features::Logger.Log( ( "SERVER: " + std::to_string( *reinterpret_cast< float* >( ( reinterpret_cast< std::uintptr_t >( ecx ) + 0x3AF4 ) ) ) + " maxs: " +
		std::to_string( *reinterpret_cast< float* >( ( reinterpret_cast< std::uintptr_t >( ecx ) + 0x3AF0 ) ) ) ).c_str( ), true );
	//0x3AF4
}*/

#pragma region hooks_get
bool Hooks::Setup( ) {
	if ( MH_Initialize( ) != MH_OK )
		throw std::runtime_error( _( "failed initialize minhook" ) );

	if ( !DTR::LockCursor.Create( MEM::GetVFunc( Interfaces::Surface, VTABLE::LOCKCURSOR ), &hkLockCursor ) )
		return false;

	//if ( !DTR::PaintTraverse.Create( MEM::GetVFunc( Interfaces::Panel, VTABLE::PAINTTRAVERSE ), &hkPaintTraverse ) )
	//	return false;

	if ( !DTR::CreateMoveProxy.Create( MEM::GetVFunc( Interfaces::Client, VTABLE::CREATEMOVE ), &hkCreateMoveProxy ) )
		return false;

	if ( !DTR::DrawModel.Create( MEM::GetVFunc( Interfaces::StudioRender, VTABLE::DRAWMODEL ), &hkDrawModel ) )
		return false;

	if ( !DTR::Paint.Create( MEM::GetVFunc( Interfaces::EngineVGui, VTABLE::VGUI_PAINT ), &HkPaint ) )
		return false;

	if ( !DTR::FrameStageNotify.Create( MEM::GetVFunc( Interfaces::Client, VTABLE::FRAMESTAGENOTIFY ), &hkFrameStageNotify ) )
		return false;

	if ( !DTR::OverrideView.Create( MEM::GetVFunc( Interfaces::ClientMode, VTABLE::OVERRIDEVIEW ), &hkOverrideView ) )
		return false;		
	
	if ( !DTR::ShouldDrawViewModel.Create( MEM::GetVFunc( Interfaces::ClientMode, 27 ), &hkShouldDrawViewModel ) )
		return false;	

	//if ( !DTR::DoPostScreenEffects.Create( MEM::GetVFunc( Interfaces::ClientMode, VTABLE::DOPOSTSCREENEFFECTS ), &hkDoPostScreenEffects ) )
	//	return false;		
	
	if ( !DTR::GetUserCmd.Create( MEM::GetVFunc( Interfaces::Input, 8 ), &hkGetUserCmd ) )
		return false;	
	
	//if ( !DTR::ClipRayToCollideable.Create( MEM::GetVFunc( Interfaces::EngineTrace, 4 ), &hkClipRayToCollideable ) )
	//	return false;
	
	if ( !DTR::OverrideConfig.Create( MEM::GetVFunc( Interfaces::MaterialSystem, VTABLE::OVERRIDECONFIG ), &hkOverrideConfig ) )
		return false;
	
	if ( !DTR::GetScreenAspectRatio.Create( MEM::GetVFunc( Interfaces::Engine, VTABLE::GETSCREENASPECTRATIO ), &hkGetScreenAspectRatio ) )
		return false;	

	if ( !DTR::IsPaused.Create( MEM::GetVFunc( Interfaces::Engine, VTABLE::ISPAUSED ), &hkIsPaused ) )
		return false;
	
	if ( !DTR::EmitSound.Create( MEM::GetVFunc( Interfaces::EngineSound, VTABLE::EMITSOUND ), &hkEmitSound ) )
		return false;	
	
	if ( !DTR::SceneEnd.Create( MEM::GetVFunc( Interfaces::RenderView, 9 ), &hkSceneEnd ) )
		return false;

	void* pClientStateSwap = ( void* )( uint32_t( Interfaces::ClientState ) + 8 );
	if ( !DTR::PacketEnd.Create( MEM::GetVFunc( pClientStateSwap, 6 ), &Hooks::hkPacketEnd ) )
		return false;

	if ( !DTR::PacketStart.Create( MEM::GetVFunc( pClientStateSwap, 5 ), &Hooks::hkPacketStart ) )
		return false;	

	if ( !DTR::ProcessTempEntities.Create( MEM::GetVFunc( pClientStateSwap, 36 ), &Hooks::hkProcessTempEntities ) )
		return false;

	if ( !DTR::WriteUserCmdDeltaToBuffer.Create( MEM::GetVFunc( Interfaces::Client, VTABLE::WRITEUSERCMDDELTATOBUFFER ), &hkWriteUserCmdDeltaToBuffer ) )
		return false;

	if ( !DTR::ProcessMovement.Create( MEM::GetVFunc( Interfaces::GameMovement, VTABLE::PROCESSMOVEMENT ), &hkProcessMovement ) )
		return false;			

	//if ( !DTR::AddBoxOverlay.Create( MEM::GetVFunc( Interfaces::DebugOverlay, 1 ), &hkAddBoxOverlay ) )
	//	return false;

	if ( !DTR::SvCheatsGetBool.Create( MEM::GetVFunc( Interfaces::ConVar->FindVar( _( "sv_cheats" ) ), VTABLE::GETBOOL ), &hkSvCheatsGetBool ) )
		return false;

	if ( !DTR::ClPredictGetInt.Create( MEM::GetVFunc( Interfaces::ConVar->FindVar( _( "cl_predict" ) ), VTABLE::GETINT ), &hkClPredictGetInt ) )
		return false;

	//if ( !DTR::ClDoResetLatchedGetBool.Create( MEM::GetVFunc( Interfaces::ConVar->FindVar( _( "cl_pred_doresetlatch" ) ), VTABLE::GETINT ), &hkClDoResetLatchedGetBool ) )
	//	return false;

	m_bClientSideAnimation = PropManager::Get( ).Hook( m_bClientSideAnimationHook, _( "DT_BaseAnimating" ), _( "m_bClientSideAnimation" ) );
	m_flSimulationTime = PropManager::Get( ).Hook( m_flSimulationTimeHook, _( "DT_BaseEntity" ), _( "m_flSimulationTime" ) );
	//m_flAbsYaw = PropManager::Get( ).Hook( m_flAbsYawHook, _( "DT_CSRagdoll" ), _( "m_flAbsYaw" ) );
	//m_hWeapon = PropManager::Get( ).Hook( m_hWeaponHook, _( "DT_BaseViewModel" ), _( "m_hWeapon" ) );
	//m_flCycle = PropManager::Get( ).Hook( m_flCycle_Recv, _( "DT_BaseViewModel" ), _( "m_flCycle" ) );
	//m_nSequence = PropManager::Get( ).Hook( m_flCycle_Recv, _( "DT_BaseViewModel" ), _( "m_nSequence" ) );
	//m_flAnimTime = PropManager::Get( ).Hook( m_flAnimTime_Recv, _( "DT_BaseViewModel" ), _( "m_flAnimTime" ) );

	D3DDEVICE_CREATION_PARAMETERS creationParameters = { };
	while ( FAILED( Interfaces::DirectDevice->GetCreationParameters( &creationParameters ) ) )
		std::this_thread::sleep_for( 200ms );

	hWindow = creationParameters.hFocusWindow;
	if ( !hWindow )
		return false;

	pOldWndProc = reinterpret_cast< WNDPROC >( SetWindowLongPtr( hWindow, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( Hooks::hkWndProc ) ) );
	if ( !pOldWndProc )
		return false;

	if ( !DTR::SetUpMovement.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 81 ? ? ? ? ? 56 57 8B ? ? ? ? ? 8B F1" ) ) ),
		&Hooks::hkSetUpMovement ) )
		return false;	
	
	if ( !DTR::isBoneAvailableForRead.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 53 56 57 8B F9 8B 37 85 F6 74 47 8B" ) ) ),
		&Hooks::hkisBoneAvailableForRead ) )
		return false;	
	
	if ( !DTR::GetAbsOrigin.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "56 8B F1 E8 ? ? ? ? 8D 86 ? ? ? ? 5E C3" ) ) ),
		&Hooks::hkGetAbsOrigin ) )
		return false;

	if ( !DTR::UpdateClientsideAnimation.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 51 56 8B F1 80 BE ? ? 00 00 00 74 36" ) ) ),
		&Hooks::hkUpdateClientsideAnimation ) )
		return false;

	/*if ( !DTR::AnimStateUpdate.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24" ) ) ),
		&Hooks::HkAnimStateUpdate ) )
		return false;

	using tmp = void( __vectorcall* )( );
	Displacement::Sigs.oUpdateAnimationState = reinterpret_cast< uintptr_t >( DTR::AnimStateUpdate.GetOriginal< tmp >( ) );*/

	if ( !DTR::GlowEffectSpectator.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 14 53 8B 5D 0C 56 57 85 DB 74 47 " ) ) ),
		&Hooks::hkGlowEffectSpectator ) )
		return false;			
	
	if ( !DTR::GetColorModulation.Create(
		( byte* )( MEM::FindPattern( MATERIALSYSTEM_DLL, _( "55 8B EC 83 EC ? 56 8B F1 8A 46" ) ) ),
		&Hooks::hkGetColorModulation ) )
		return false;		
	
	if ( !DTR::GetAlphaModulation.Create(
		( byte* )( MEM::FindPattern( MATERIALSYSTEM_DLL, _( "56 8B F1 8A 46 20 C0 E8 02 A8 01 75 0B 6A 00 6A 00 6A 00 E8 ? ? ? ? 80 7E 22 05 76 0E" ) ) ),
		&Hooks::hkGetAlphaModulation ) )
		return false;	

	if ( !DTR::Setupbones.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F0 B8 D8" ) ) ),
		&Hooks::hkSetupbones ) )
		return false;	

	if ( !DTR::PhysicsSimulate.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23" ) ) ),
		&Hooks::hkPhysicsSimulate ) )
		return false;
	
	if ( !DTR::CalcViewmodelBob.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC A1 ? ? ? ? 83 EC 10 8B 40 34" ) ) ),
		&Hooks::hkCalcViewmodelBob ) )
		return false;		
	
	if ( !DTR::CalcView.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 14 53 56 57 FF 75 18" ) ) ),
		&Hooks::hkCalcView ) )
		return false;			
	
	if ( !DTR::IsBoneAvailable.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 56 FF 75 08 8B F1 E8 ? ? ? ? 84 C0 74 15" ) ) ),
		&Hooks::hkIsBoneAvailable ) )
		return false;		
	
	/*if ( !DTR::ClampBonesInBBox.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 38 83 BF" ) ) ),
		&Hooks::hkClampBonesInBBox ) )
		return false;	

	if ( !DTR::OnNewCollisionBounds.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 8B 45 10 F3 0F 10 81" ) ) ),
		&Hooks::hkOnNewCollisionBounds ) )
		return false;	*/
	
	if ( !DTR::UpdatePostProcessingEffects.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 51 53 56 57 8B F9 8B 4D 04 E8 ? ? ? ? 8B 35" ) ) ),
		&Hooks::hkUpdatePostProcessingEffects ) )
		return false;
	
	if ( !DTR::CHudScopePaint.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 ? 83 EC ? 56 89 4C 24 ? 8B 4D" ) ) ),
		&Hooks::hkCHudScopePaint ) )
		return false;		
	
	if ( !DTR::GetWeaponType.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "8B 01 FF 90 ? ? ? ? 8B 80 ? ? ? ? C3 CC 56" ) ) ),
		&Hooks::hkGetWeaponType ) )
		return false;	
	
	if ( !DTR::C_BaseViewModel__Interpolate.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 0C 53 56 8B F1 57 83 BE" ) ) ),
		&Hooks::hkC_BaseViewModel__Interpolate ) )
		return false;		
	
	if ( !DTR::GetExposureRange.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 51 80 3D ? ? ? ? ? 0F 57" ) ) ),
		&Hooks::hkGetExposureRange ) )
		return false;	
	
	/*if ( !DTR::ResetLatched.Create(
		( byte* )( MEM::FindPattern( CLIENT_DLL, _( "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 35 53 8B 5F 30" ) ) ),
		&Hooks::hkResetLatched ) )
		return false;


	const auto CInterpolatedVar_class_Vector_vtable{ ( void* ) MEM::FindPattern( CLIENT_DLL, _( "C0 8E ? ? ? ? ? 50 70 76 00 A5 ? ? ? ? 80 A6 ? ? ? ? ? 40 40 50 60 60 F0 6C 90 43" ) ) };
	if ( !CInterpolatedVar_class_Vector_vtable )
		return false;
	if ( !DTR::InterpolatedVarArrayBase_Reset.Create( MEM::GetVFunc( CInterpolatedVar_class_Vector_vtable, 0x14 ), &hkInterpolatedVarArrayBase_Reset ) )
		return false;
	
	if ( !DTR::CL_Move.Create(
		( byte* )( MEM::FindPattern( ENGINE_DLL, _( "55 8B EC 81 EC ? ? ? ? 53 56 8A F9 F3 0F 11 45 ? 8B 4D 04" ) ) ),
		&Hooks::hkCL_Move ) )
		return false;	

	if ( !DTR::CL_ReadPackets.Create(
		( byte* )( MEM::FindPattern( ENGINE_DLL, _( "53 8A D9 8B 0D ? ? ? ? 56 57 8B B9" ) ) ),
		&Hooks::hkCL_ReadPackets ) )
		return false;

	if ( !DTR::_Host_RunFrame_Client.Create(
		( byte* )( MEM::FindPattern( ENGINE_DLL, _( "55 8B EC 83 EC 08 53 56 8A D9 FF 15" ) ) ),
		&Hooks::hk_Host_RunFrame_Client ) )
		return false;	

	if ( !DTR::_Host_RunFrame_Input.Create(
		( byte* )( MEM::FindPattern( ENGINE_DLL, _( "55 8B EC 83 EC 10 53 8A D9 F3 0F 11 45" ) ) ),
		&Hooks::hk_Host_RunFrame_Input ) )
		return false;*/
		


#ifdef SERVER_DBGING
	if ( !DTR::ServerSetupBones.Create(
		( byte* )( MEM::FindPattern( SERVER_DLL, _( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 8B C1 56 57 89 44" ) ) ),
		&Hooks::hkServerSetupBones ) )
		return false;
#endif

	return true;
}

void Hooks::Restore( ) {
	PropManager::Get( ).Hook( m_bClientSideAnimation, _( "DT_BaseAnimating" ), _( "m_bClientSideAnimation" ) );
	PropManager::Get( ).Hook( m_flSimulationTime, _( "DT_BaseEntity" ), _( "m_flSimulationTime" ) );
	//PropManager::Get( ).Hook( m_flAbsYaw, _( "DT_CSRagdoll" ), _( "m_flAbsYaw" ) );
	//PropManager::Get( ).Hook( m_hWeapon, _( "DT_BaseViewModel" ), _( "m_hWeapon" ) );
	//PropManager::Get( ).Hook( m_flCycle, _( "DT_BaseViewModel" ), _( "m_flCycle" ) );
	//PropManager::Get( ).Hook( m_nSequence, _( "DT_BaseViewModel" ), _( "m_nSequence" ) );
	//PropManager::Get( ).Hook( m_flAnimTime, _( "DT_BaseViewModel" ), _( "m_flAnimTime" ) );

	MH_DisableHook( MH_ALL_HOOKS );
	MH_RemoveHook( MH_ALL_HOOKS );

	MH_Uninitialize( );
	
	if ( pOldWndProc )
		SetWindowLongPtrW( hWindow, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( pOldWndProc ) );

	// reset input state
	Interfaces::InputSystem->EnableInput( true );
}
#pragma endregion