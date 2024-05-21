#pragma once
#include "../animations/animation.h"
#include "../misc/engine_prediction.h"
#include "../misc/logger.h"
#include "../misc/shots.h"
#include "../../sdk/entity.h"
#include "../../context.h"
#include "autowall.h"

#define CBB_MATRIX_COUNT ( 1 * 17 )
#define DESYNC_MATRIX_COUNT 3
#define MATRIX_COUNT CBB_MATRIX_COUNT + DESYNC_MATRIX_COUNT

#define SETRAGEBOOL( name ) switch ( type ) { \
case WEAPONTYPE_PISTOL: if ( idx == WEAPON_DEAGLE || idx == WEAPON_REVOLVER ) MenuVars.name = Config::Get<bool>( Vars.name##HeavyPistol ); else MenuVars.name = Config::Get<bool>( Vars.name##Pistol );break; \
case WEAPONTYPE_SUBMACHINEGUN: MenuVars.name = Config::Get<bool>( Vars.name##SMG );break; \
case WEAPONTYPE_RIFLE: MenuVars.name = Config::Get<bool>( Vars.name##Rifle );break; \
case WEAPONTYPE_SHOTGUN: MenuVars.name = Config::Get<bool>( Vars.name##Shotgun );break; \
case WEAPONTYPE_SNIPER: if ( idx == WEAPON_AWP ) MenuVars.name = Config::Get<bool>( Vars.name##AWP );else if ( idx == WEAPON_SSG08 ) MenuVars.name = Config::Get<bool>( Vars.name##Scout ); else MenuVars.name = Config::Get<bool>( Vars.name##Auto );break; \
case WEAPONTYPE_MACHINEGUN: MenuVars.name = Config::Get<bool>( Vars.name##Machine );break; }\

#define SETRAGEINT( name ) switch ( type ) { \
case WEAPONTYPE_PISTOL: if ( idx == WEAPON_DEAGLE || idx == WEAPON_REVOLVER ) MenuVars.name = Config::Get<int>( Vars.name##HeavyPistol ); else MenuVars.name = Config::Get<int>( Vars.name##Pistol );break; \
case WEAPONTYPE_SUBMACHINEGUN: MenuVars.name = Config::Get<int>( Vars.name##SMG );break; \
case WEAPONTYPE_RIFLE: MenuVars.name = Config::Get<int>( Vars.name##Rifle );break; \
case WEAPONTYPE_SHOTGUN: MenuVars.name = Config::Get<int>( Vars.name##Shotgun );break; \
case WEAPONTYPE_SNIPER: if ( idx == WEAPON_AWP ) MenuVars.name = Config::Get<int>( Vars.name##AWP );else if ( idx == WEAPON_SSG08 ) MenuVars.name = Config::Get<int>( Vars.name##Scout ); else MenuVars.name = Config::Get<int>( Vars.name##Auto );break; \
case WEAPONTYPE_MACHINEGUN: MenuVars.name = Config::Get<int>( Vars.name##Machine );break; }\

struct PreviousExtrapolationData_t {
	PreviousExtrapolationData_t( float duck, int flags ) : m_flDuckAmount( duck ), m_iFlags( flags ) {};

	float m_flDuckAmount{ };

	int m_iFlags{ };
};

struct ExtrapolationBackup_t {
	ExtrapolationBackup_t( CBasePlayer* player )
		: m_iFlags( player->m_fFlags( ) ), m_iMoveState( player->m_iMoveState( ) ), m_flDuckAmount( player->m_flDuckAmount( ) ), m_flViewOffsetZ( player->m_vecViewOffset( ).z ),
		m_flNewBoundsTime( player->m_flNewBoundsTime( ) ), m_flNewBoundsMaxs( player->m_flNewBoundsMaxs( ) ), m_vecVelocity( player->m_vecVelocity( ) ),
		m_vecBaseVelocity( player->m_vecBaseVelocity( ) ), m_vecMins( player->m_vecMins( ) ), m_vecMaxs( player->m_vecMaxs( ) ), m_hGroundEntity( player->m_hGroundEntity( ) ),
		m_cAnimstate( *player->m_pAnimState( ) ),
		m_iEFlags( player->m_iEFlags( ) ),
		m_angEyeAngles( player->m_angEyeAngles( ) ), m_vecOrigin( player->m_vecOrigin( ) ), m_vecAbsOrigin( player->GetAbsOrigin( ) )
	{
		std::memcpy( m_pLayers, player->m_AnimationLayers( ), sizeof( CAnimationLayer ) * 13 );
	}

	void restore( CBasePlayer* player ) {
		player->m_fFlags( ) = m_iFlags;
		player->m_iMoveState( ) = m_iMoveState;
		player->m_flDuckAmount( ) = m_flDuckAmount;
		player->m_vecVelocity( ) = m_vecVelocity;
		player->m_vecBaseVelocity( ) = m_vecBaseVelocity;
		player->m_angEyeAngles( ) = m_angEyeAngles;
		player->m_vecOrigin( ) = m_vecOrigin;
		player->m_iEFlags( ) = m_iEFlags;
		player->m_hGroundEntity( ) = m_hGroundEntity;
		player->m_vecViewOffset( ).z = m_flViewOffsetZ;

		*player->m_pAnimState( ) = m_cAnimstate;

		std::memcpy( player->m_AnimationLayers( ), m_pLayers, sizeof( CAnimationLayer ) * 13 );

		player->SetAbsOrigin( m_vecAbsOrigin );
		player->m_flNewBoundsTime( ) = m_flNewBoundsTime;
		player->m_flNewBoundsMaxs( ) = m_flNewBoundsMaxs;
		player->SetCollisionBounds( m_vecMins, m_vecMaxs );
	}

	int m_iFlags{ };
	int m_iMoveState{ };
	int m_iEFlags{ };

	float m_flDuckAmount{ };
	float m_flNewBoundsTime{ };
	float m_flNewBoundsMaxs{ };
	float m_flViewOffsetZ{ };

	Vector m_vecVelocity{ };
	Vector m_vecBaseVelocity{ };
	Vector m_vecOrigin{ };
	Vector m_vecAbsOrigin{ };

	QAngle m_angEyeAngles{ };

	CBaseHandle m_hGroundEntity{ };

	Vector m_vecMins{ };
	Vector m_vecMaxs{ };

	CCSGOPlayerAnimState m_cAnimstate{ };
	CAnimationLayer m_pLayers[ 13 ];
	// TODO: do i restore layers?
};

struct AimPoint_t {
	AimPoint_t( ) {}
	AimPoint_t( Vector point, int ind ) : m_vecPoint{ point }, m_iHitgroup{ ind } {}

	Vector m_vecPoint{ };
	int m_iHitgroup{ };
	uint8_t m_iDesyncIntersections{ };
	uint8_t m_iCBBIntersections{ };
	int m_flDamage{ };

	bool m_bValid{ };
};

struct AimTarget_t {
	/*AimTarget_t( ) {
		this->m_pMatrices = new matrix3x4_t * [ MATRIX_COUNT ];

		for ( int i = 0; i < MATRIX_COUNT; ++i )
			this->m_pMatrices[ i ] = new matrix3x4_t[ 256 ];
	}

	~AimTarget_t( ) {
		for ( int i = 0; i < MATRIX_COUNT; ++i )
			delete[ ] this->m_pMatrices[ i ];

		delete[ ] this->m_pMatrices;
	}*/

	float m_flCBBMaxz{ };
	float m_flCBBTime{ };
	float m_flViewOffsetZ{ };
	CBaseHandle m_hGroundEntity{ };

	CBasePlayer* m_pPlayer{ };

	unsigned int m_iBestDamage{ };
	unsigned int m_iResolverSide{ };

	bool m_bAbuseRecord{ };
	bool m_bExtrapolating{ };
	float m_flYaw{ };

	AimPoint_t m_cPoint{ };

	std::shared_ptr< std::string > m_pDbgLog{ };
	std::shared_ptr< LagRecord_t > m_pRecord{ };

	matrix3x4_t m_pMatrices[ MATRIX_COUNT ][ 256 ];

	void Extrapolate( PlayerEntry& entry, bool newRecord );
	LagRecord_t* GetAbuseRecord( PlayerEntry& entry );
	void GetBestLagRecord( PlayerEntry& entry );
	void CompareTarget( AimTarget_t& target );
	void SetupMatrices( std::shared_ptr< LagRecord_t > record, float yaw = 0.f );
	int QuickScan( std::shared_ptr< LagRecord_t > record, std::vector<int>& hitgroups );
	void ScanTarget( std::vector<EHitboxIndex>& hitboxes );
	void GetBestPoint( bool forceBaim, std::vector<EHitboxIndex>& hitboxes );
	void Multipoint( EHitboxIndex index, Vector center, mstudiobbox_t* hitbox, float scale, std::vector<EHitboxIndex>& hitboxes );
	void ScanPoint( AimPoint_t& point, std::vector<EHitboxIndex>& hitboxes );
	int SafePoint( Vector aimpoint, int index, bool cbb );
	int HitChance( const QAngle& ang, int hitchance );
	void Fire( CUserCmd& cmd );
};

struct KnifeTarget_t {
	KnifeTarget_t( CBasePlayer* player ) : m_pPlayer( player ) {}
	CBasePlayer* m_pPlayer{ };
	bool m_bIsBackstab{ };
	bool m_bCanHitSecondary{ };
	bool m_bIsDamageable{ };
	std::shared_ptr< LagRecord_t > m_pRecord{ };
};

class CRageBot {
public:
	void Main( CUserCmd& cmd, bool shoot );

	bool m_bShouldStop{ };
	std::vector<std::pair<float, float>> m_vecPrecomputedSeeds{ };

	float ExtrapolateYawFromRecords( std::vector< std::shared_ptr<LagRecord_t>>& records, int extrapolationAmount );
	float ExtrapolateYaw( std::vector<PreviousYaw_t >& pattern, int extrapolationAmount );
	bool ExtrapolatePlayer( PlayerEntry& entry, float yaw, int resolverSide, int amount, Vector previousVelocity );
	bool ExtrapolatePlayer( CBasePlayer* player, float baseTime, int amount, QAngle angles, Vector previousVelocity, bool local );
	void AdjustDuckingVars( PlayerEntry& entry, int extrapolationAmount );

	Vector2D CalcSpreadAngle( const int itemIndex, const float recoilIndex, float one, float twoPI );
private:
	std::vector< EHitboxIndex > m_vecHitboxes{ };
	void SimulatePlayer( CBasePlayer* player, float time, QAngle angles, int resolverSide, bool last, PreviousExtrapolationData_t& previous, float direction, float assumedSpeed, bool local );

	void KnifeBot( CUserCmd& cmd );
	void KnifeBotTargetPlayer( CUserCmd& cmd, KnifeTarget_t& target, LagRecord_t* record );
	bool KnifeBotCanHitTarget( CUserCmd& cmd, KnifeTarget_t& target, Vector dst, bool backStab );


	void PrecomputeSeeds( ) {
		for ( int i{ 1 }; i <= 256; ++i ) {
			Math::RandomSeed( i );
			m_vecPrecomputedSeeds.emplace_back( Math::RandomFloat( 0.f, 1.f ), Math::RandomFloat( 0.f, 2.f * M_PI ) );
		}
	}

	// this makes me want to quit programming im so embarrassed
	void SetupHitboxes( ) {
		if ( m_vecHitboxes.size( ) )
			m_vecHitboxes.clear( );

		if ( m_iLastWeaponIndex == WEAPON_TASER ) {
			m_vecHitboxes.push_back( HITBOX_CHEST );
			return;
		}

		if ( MenuVars.RagebotHBArms ) {
			m_vecHitboxes.push_back( HITBOX_RIGHT_UPPER_ARM );
			m_vecHitboxes.push_back( HITBOX_LEFT_UPPER_ARM );
		}

		if ( MenuVars.RagebotHBLegs ) {
			m_vecHitboxes.push_back( HITBOX_RIGHT_THIGH );
			m_vecHitboxes.push_back( HITBOX_LEFT_THIGH );

			m_vecHitboxes.push_back( HITBOX_RIGHT_CALF );
			m_vecHitboxes.push_back( HITBOX_LEFT_CALF );
		}

		if ( MenuVars.RagebotHBFeet ) {
			m_vecHitboxes.push_back( HITBOX_RIGHT_FOOT );
			m_vecHitboxes.push_back( HITBOX_LEFT_FOOT );
		}

		if ( MenuVars.RagebotHBUpperChest )
			m_vecHitboxes.push_back( HITBOX_UPPER_CHEST );

		if ( MenuVars.RagebotHBChest )
			m_vecHitboxes.push_back( HITBOX_CHEST );

		if ( MenuVars.RagebotHBLowerChest )
			m_vecHitboxes.push_back( HITBOX_LOWER_CHEST );

		if ( MenuVars.RagebotHBStomach )
			m_vecHitboxes.push_back( HITBOX_STOMACH );

		if ( MenuVars.RagebotHBPelvis )
			m_vecHitboxes.push_back( HITBOX_PELVIS );

		if ( MenuVars.RagebotHBHead )
			m_vecHitboxes.push_back( HITBOX_HEAD );
	}

	FORCEINLINE void ParseCfgItems( int type ) {
		const int idx{ ctx.m_pWeapon->m_iItemDefinitionIndex( ) };
		if ( idx == m_iLastWeaponIndex
			&& !Menu::m_bOpened )
			return;

		m_iLastWeaponType = type;
		m_iLastWeaponIndex = idx;

		if ( idx == WEAPON_TASER ) {
			MenuVars.RagebotMinimumDamage = 100;
			MenuVars.RagebotPenetrationDamage = 100;
			MenuVars.RagebotOverrideDamage = 100;
			MenuVars.RagebotHitchance = 80;
			MenuVars.RagebotNoscopeHitchance = 80;
			MenuVars.RagebotHitchanceThorough = false;
			SetupHitboxes( );
			return;
		}

		SETRAGEBOOL( RagebotHBHead );
		SETRAGEBOOL( RagebotHBUpperChest );
		SETRAGEBOOL( RagebotHBChest );
		SETRAGEBOOL( RagebotHBLowerChest );
		SETRAGEBOOL( RagebotHBStomach );
		SETRAGEBOOL( RagebotHBPelvis );
		SETRAGEBOOL( RagebotHBArms );
		SETRAGEBOOL( RagebotHBLegs );
		SETRAGEBOOL( RagebotHBFeet );

		SETRAGEBOOL( RagebotMPHead );
		SETRAGEBOOL( RagebotMPUpperChest );
		SETRAGEBOOL( RagebotMPChest );
		SETRAGEBOOL( RagebotMPLowerChest );
		SETRAGEBOOL( RagebotMPStomach );
		SETRAGEBOOL( RagebotMPPelvis );
		SETRAGEBOOL( RagebotMPArms );
		SETRAGEBOOL( RagebotMPLegs );
		SETRAGEBOOL( RagebotMPFeet );

		SETRAGEBOOL( RagebotAutoScope );
		SETRAGEBOOL( RagebotAutoFire );
		SETRAGEBOOL( RagebotSilentAim );
		SETRAGEBOOL( RagebotHitchanceThorough );
		SETRAGEBOOL( RagebotAutowall );
		SETRAGEBOOL( RagebotScaleDamage );
		SETRAGEBOOL( RagebotAutoStop );
		SETRAGEBOOL( RagebotBetweenShots );
		SETRAGEBOOL( RagebotAutostopInAir );
		SETRAGEBOOL( RagebotStaticPointscale );
		SETRAGEBOOL( RagebotIgnoreLimbs );
		SETRAGEBOOL( RagebotPreferBaim );
		SETRAGEBOOL( RagebotPreferBaimDoubletap );
		SETRAGEBOOL( RagebotPreferBaimLethal );
		SETRAGEBOOL( RagebotForceBaimAfterX );

		SETRAGEINT( RagebotFOV );
		SETRAGEINT( RagebotHitchance );
		SETRAGEINT( RagebotNoscopeHitchance );
		SETRAGEINT( RagebotMinimumDamage );
		SETRAGEINT( RagebotPenetrationDamage );
		SETRAGEINT( RagebotOverrideDamage );
		SETRAGEINT( RagebotHeadScale );
		SETRAGEINT( RagebotBodyScale );
		SETRAGEINT( RagebotForceBaimAfterXINT );

		SetupHitboxes( );
	}

	struct MenuVars_t {
		bool RagebotHBHead{ };
		bool RagebotHBUpperChest{ };
		bool RagebotHBChest{ };
		bool RagebotHBLowerChest{ };
		bool RagebotHBStomach{ };
		bool RagebotHBPelvis{ };
		bool RagebotHBArms{ };
		bool RagebotHBLegs{ };
		bool RagebotHBFeet{ };

		bool RagebotMPHead{ };
		bool RagebotMPUpperChest{ };
		bool RagebotMPChest{ };
		bool RagebotMPLowerChest{ };
		bool RagebotMPStomach{ };
		bool RagebotMPPelvis{ };
		bool RagebotMPArms{ };
		bool RagebotMPLegs{ };
		bool RagebotMPFeet{ };

		bool RagebotAutoFire{ };
		bool RagebotAutoScope{ };
		bool RagebotSilentAim{ };
		bool RagebotHitchanceThorough{ };
		bool RagebotScaleDamage{ };
		bool RagebotStaticPointscale{ };
		bool RagebotIgnoreLimbs{ };
		bool RagebotPreferBaim{ };
		bool RagebotPreferBaimDoubletap{ };
		bool RagebotPreferBaimLethal{ };
		bool RagebotForceBaimAfterX{ };

		int RagebotFOV{ };
		int RagebotHitchance{ };
		int RagebotNoscopeHitchance{ };
		int RagebotMinimumDamage{ };
		int RagebotPenetrationDamage{ };
		int RagebotOverrideDamage{ };
		int RagebotHeadScale{ };
		int RagebotBodyScale{ };
		int RagebotForceBaimAfterXINT{ };

		bool RagebotAutoStop{ };
		bool RagebotBetweenShots{ };
		bool RagebotAutostopInAir{ };
		bool RagebotAutowall{ };
	};

	int m_iLastWeaponIndex{ -1 };
	int m_iLastWeaponType{ -1 };
public:
	MenuVars_t MenuVars;

	public:
		FORCEINLINE std::string HitgroupToString( int hitgroup ) {
			switch ( hitgroup )
			{
			case HITGROUP_HEAD:
				return _( "head" );
			case HITGROUP_CHEST:
				return _( "chest" );
			case HITGROUP_STOMACH:
				return _( "stomach" );
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				return _( "arm" );
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
				return _( "leg" );
			case HITGROUP_GEAR:
				return _( "gear" );
			case HITGROUP_NECK:
				return _( "neck" );
			case HITGROUP_GENERIC:
				return _( "generic" );
			}
		}
		FORCEINLINE int HitboxToHitgroup( int hitgroup ) {
			switch ( hitgroup )
			{
			case HITBOX_HEAD:
				return HITGROUP_HEAD;
			case HITBOX_NECK:
				return HITGROUP_NECK;
			case HITBOX_PELVIS:
			case HITBOX_STOMACH:
				return HITGROUP_STOMACH;
			case HITBOX_LOWER_CHEST:
			case HITBOX_CHEST:
			case HITBOX_UPPER_CHEST:
				return HITGROUP_CHEST;
			case HITBOX_RIGHT_THIGH:
			case HITBOX_RIGHT_FOOT:
			case HITBOX_RIGHT_CALF:
				return HITGROUP_RIGHTLEG;
			case HITBOX_LEFT_CALF:
			case HITBOX_LEFT_THIGH:
			case HITBOX_LEFT_FOOT:
				return HITGROUP_LEFTLEG;
			case HITBOX_RIGHT_HAND:
			case HITBOX_RIGHT_UPPER_ARM:
			case HITBOX_RIGHT_FOREARM:
				return HITGROUP_RIGHTARM;
			case HITBOX_LEFT_UPPER_ARM:
			case HITBOX_LEFT_FOREARM:
			case HITBOX_LEFT_HAND:
				return HITGROUP_LEFTARM;
			}

			return 0;
		}

		bool IsMultiPointEnabled( int hitbox ) {
			if ( m_iLastWeaponIndex == WEAPON_TASER )
				return false;

			switch ( hitbox )
			{
			case HITBOX_HEAD:
			case HITBOX_NECK:
				return MenuVars.RagebotMPHead;
			case HITBOX_UPPER_CHEST:
				return MenuVars.RagebotMPUpperChest;
			case HITBOX_CHEST:
				return MenuVars.RagebotMPChest;
			case HITBOX_LOWER_CHEST:
				return MenuVars.RagebotMPLowerChest;
			case HITBOX_STOMACH:
				return MenuVars.RagebotMPStomach;
			case HITBOX_PELVIS:
				return MenuVars.RagebotMPPelvis;
			case HITBOX_RIGHT_UPPER_ARM:
			case HITBOX_LEFT_UPPER_ARM:
				return MenuVars.RagebotMPArms;
			case HITBOX_LEFT_THIGH:
			case HITBOX_RIGHT_THIGH:
			case HITBOX_RIGHT_CALF:
			case HITBOX_LEFT_CALF:
				return MenuVars.RagebotMPLegs;
			case HITBOX_RIGHT_FOOT:
			case HITBOX_LEFT_FOOT:
				return MenuVars.RagebotMPFeet;
			default:
				return false;
			}
		}

};

namespace Features { inline CRageBot Ragebot; };