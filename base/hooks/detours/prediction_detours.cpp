#include "../../core/hooks.h"
#include "../../context.h"
#include "../../features/misc/engine_prediction.h"
#include "../../core/variables.h"
#include "../../features/misc/logger.h"
#include "../../features/rage/exploits.h"
#include <intrin.h>

void FASTCALL Hooks::hkPhysicsSimulate( CBasePlayer* player, int time ) {
	static auto oPhysicsSimulate = DTR::PhysicsSimulate.GetOriginal<decltype( &hkPhysicsSimulate )>( );
	if ( player != ctx.m_pLocal
		|| player->IsDead( )
		|| Interfaces::Globals->iTickCount == player->m_nSimulationTick( ) )
		return oPhysicsSimulate( player, time );

	//player->m_vphysicsCollisionState( ) = 0;

	auto cctx = &player->m_CmdContext( );

	if ( cctx->cmd.iTickCount == INT_MAX ) {
		player->m_nSimulationTick( ) = Interfaces::Globals->iTickCount;

		return Features::EnginePrediction.StoreNetvars( cctx->cmd.iCommandNumber );
	}

	Features::EnginePrediction.RestoreNetvars( cctx->cmd.iCommandNumber - 1 );


	/*const auto weapon = player->get_weapon( );
	if ( weapon
		&& weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER
		&& weapon->m_flPostponeFireReadyTime( ) == INT_MAX )
		weapon->m_flPostponeFireReadyTime( ) = hacks::g_eng_pred->postpone_fire_ready_time( );*/

	//const auto backup{ Displacement::Cvars.sv_showimpacts->GetInt( ) };
	//if ( Config::Get<bool>( Vars.VisClientBulletImpacts ) )
	//	Displacement::Cvars.sv_showimpacts->SetValue( 2 );

	auto& localData{ ctx.m_cLocalData.at( cctx->cmd.iCommandNumber % 150 ) };
	//auto& cmd{ *Interfaces::Input->GetUserCmd( Interfaces::ClientState->iLastOutgoingCommand ) };

	//player->m_nTickBase( ) = localData.m_iTickbase;

	//player->m_nOldButtons( ) = cmd.iButtons;
	oPhysicsSimulate( player, time );
	//player->m_nOldButtons( ) = cmd.iButtons;
	//ctx.m_pLocal->m_nTickBase( ) = backupTickbase + 1;

	//if ( ctx.m_iSentCmds.size( )
	//	&& std::find( ctx.m_iSentCmds.rbegin( ), ctx.m_iSentCmds.rend( ), cctx->cmd.iCommandNumber ) != ctx.m_iSentCmds.rend( ) ) {
	//	if ( ctx.m_pLocal->m_nTickBase( ) > ctx.m_iHighestTickbase )
	///		ctx.m_iHighestTickbase = ctx.m_pLocal->m_nTickBase( );
	//}

	//Displacement::Cvars.sv_showimpacts->SetValue( backup );

	//if ( ctx.m_iFixedTickBase )
	//	ctx.m_iFixedTickBase = 0;

	Features::EnginePrediction.StoreNetvars( cctx->cmd.iCommandNumber );
}

bool FASTCALL Hooks::hkPreThink( void* ecx, int edx, int a2 ) {
	static auto oPreThink = DTR::PreThink.GetOriginal<decltype( &hkPreThink )>( );

	const auto backupCurtime{ Interfaces::Globals->flCurTime };

	//if ( ctx.m_iFixedTickBase )
	//	Interfaces::Globals->flCurTime = TICKS_TO_TIME( ctx.m_iFixedTickBase );

	const auto ret{ oPreThink( ecx, edx, a2 ) };

	Interfaces::Globals->flCurTime = backupCurtime;

	return ret;
}

bool FASTCALL Hooks::hkItemPostFrame( void* ecx, int edx ) {
	static auto oItemPostFrame = DTR::ItemPostFrame.GetOriginal<decltype( &hkItemPostFrame )>( );

	const auto backupCurtime{ Interfaces::Globals->flCurTime };

	/*if ( ctx.m_iNextTickBase ) {
		auto& oldLocalData{ ctx.m_cLocalData.at( Interfaces::ClientState->iLastCommandAck % 150 ) };
		const auto delta{ ctx.m_pLocal->m_nTickBase( ) - ( oldLocalData.m_iTickbase + 1 ) };

		Interfaces::Globals->flCurTime = TICKS_TO_TIME( ctx.m_iNextTickBase + delta );
	}*/

	const auto ret{ oItemPostFrame( ecx, edx ) };

	Interfaces::Globals->flCurTime = backupCurtime;

	return ret;
}

void FASTCALL Hooks::hkSelectItem( void* ecx, int edx, int a2 ) {
	/*static auto oSelectItem = DTR::SelectItem.GetOriginal<decltype( &hkSelectItem )>( );
	
	const auto backupCurtime{ Interfaces::Globals->flCurTime };

	if ( ctx.m_iFixedTickBase )
		Interfaces::Globals->flCurTime = TICKS_TO_TIME( ctx.m_iFixedTickBase );

	oSelectItem( ecx, edx, a2 );

	Interfaces::Globals->flCurTime = backupCurtime; */
}

bool FASTCALL Hooks::hkC_BaseViewModel__Interpolate( CBaseViewModel* ecx, int edx, float a2 ) {
	static auto oC_BaseViewModel__Interpolate = DTR::C_BaseViewModel__Interpolate.GetOriginal<decltype( &hkC_BaseViewModel__Interpolate )>( );

	const auto backupCurtime{ Interfaces::Globals->flCurTime };
	const auto backupOwnerHandle{ ecx->m_bPredictable( ) };

	Interfaces::Globals->flCurTime = ecx->m_flAnimTime( ) + Interfaces::Globals->flRealTime - ctx.m_flLastAnimTimeUpdate;

	const auto backupAnimTime{ ecx->m_flAnimTime( ) };

	ecx->m_bPredictable( ) = false;

	//originalCurtime = a2;
	const auto ret{ oC_BaseViewModel__Interpolate( ecx, edx, Interfaces::Globals->flCurTime ) };
	//originalCurtime = 0.f;

	Interfaces::Globals->flCurTime = backupCurtime;
	ecx->m_bPredictable( ) = backupOwnerHandle;

	return ret;
}


//C_BaseEntity
void FASTCALL Hooks::hkResetLatched( void* ecx, int edx ) {
	static auto oResetLatched = DTR::ResetLatched.GetOriginal<decltype( &hkResetLatched )>( );

	if ( ctx.m_pLocal ) {
		if ( const auto viewModel{ static_cast< CBaseViewModel* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( ctx.m_pLocal->m_hViewModel( ) ) ) }; viewModel ) {
			if ( ecx != ctx.m_pLocal
				&& ecx != ctx.m_pWeapon
				&& ecx != viewModel )
				oResetLatched( ecx, edx );
		}
	}
	else
		oResetLatched( ecx, edx );
}