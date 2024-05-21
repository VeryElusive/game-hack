#pragma once
#include "sdk/datatypes/vector.h"
#include "sdk/entity.h"
#include "core/interfaces.h"
#include <deque>

struct PredictedNetvars_t {
	int m_MoveType{ };
	int m_iFlags{ };
	Vector m_vecOrigin{ };
	Vector m_vecVelocity{ };
	float m_flDuckAmount{ };
	int m_nTickBase{ };
};

struct FakeAnimData_t {
	CCSGOPlayerAnimState m_sState{ };
	bool init{ };
	matrix3x4a_t m_matMatrix[ 256 ];
};

struct SequenceObject_t {
	SequenceObject_t( int iInReliableState, int iOutReliableState, int iSequenceNr, float flCurrentTime )
		: iInReliableState( iInReliableState ), iOutReliableState( iOutReliableState ), iSequenceNr( iSequenceNr ), flCurrentTime( flCurrentTime ) { }

	int iInReliableState;
	int iOutReliableState;
	int iSequenceNr;
	float flCurrentTime;
};

struct LocalData_t {
	int m_iCommandNumber{ };

	int m_iTickCount{ };
	int m_MoveType{ };
	int m_iTickbase{ };
	int m_iAdjustedTickbase{ };
	bool m_bOverrideTickbase{ };

	bool m_bCanAA{ };

	float m_flSpawnTime{ };

	PredictedNetvars_t PredictedNetvars{ };


	FORCEINLINE void Reset( ) {
		m_iTickbase = 0;
		m_iCommandNumber = 0;

		m_iAdjustedTickbase = 0;
		m_bOverrideTickbase = 0;

		m_flSpawnTime = 0;

		PredictedNetvars = { };
	}

	FORCEINLINE void Save( CBasePlayer* local, CUserCmd& cmd, CWeaponCSBase* weapon ) {
		this->m_flSpawnTime = local->m_flSpawnTime( );
		this->m_bOverrideTickbase = false;
		this->m_iCommandNumber = cmd.iCommandNumber;
		this->m_iTickbase = local->m_nTickBase( );
		this->m_iAdjustedTickbase = 0;
		this->m_MoveType = local->m_MoveType( );
	}

	FORCEINLINE void SavePredVars( CBasePlayer* local, CUserCmd& cmd ) {
		this->PredictedNetvars.m_MoveType = local->m_MoveType( );
		this->PredictedNetvars.m_iFlags = local->m_fFlags( );
		this->PredictedNetvars.m_nTickBase = local->m_nTickBase( );
		this->PredictedNetvars.m_vecOrigin = local->m_vecOrigin( );
		this->PredictedNetvars.m_vecVelocity = local->m_vecVelocity( );
		this->PredictedNetvars.m_flDuckAmount = local->m_flDuckAmount( );
	}

	FORCEINLINE void RestorePredVars( CBasePlayer* local ) {
		local->m_MoveType( ) = this->PredictedNetvars.m_MoveType;
		local->m_fFlags( ) = this->PredictedNetvars.m_iFlags;
		local->m_vecOrigin( ) = this->PredictedNetvars.m_vecOrigin;
		local->m_vecVelocity( ) = local->m_vecAbsVelocity( ) = this->PredictedNetvars.m_vecVelocity;
		local->m_flDuckAmount( ) = this->PredictedNetvars.m_flDuckAmount;
	}
};

struct LocalData_t;

struct HAVOCCTX {
	Vector2D m_ve2ScreenSize{ };
	CBasePlayer* m_pLocal{ };
	CWeaponCSBase* m_pWeapon{ };
	CCSWeaponData* m_pWeaponData{ };

	//EClientFrameStage m_iLastFSNStage{ };

	matrix3x4a_t m_matRealLocalBones[ 256 ];

	FakeAnimData_t m_cFakeData{ };

	QAngle m_angOriginalViewangles{ };

	Vector m_vecEyePos{ };

	Vector m_vecSetupBonesOrigin{ };

	std::array< LocalData_t, MULTIPLAYER_BACKUP > m_cLocalData{ };

	std::vector<int> m_iSentCmds{ };
	std::vector<std::shared_ptr< std::string > > m_strDbgLogs{ };

	//std::vector<std::pair<QAngle, int>> m_pQueuedCommands{ };

	CAnimationLayer m_pAnimationLayers[ 13 ];

	bool m_bCanShoot{ };
	bool m_bCanPenetrate{ };
	//bool m_bSetupBones{ };
	bool m_bFakeDucking{ };
	bool m_bSendPacket{ };
	//bool m_bNewPacket{ };
	bool m_bClearKillfeed{ };
	bool m_bExploitsEnabled{ };
	bool m_bRevolverCanShoot{ };
	bool m_bRevolverCanCock{ };
	bool m_bInCreatemove{ };
	bool m_bInPeek{ };
	bool m_bSafeFromDefensive{ };
	bool m_bUpdatingAnimations{ };
	bool m_bProhibitSounds{ };
	bool m_bUnload{ };
	bool m_bExtrapolating{ };
	//bool m_bSpawningCmds{ };
	//bool m_bReadPackets{ };
	//bool m_bRepredict{ };
	//bool m_bCollisionForced{ };

	int m_iLastPeekCmdNum{ };
	int m_iTicksAllowed{ };
	//int m_iLastSentCmdNumber{ };
	int m_iLastShotNumber{ };
	int m_iPenetrationDamage{ };
	int m_iBombCarrier{ };
	int m_iHighestTickbase{ };
	int m_iLast4Deltas[ 4 ]{ };

	int m_iRageRecordPerfTimer{ };
	//int m_iRageScanTargetsPerfTimer{ };
	int m_iAnimsysPerfTimer{ };

	uint8_t m_iLastID{ };

	float m_flLastAnimTimeUpdate{ };
	float m_iLastStopTime{ };
	float m_flOutLatency{ };
	int m_iRealOutLatencyTicks{ };
	float m_flInLatency{ };
	float m_flLerpTime{ };
	float m_flFixedCurtime{ };

	FORCEINLINE void GetLocal( ) {
		if ( !Interfaces::Engine->IsInGame( ) || !Interfaces::Engine->IsConnected( ) ) {
			m_pLocal = nullptr;
			return;
		}

		m_pLocal = static_cast< CBasePlayer * >( Interfaces::ClientEntityList->GetClientEntity( Interfaces::Engine->GetLocalPlayer( ) ) );

		//return *reinterpret_cast< CBasePlayer** >( Displacement::Sigs.LocalPlayer );
	};

	FORCEINLINE int CalcCorrectionTicks( ) {
		return Interfaces::Globals->nMaxClients == 1
			? -1 : TIME_TO_TICKS( std::clamp<float>( Displacement::Cvars.sv_clockcorrection_msecs->GetFloat( ) / 1000.f, 0.f, 1.f ) );
	}
};
inline HAVOCCTX ctx;