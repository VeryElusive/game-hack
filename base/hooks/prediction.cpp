#include "../core/hooks.h"
#include "../context.h"
#include "../features/misc/logger.h"
#include "../features/rage/exploits.h"
#include <intrin.h>

void FASTCALL Hooks::hkProcessMovement( void* ecx, DWORD edx, CBasePlayer* basePlayer, CMoveData* moveData ) {
	static auto oProcessMovement{ DTR::ProcessMovement.GetOriginal<decltype( &hkProcessMovement )>( ) };

	// fix prediction errors when jumping
	moveData->bGameCodeMovedPlayer = false;

	oProcessMovement( ecx, edx, basePlayer, moveData );
}

void** STDCALL Hooks::hkFinishTrackPredictionErrors( CBasePlayer* pPlayer ) {
	static auto oFinishTrackPredictionErrors{ DTR::FinishTrackPredictionErrors.GetOriginal<decltype( &hkFinishTrackPredictionErrors )>( ) };

	/*if ( ctx.m_iFixedTickBase ) {
		ctx.m_pLocal->m_nTickBase( ) = ctx.m_iFixedTickBase;
		Interfaces::Globals->flCurTime = TICKS_TO_TIME( ctx.m_iFixedTickBase );
	}*/

	return oFinishTrackPredictionErrors( pPlayer );
}

CUserCmd* FASTCALL Hooks::hkGetUserCmd( uint8_t* ecx, uint8_t* edx, int slot, int seqnr ) {
	static auto oGetUserCmd{ DTR::GetUserCmd.GetOriginal<decltype( &hkGetUserCmd )>( ) };

	if ( !ctx.m_pLocal )
		return oGetUserCmd( ecx, edx, slot, seqnr );

	const auto& localData{ ctx.m_cLocalData.at( seqnr % 150 ) };

	const auto ret{ oGetUserCmd( ecx, edx, slot, seqnr ) };

	if ( reinterpret_cast< uintptr_t >( _ReturnAddress( ) ) == Displacement::Sigs.ReturnToPerformPrediction ) {
		if ( localData.m_bOverrideTickbase
			&& std::abs( ctx.m_pLocal->m_nTickBase( ) - localData.m_iAdjustedTickbase ) <= 17 )
			ctx.m_pLocal->m_nTickBase( ) = localData.m_iAdjustedTickbase;

		if ( ret && ret->iTickCount == INT_MAX )
			ret->bHasBeenPredicted = true;

		//if ( !ret || ret->iTickCount == INT_MAX || Config::Get<keybind_t>( Vars.DBGKeybind ).enabled )
		//	return nullptr;
	}

	return ret;
}