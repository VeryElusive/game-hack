#include "../core/hooks.h"
#include "../context.h"
#include "../features/visuals/visuals.h"
#include "../features/rage/exploits.h"

void FASTCALL Hooks::hkPacketEnd( void* ecx, void* edx ) {
	static auto oPacketEnd = DTR::PacketEnd.GetOriginal<decltype( &Hooks::hkPacketEnd )>( );

	oPacketEnd( ecx, edx );

	auto& localData{ ctx.m_cLocalData.at( Interfaces::ClientState->iCommandAck % 150 ) };

	Features::Visuals.DormantESP.GetActiveSounds( );

	if ( ctx.m_iSentCmds.empty( )
		|| std::find( ctx.m_iSentCmds.rbegin( ), ctx.m_iSentCmds.rend( ), Interfaces::ClientState->iCommandAck ) == ctx.m_iSentCmds.rend( ) )
		return;

	static auto last{ Interfaces::ClientState->iCommandAck };

	if ( Interfaces::ClientState->iCommandAck != last ) {
		last = Interfaces::ClientState->iCommandAck;
		int lastDelta{ std::min( Interfaces::Globals->iTickCount - localData.m_iTickCount, 64 ) };
		for ( int i{ }; i < 4; ++i ) {
			const auto tmp{ ctx.m_iLast4Deltas[ i ] };
			ctx.m_iLast4Deltas[ i ] = lastDelta;
			lastDelta = tmp;
		}
	}

	if ( ctx.m_pLocal
		&& localData.m_flSpawnTime == ctx.m_pLocal->m_flSpawnTime( ) ) {
		//if ( !localData.m_iAdjustedTickbase && ctx.m_pLocal->m_nTickBase( ) - 1 != localData.m_iTickbase )
		//	Features::Logger.Log( "Differed 1: " + std::to_string( ctx.m_pLocal->m_nTickBase( ) - 1 ) + " | " + std::to_string( localData.m_iTickbase ), false );

		//if ( localData.m_iAdjustedTickbase && ctx.m_pLocal->m_nTickBase( ) - 1 != localData.m_iAdjustedTickbase )
		//	Features::Logger.Log( "Differed 2: " + std::to_string( ctx.m_pLocal->m_nTickBase( ) - 1 ) + " | " + std::to_string( localData.m_iAdjustedTickbase ), false );

		//localData.m_iTickbase = localData.m_iAdjustedTickbase = ctx.m_pLocal->m_nTickBase( ) - 1;
	}

}

void FASTCALL Hooks::hkPacketStart( void* ecx, void* edx, int in_seq, int out_acked ) {
	static auto oPacketStart = DTR::PacketStart.GetOriginal<decltype( &Hooks::hkPacketStart )>( );
	
	if ( !ctx.m_pLocal
		|| ctx.m_pLocal->IsDead( ) )
		return oPacketStart( ecx, edx, in_seq, out_acked );

	/*auto& localData{ ctx.m_cLocalData.at( Interfaces::ClientState->iLastCommandAck % 150 ) };

	if ( localData.m_flSpawnTime == ctx.m_pLocal->m_flSpawnTime( )
		&& !localData.m_iShiftAmount
		&& !localData.m_bOverrideTickbase
		&& localData.m_iAdjustedTickbase ) {
		localData.m_iAdjustedTickbase = 0;
	}*/

	if ( ctx.m_iSentCmds.empty( )
		|| std::find( ctx.m_iSentCmds.rbegin( ), ctx.m_iSentCmds.rend( ), out_acked ) == ctx.m_iSentCmds.rend( ) )
		return;

	ctx.m_iSentCmds.erase(
		std::remove_if(
			ctx.m_iSentCmds.begin( ), ctx.m_iSentCmds.end( ),
			[ & ]( const int cmd_number ) {
				return cmd_number < out_acked;
			}
		),
		ctx.m_iSentCmds.end( )
				);

	return oPacketStart( ecx, edx, in_seq, out_acked );
}

bool FASTCALL Hooks::hkProcessTempEntities( void* ecx, void* edx, void* msg ) {
	static auto oProcessTempEntities = DTR::ProcessTempEntities.GetOriginal<decltype( &Hooks::hkProcessTempEntities )>( );

	const auto ret = oProcessTempEntities( ecx, edx, msg );
	
	if ( ctx.m_pLocal && !ctx.m_pLocal->IsDead( ) ) {
		for ( auto i = Interfaces::ClientState->pEvents; i; i = Interfaces::ClientState->pEvents->next ) {
			if ( i->iClassID == 0 )
				continue;

			i->flFireDelay = 0.f;
		}
	}

	Interfaces::Engine->FireEvents( );

	return ret;
}

bool FASTCALL Hooks::hkSendNetMsg( INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice ) {
	static auto oSendNetMsg = DTR::SendNetMsg.GetOriginal<decltype( &Hooks::hkSendNetMsg )>( );

	if ( msg.GetType( ) == 14 ) // Return and don't send messsage if its FileCRCCheck
		return false;

	if ( ctx.m_pLocal ) {
		if ( msg.GetGroup( ) == 9 ) // Fix lag when transmitting voice and fakelagging
			bVoice = true;

		return oSendNetMsg( pNetChan, edx, msg, bForceReliable, bVoice );
	}

	return oSendNetMsg( pNetChan, edx, msg, bForceReliable, bVoice );
}

// TODO: experiment by only setting this during createmove when we dont send a packet, alternatively, just skip this if we shoot?
int FASTCALL Hooks::hkSendDatagram( INetChannel* thisptr, int edx, bf_write* pDatagram ) {
	static auto oSendDatagram = DTR::SendDatagram.GetOriginal<decltype( &hkSendDatagram )>( );

	INetChannelInfo* pNetChannelInfo = Interfaces::Engine->GetNetChannelInfo( );

	if ( !ctx.m_pLocal || pDatagram || !pNetChannelInfo || !Config::Get<bool>( Vars.MiscFakePing ) || ctx.m_pLocal->IsDead( ) )
		return oSendDatagram( thisptr, edx, pDatagram );

	const int iOldInReliableState = thisptr->iInReliableState;
	const int iOldInSequenceNr = thisptr->iInSequenceNr;

	const float flMaxLatency = std::max( 0.f, Displacement::Cvars.sv_maxunlag->GetFloat( ) - pNetChannelInfo->GetLatency( FLOW_OUTGOING ) );
	Features::Misc.AddLatencyToNetChannel( thisptr, flMaxLatency );

	const int iReturn = oSendDatagram( thisptr, edx, pDatagram );

	thisptr->iInReliableState = iOldInReliableState;
	thisptr->iInSequenceNr = iOldInSequenceNr;

	return iReturn;
}