#include "engine_prediction.h"
#include "logger.h"

void CEnginePrediction::RunCommand( CUserCmd& cmd ) {
	if ( !ctx.m_pWeapon )
		return;

	m_flCurtime = Interfaces::Globals->flCurTime;
	m_flFrametime = Interfaces::Globals->flFrameTime;

	Interfaces::Globals->flCurTime = ctx.m_flFixedCurtime;
	Interfaces::Globals->flFrameTime = Interfaces::Prediction->bEnginePaused ? 0.f : Interfaces::Globals->flIntervalPerTick;

	const auto backupInPrediction{ Interfaces::Prediction->bInPrediction };
	const auto backupFirstTimePrediction{ Interfaces::Prediction->Split->bIsFirstTimePredicted };

	Interfaces::Prediction->bInPrediction = true;
	Interfaces::Prediction->Split->bIsFirstTimePredicted = false;

	ctx.m_bProhibitSounds = true;
	Interfaces::Prediction->CheckMovingGround( ctx.m_pLocal, Interfaces::Globals->flFrameTime );
	Interfaces::MoveHelper->SetHost( ctx.m_pLocal );
	Interfaces::Prediction->SetupMove( ctx.m_pLocal, &cmd, Interfaces::MoveHelper, &MoveData );
	Interfaces::GameMovement->ProcessMovement( ctx.m_pLocal, &MoveData );
	Interfaces::Prediction->FinishMove( ctx.m_pLocal, &cmd, &MoveData );
	Interfaces::MoveHelper->SetHost( nullptr );
	ctx.m_bProhibitSounds = false;

	if ( ctx.m_pLocal->m_fFlags( ) & FL_DUCKING ) {
		ctx.m_pLocal->SetCollisionBounds( { -16.f, -16.f, 0.f }, { 16.f, 16.f, 54.f } );
		ctx.m_pLocal->m_vecViewOffset( ).z = 46.f;
	}
	else {
		ctx.m_pLocal->SetCollisionBounds( { -16.f, -16.f, 0.f }, { 16.f, 16.f, 72.f } );
		ctx.m_pLocal->m_vecViewOffset( ).z = 64.f;
	}

	ctx.m_pWeapon->UpdateAccuracyPenalty( );

	Spread = ctx.m_pWeapon->GetSpread( );
	Inaccuracy = ctx.m_pWeapon->GetInaccuracy( );

	Interfaces::Prediction->bInPrediction = backupInPrediction;
	Interfaces::Prediction->Split->bIsFirstTimePredicted = backupFirstTimePrediction;

}

void CEnginePrediction::Finish( ) {
	if ( !ctx.m_pWeapon )
		return;

	Interfaces::Globals->flCurTime = m_flCurtime;
	Interfaces::Globals->flFrameTime = m_flFrametime;
}

void CEnginePrediction::RestoreNetvars( int slot ) {
	const auto& local{ ctx.m_pLocal };
	if ( !local || local->IsDead( ) )
		return;

	const auto& data{ m_cCompressionVars.at( slot % 150 ) };
	if ( data.m_iCommandNumber != slot )
		return;

	const auto aimPunchAngleVelDiff{ local->m_aimPunchAngleVel( ) - data.m_aimPunchAngleVel };
	const auto aimPunchAngleDiff{ local->m_aimPunchAngle( ) - data.m_aimPunchAngle };

	if ( std::abs( aimPunchAngleDiff.x ) <= 0.03125f 
		&& std::abs( aimPunchAngleDiff.y ) <= 0.03125f 
		&& std::abs( aimPunchAngleDiff.z ) <= 0.03125f )
		local->m_aimPunchAngle( ) = data.m_aimPunchAngle;

	if ( std::abs( aimPunchAngleVelDiff.x ) <= 0.03125f 
		&& std::abs( aimPunchAngleVelDiff.y ) <= 0.03125f 
		&& std::abs( aimPunchAngleVelDiff.z ) <= 0.03125f )
		local->m_aimPunchAngleVel( ) = data.m_aimPunchAngleVel;

	auto& viewOffset{ local->m_vecViewOffset( ) };
	if ( viewOffset.z > 46.f && viewOffset.z < 47.f )
		viewOffset.z = 46.f;
	else if ( viewOffset.z > 64.f )
		viewOffset.z = 64.f;
}

void CEnginePrediction::StoreNetvars( int slot ) {
	const auto& local{ ctx.m_pLocal };
	if ( !local || local->IsDead( ) )
		return;

	auto& data = m_cCompressionVars.at( slot % 150 );

	data.m_aimPunchAngleVel = local->m_aimPunchAngleVel( );
	data.m_aimPunchAngle = local->m_aimPunchAngle( );
	data.m_vecViewOffsetZ = local->m_vecViewOffset( ).z;
}

#define FTYPEDESC_INSENDTABLE 0x0100		// This field is present in a network SendTable
#define FTYPEDESC_NOERRORCHECK 0x400 
bool CEnginePrediction::ModifyDatamap( ) {
	auto map{ ctx.m_pLocal->GetPredDescMap( ) };

	bool ret{ };

	while ( map ) {
		// BEGIN_PREDICTION_DATA( C_CSPlayer )
		if ( FNV1A::Hash( map->szDataClassName ) == FNV1A::HashConst( ( "C_CSPlayer" ) ) ) {
			auto data( new TypeDescription_t[ map->nDataFields + 2 ] );
			std::memcpy( data, map->pDataDesc, map->nDataFields * sizeof TypeDescription_t );

			// extend datamap
			map->pDataDesc = data;
			map->nDataFields += 2;
			map->iPackedSize += sizeof TypeDescription_t;

			// add m_flVelocityModifier
			TypeDescription_t velocityModifier{ };
			velocityModifier.szFieldName = _( "m_flVelocityModifier" );
			velocityModifier.fFlags = FTYPEDESC_INSENDTABLE;
			velocityModifier.fieldTolerance = 0.01f;
			velocityModifier.iFieldOffset = Displacement::Netvars->m_flVelocityModifier;
			velocityModifier.uFieldSize = 0x1;
			velocityModifier.fieldSizeInBytes = 0x4;
			velocityModifier.iFieldType = 0x1;
			velocityModifier.flatOffset[ TD_OFFSET_NORMAL ] = Displacement::Netvars->m_flVelocityModifier;
			map->pDataDesc[ map->nDataFields - 1 ] = velocityModifier;

			// add m_bWaitForNoAttack
			TypeDescription_t waitForNoAttack{ };
			waitForNoAttack.szFieldName = _( "m_bWaitForNoAttack" );
			waitForNoAttack.fFlags = FTYPEDESC_INSENDTABLE;
			waitForNoAttack.fieldTolerance = 0x0;
			waitForNoAttack.iFieldOffset = Displacement::Netvars->m_bWaitForNoAttack;
			waitForNoAttack.uFieldSize = 0x1;
			waitForNoAttack.fieldSizeInBytes = 0x1;
			waitForNoAttack.iFieldType = 0x6;
			waitForNoAttack.flatOffset[ TD_OFFSET_NORMAL ] = Displacement::Netvars->m_bWaitForNoAttack;
			map->pDataDesc[ map->nDataFields - 2 ] = waitForNoAttack;

			map->pOptimizedDataMap = nullptr;

			ret = true;
		}

		map = map->pBaseMap;
	}

	const auto m_flNextPrimaryAttack{ MEM::GetTypeDescription( ctx.m_pWeapon->GetPredDescMap( ), _( "m_flNextPrimaryAttack" ) ) };
	m_flNextPrimaryAttack->fFlags = FTYPEDESC_INSENDTABLE;

	const auto m_flNextSecondaryAttack{ MEM::GetTypeDescription( ctx.m_pWeapon->GetPredDescMap( ), _( "m_flNextSecondaryAttack" ) ) };
	m_flNextSecondaryAttack->fFlags = FTYPEDESC_INSENDTABLE;

	if ( ctx.m_pLocal->m_hViewModel( ) ) {
		// BEGIN_PREDICTION_DATA( CBaseViewModel )
		if ( const auto viewModel{ static_cast< CBaseViewModel* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( ctx.m_pLocal->m_hViewModel( ) ) ) }; viewModel ) {
			const auto dmap{ viewModel->GetPredDescMap( ) };

			// FTYPEDESC_PRIVATE | FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK
			const auto m_flAnimTime{ MEM::GetTypeDescription( dmap, _( "m_flAnimTime" ) ) };
			if ( m_flAnimTime->fFlags != ( 0x0200 | 0x0080 | 0x0400 ) )
				m_flAnimTime->fFlags = 0x0200 | 0x0080 | 0x0400;

			const auto m_nAnimationParity{ MEM::GetTypeDescription( dmap, _( "m_nAnimationParity" ) ) };
			m_nAnimationParity->fFlags |= FTYPEDESC_NOERRORCHECK;

			const auto m_nSequence{ MEM::GetTypeDescription( dmap, _( "m_nSequence" ) ) };
			m_nSequence->fFlags |= FTYPEDESC_NOERRORCHECK;

			dmap->pOptimizedDataMap = nullptr;
		}
	}

	if ( ret ) {
		Interfaces::Prediction->ShutDownPredictables( );
		Interfaces::Prediction->ReinitPredictables( );
	}

	return ret;
}