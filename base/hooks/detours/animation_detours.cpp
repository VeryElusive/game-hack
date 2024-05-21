#include "../../core/hooks.h"
#include "../../context.h"
#include "../../features/rage/antiaim.h"
#include "../../features/animations/animation.h"
#include "../../features/visuals/visuals.h"
#include <intrin.h>

void FASTCALL Hooks::hkStandardBlendingRules( CBasePlayer* const ent, const std::uintptr_t edx, CStudioHdr* const mdl_data, Vector* a1, Quaternion* a2, float a3, int mask ) {
	static auto oStandardBlendingRules = DTR::StandardBlendingRules.GetOriginal<decltype( &hkStandardBlendingRules )>( );
	if ( !ent || !ctx.m_pLocal || !ent->IsPlayer( ) )
		return oStandardBlendingRules( ent, edx, mdl_data, a1, a2, a3, mask );

	//if ( ctx.m_bSetupBones )
	//	mask = BONE_USED_BY_HITBOX;

	static auto lookupBone{ *reinterpret_cast< int( __thiscall* )( void*, const char* ) >( Displacement::Sigs.LookupBone ) };

	//Vector pos[ 256 ];
	//Quaternion q[ 256 ];

	const auto bone_index = lookupBone( ent, _( "lean_root" ) );
	if ( bone_index == -1 )
		return oStandardBlendingRules( ent, edx, mdl_data, a1, a2, a3, mask );

	auto& boneFlags{ mdl_data->vecBoneFlags[ bone_index ] };

	const auto backupBoneFlags = boneFlags;

	boneFlags = 0u;

	oStandardBlendingRules( ent, edx, mdl_data, a1, a2, a3, mask );

	boneFlags = backupBoneFlags;

	/*uint8_t bone_computed[ 256 ]; std::memset( bone_computed, 0, 256 );
	matrix3x4a_t parent_transform; parent_transform.SetAngles( { 0.f, ent->GetAbsAngles( ).y, 0.f } ); parent_transform.SetOrigin( ent->GetAbsOrigin( ) );
	ent->BuildTransformations( ent->m_pStudioHdr( ), a1, a2, parent_transform, BONE_USED_BY_SERVER, bone_computed );*/
}

void FASTCALL Hooks::hkDoExtraBonesProcessing( void* ecx, uint32_t edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_computed, void* context ) {
	static auto oDoExtraBonesProcessing = DTR::DoExtraBonesProcessing.GetOriginal<decltype( &hkDoExtraBonesProcessing )>( );
	if ( ecx == ctx.m_pLocal ) {
		const auto backupMoveType{ ctx.m_pLocal->m_MoveType( ) };

		ctx.m_pLocal->m_MoveType( ) = MOVETYPE_OBSERVER; // not walk! just wanted to confuse ppl!

		oDoExtraBonesProcessing( ecx, edx, hdr, pos, q, matrix, bone_computed, context );

		ctx.m_pLocal->m_MoveType( ) = backupMoveType;
	}
}

bool FASTCALL Hooks::hkShouldSkipAnimFrame( void* ecx, uint32_t ebx ) {
	return false;
}

void FASTCALL Hooks::hkOnNewCollisionBounds( CBasePlayer* ecx, uint32_t edx, Vector* oldMins, Vector* newMins, Vector* oldMaxs, Vector* newMaxs ) {
	static auto oOnNewCollisionBounds = DTR::OnNewCollisionBounds.GetOriginal<decltype( &hkOnNewCollisionBounds )>( );

	/*oOnNewCollisionBounds( ecx, edx, oldMins, newMins, oldMaxs, newMaxs );

	if ( ecx != ctx.m_pLocal )
	Features::Logger.Log( ( "CLIENT: " + std::to_string( ecx->m_flNewBoundsTime( ) ) + " maxs: " +
		std::to_string( ecx->m_flNewBoundsMaxs( ) ) ).c_str( ), true );

	static auto oOnNewCollisionBounds = DTR::OnNewCollisionBounds.GetOriginal<decltype( &hkOnNewCollisionBounds )>( );

	const auto backupCurtime{ Interfaces::Globals->flCurTime };
	Interfaces::Globals->flCurTime = ecx->m_flSimulationTime( );

	oOnNewCollisionBounds( ecx, edx, oldMins, newMins, oldMaxs, newMaxs );

	Interfaces::Globals->flCurTime = backupCurtime;*/

	//Features::Logger.Log( std::to_string( Interfaces::Globals->flRealTime ), true );

	// rebuild of this function
	//*( float* )( uintptr_t( ecx ) + Displacement::Netvars->m_bIsScoped - 0x54 ) = *( float* )( uintptr_t( ecx ) + Displacement::Netvars->m_fFlags - 0x50 ) + oldMaxs->z;
	//*( float* )( uintptr_t( ecx ) + Displacement::Netvars->m_bIsScoped - 0x50 ) = ecx->m_flSimulationTime( );
}

#ifdef SERVER_DBGING
void FASTCALL Hooks::hkServerSetupBones( CBaseAnimating* ecx, int edx, matrix3x4a_t* pBoneToWorld, int boneMask ) {
	static auto oServerSetupBones{ DTR::ServerSetupBones.GetOriginal<decltype( &hkServerSetupBones )>( ) };

	const QAngle absrotation{ *reinterpret_cast< QAngle* >( ( reinterpret_cast< std::uintptr_t >( ecx ) + 0x1e8 ) ) };

	oServerSetupBones( ecx, edx, pBoneToWorld, boneMask );
}
#endif

void FASTCALL Hooks::hkClampBonesInBBox( CBasePlayer* ecx, uint32_t edx, matrix3x4_t* bones, int boneMask ) {
	
}

bool FASTCALL Hooks::hkSetupbones( const std::uintptr_t ecx, const std::uintptr_t edx, matrix3x4_t* bones, 
	int max_bones, int mask, float time ) {
	static auto oSetupbones{ DTR::Setupbones.GetOriginal<decltype( &hkSetupbones )>( ) };
	const auto player = reinterpret_cast< CBasePlayer* >( ecx - sizeof( std::uintptr_t ) );
	const auto boneCount{ player->m_iBoneCount( ) };

	if ( ctx.m_bInCreatemove ) {
		if ( !bones
			|| max_bones == -1 )
			return true;

		std::memcpy(
			bones, player->m_CachedBoneData( ).Base( ),
			boneCount * sizeof( matrix3x4_t )
		);

		//if ( mask & BONE_USED_BY_ATTACHMENT )
		//	player->SetupBonesAttachmentHelper( );

		return true;
	}

	if ( player->IsDead( )
		|| !player->IsPlayer( ) )
		return oSetupbones( ecx, edx, bones, max_bones, mask, time );

	auto& entry = Features::AnimSys.m_arrEntries.at( player->Index( ) - 1 );
	if ( entry.m_pPlayer != player
		&& player != ctx.m_pLocal )
		return oSetupbones( ecx, edx, bones, max_bones, mask, time );

	const auto absOrigin{ player->GetAbsOrigin( ) };
	if ( entry.m_vecLastMergeOrigin != absOrigin ) {
		const auto org{ player == ctx.m_pLocal ? ctx.m_vecSetupBonesOrigin : entry.m_vecUpdatedOrigin };
		const auto delta{ player->GetAbsOrigin( ) - org };

		auto& mat{ player == ctx.m_pLocal ? ctx.m_matRealLocalBones : entry.m_matMatrix };

		for ( std::size_t i{ }; i < boneCount; ++i ) {
			auto& bone{ player->m_CachedBoneData( ).Base( )[ i ] };
			auto& boneNew{ mat[ i ] };

			bone.SetOrigin( boneNew.GetOrigin( ) + delta );
		}

		entry.m_vecLastMergeOrigin = absOrigin;
	}

	if ( mask & BONE_USED_BY_ATTACHMENT )
		player->SetupBonesAttachmentHelper( );

	if ( bones ) {
		std::memcpy(
			bones, player->m_CachedBoneData( ).Base( ),
			boneCount * sizeof( matrix3x4_t )
		);
	}

	return true;
}
void FASTCALL Hooks::hkUpdateClientsideAnimation( CBasePlayer* ecx, void* edx ) {
	static auto oUpdateClientsideAnimation{ DTR::UpdateClientsideAnimation.GetOriginal<decltype( &hkUpdateClientsideAnimation )>( ) };
	if ( ecx->IsDead( )
		|| !ecx->IsPlayer( ) )
		return oUpdateClientsideAnimation( ecx, edx );

	const auto backup{ *ecx->m_pAnimState( ) };
	if ( !ctx.m_bUpdatingAnimations ) {
		if ( ecx != ctx.m_pLocal )
			return;
	}

	oUpdateClientsideAnimation( ecx, edx );

	if ( !ctx.m_bUpdatingAnimations
		&& ecx == ctx.m_pLocal )
		*ecx->m_pAnimState( ) = backup;
}

void __vectorcall Hooks::HkAnimStateUpdate( void* ecx, void*, float, float, float, void* ) {
	// fuck your game calls i wanna do this myself >:(

}

void FASTCALL Hooks::hkAccumulateLayers( CBasePlayer* const ecx, const std::uintptr_t edx, int a0, int a1, float a2, int a3 ) {
	static auto oAccumulateLayers = DTR::AccumulateLayers.GetOriginal<decltype( &hkAccumulateLayers )>( );
	if ( !ecx->IsPlayer( ) )
		return oAccumulateLayers( ecx, edx, a0, a1, a2, a3 );

	if ( const auto state = ecx->m_pAnimState( ) ) {
		const auto backupFirstUpdate = state->bFirstUpdate;

		// bone snapshots fix
		state->bFirstUpdate = true;

		oAccumulateLayers( ecx, edx, a0, a1, a2, a3 );

		state->bFirstUpdate = backupFirstUpdate;

		return;
	}

	oAccumulateLayers( ecx, edx, a0, a1, a2, a3 );
}

bool FASTCALL Hooks::hkIsBoneAvailable( void* ecx, uint32_t edx, int a1 ) {
	static auto oIsBoneAvailable = DTR::IsBoneAvailable.GetOriginal<decltype( &hkIsBoneAvailable )>( );

	// skip this check since its not present on the server
	if ( *reinterpret_cast< std::uintptr_t* >( _AddressOfReturnAddress( ) ) == Displacement::Sigs.ReturnToClampBonesInBBox )
		return true;

	return oIsBoneAvailable( ecx, edx, a1 );
}

void FASTCALL Hooks::hkPostDataUpdate( void* ecx, int edx, int update ) {
	static auto oPostDataUpdate = DTR::PostDataUpdate.GetOriginal<decltype( &hkPostDataUpdate )>( );
	const auto player{ reinterpret_cast< CBasePlayer* >( reinterpret_cast< int >( ecx ) - 8 ) };
	if ( !player || !player->IsPlayer( ) || player == ctx.m_pLocal || player->IsDead( ) )
		return oPostDataUpdate( ecx, edx, update );

	const auto backupSimTime{ player->m_flSimulationTime( ) };

	player->m_flSimulationTime( ) = Interfaces::Globals->flRealTime;

	oPostDataUpdate( ecx, edx, update );

	player->m_flSimulationTime( ) = backupSimTime;
}

void FASTCALL Hooks::hkSetUpMovement( CCSGOPlayerAnimState * ecx, int edx ) {
	static auto oSetUpMovement = DTR::SetUpMovement.GetOriginal<decltype( &hkSetUpMovement )>( );
	if ( !ctx.m_pLocal
		|| ecx->m_pPlayer == ctx.m_pLocal
		|| ecx == ctx.m_pLocal->m_pAnimState( )
		|| ctx.m_bExtrapolating )
		return oSetUpMovement( ecx, edx );

	const auto m_bOnGround{ bool( ecx->m_pPlayer->m_fFlags( ) & FL_ONGROUND ) };

	const auto backupLanding{ ecx->m_bLanding };

	// REMEMBER: m_bJumping is actaully bSmoothHeightValid
	auto MOVEMENT_LAND_OR_CLIMB{ &ecx->m_pPlayer->m_AnimationLayers( )[ 5 ] };
	auto MOVEMENT_JUMP_OR_FALL{ &ecx->m_pPlayer->m_AnimationLayers( )[ 4 ] };

	if ( ecx->m_pPlayer->m_flNextAttack( ) && TIME_TO_TICKS( ecx->m_pPlayer->m_flNextAttack( ) ) == TIME_TO_TICKS( Interfaces::Globals->flCurTime ) ) {// even tho it should be exact
		ecx->SetLayerSequence( MOVEMENT_JUMP_OR_FALL, ACT_CSGO_JUMP );
		ecx->m_bJumping = true;
	}

	const auto m_bLandedOnGroundThisFrame = ( !ecx->bFirstUpdate && ecx->bOnGround != m_bOnGround && m_bOnGround );
	const auto m_bLeftTheGroundThisFrame = ( ecx->bOnGround != m_bOnGround && !m_bOnGround );
	const auto m_bOnLadder = !m_bOnGround && ecx->m_pPlayer->m_MoveType( ) == MOVETYPE_LADDER;

	const auto bStoppedLadderingThisFrame = ( ecx->bOnLadder && !m_bOnLadder );

	if ( m_bOnGround ) {
		if ( !ecx->m_bLanding && ( m_bLandedOnGroundThisFrame || bStoppedLadderingThisFrame ) ) {
			ecx->SetLayerSequence( MOVEMENT_LAND_OR_CLIMB, ecx->flDurationInAir > 1.f ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT );
			ecx->m_bLanding = true;
		}

		if ( ecx->m_bLanding && ecx->m_pPlayer->GetSequenceActivity( MOVEMENT_LAND_OR_CLIMB->nSequence ) != ACT_CSGO_CLIMB_LADDER ) {
			ecx->m_bJumping = false;

			if ( MOVEMENT_JUMP_OR_FALL->flCycle + ( Interfaces::Globals->flFrameTime * MOVEMENT_JUMP_OR_FALL->flPlaybackRate ) * 2 >= 1.f )
				ecx->m_bLanding = false;
		}

		// ladder weight not set leading to 1 tick difference
		//if ( !ecx->m_bLanding && !ecx->m_bJumping && ecx->m_flLadderWeight <= 0 )
		//	MOVEMENT_LAND_OR_CLIMB->flWeight = 0;
	}
	else if ( !m_bOnLadder ) {
		ecx->m_bLanding = false;

		if ( m_bLeftTheGroundThisFrame || bStoppedLadderingThisFrame ) {
			if ( !ecx->m_bJumping )
				ecx->SetLayerSequence( MOVEMENT_JUMP_OR_FALL, ACT_CSGO_FALL );
		}
	}

	ecx->m_bLanding = backupLanding;

	return oSetUpMovement( ecx, edx );
}