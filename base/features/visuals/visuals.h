#pragma once
#include "../../core/interfaces.h"
#include "../../core/config.h"
#include "../../core/menu rework/menu.h"
#include "../../utils/math.h"
#include "../../utils/render.h"
#include "../misc/misc.h"
#include "../../context.h"
#include <unordered_map>

struct rect {
	bool is_zero( ) {
		return x == 0 && y == 0 && w == 0 && h == 0;
	}

	int x;
	int y;
	int w;
	int h;
};

enum player_type {
	LOCAL,
	TEAM,
	ENEMY
};

struct VisualPlayerEntry {
	int type{ };
	int health{ 100 };
	float Alpha{ 1 };
	float DormancyFade{ 0 };
	CBasePlayer* ent{ };
	rect BBox{ };

	int m_iNadeDamage{ };
};

struct hitmarker_t {
	hitmarker_t( Vector pos, int dmg, float curTime )
	{
		position = pos;
		damage = dmg;
		time = curTime;
		alpha = 1.f;
		step = 0.f;
		screen = Vector2D( 0, 0 );
	}
	Vector position;
	Vector2D screen;
	int damage = INT_MIN;
	float time = FLT_MIN;
	float alpha;
	float step;
};

struct BindInfo_t {
	BindInfo_t( const char* n, int mode ) {
		this->m_szName = n;

		switch ( mode ) {
		case EKeyMode::AlwaysOn:
			this->m_szMode = _( "[Always]" );
			break;
		case EKeyMode::Hold:
			this->m_szMode = _( "[Held]" );
			break;
		case EKeyMode::Toggle:
			this->m_szMode = _( "[Toggled]" );
			break;
		case EKeyMode::Off:
			this->m_szMode =  _( "[Off key]" );
			break;
		}

		this->m_iTextLength = Render::GetTextSize( std::string( this->m_szName ) + std::string( this->m_szMode ), Fonts::Menu ).x + 20;
	}

	int m_iTextLength = 0;
	const char* m_szName;
	const char* m_szMode;
};

struct C_HitMatrixEntry {
	CBasePlayer* m_pPlayer{ };
	int ent_index;
	ModelRenderInfo_t info;
	DrawModelState_t state;
	matrix3x4_t pBoneToWorld[ 256 ] = { };
	float time;
	matrix3x4_t model_to_world;
};

class CPlayerESP {
public:
	bool GetBBox( CBasePlayer* ent, rect& box );
	void Main( CBasePlayer* ent );

	std::array< VisualPlayerEntry, 64> Entries;

	void DrawBox( VisualPlayerEntry& entry );
	void DrawHealth( VisualPlayerEntry& entry );
	void DrawName( VisualPlayerEntry& entry );
	void DrawWeapon( VisualPlayerEntry& entry, bool AmmoBar );
	void DrawSkeleton( VisualPlayerEntry& entry );
	void DrawFlags( VisualPlayerEntry& entry );
	void DrawOOF( VisualPlayerEntry& entry );
	bool DrawAmmo( VisualPlayerEntry& entry );

	const Color DormantCol = Color( 200, 200, 200 );
};

class CChams {
private:
	bool init = false;
	IMaterial* FlatMat = nullptr;
	IMaterial* RegularMat = nullptr;
	IMaterial* GlowMat = nullptr;
	IMaterial* MetallicMat = nullptr;
	IMaterial* GalaxyMat = nullptr;

	std::vector<C_HitMatrixEntry> m_Hitmatrix;

	IMaterial* CreateMaterial( const std::string_view name, const std::string_view shader, const std::string_view data ) const;
	void OverrideMaterial( const int type, const bool ignore_z, Color col, const int glow, const bool wireframe ) const;
public:
	void InitMaterials( );
	void Main( DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags );
	void OnSceneEnd( );
	void RenderShotChams( );
	void AddHitmatrix( CBasePlayer* player, matrix3x4_t* bones );

	bool m_bInSceneEnd{ };

};

class CExtendedESP {
public:
	void Start( );
	void GetActiveSounds( );
	bool ValidSound( SoundInfo_t& sound );
	void SetupAdjustPlayer( CBasePlayer* player, SoundInfo_t& sound );
	void AdjustPlayer( CBasePlayer* player );

	struct SoundPlayer {
		void Override( SoundInfo_t& sound ) {
			m_iIndex = sound.nSoundSource;
			m_vecOrigin = *sound.vecOrigin;
			m_iReceiveTime = Interfaces::Globals->flRealTime;
			valid = true;
		}

		int m_iIndex = 0;
		int m_iReceiveTime = 0;
		Vector m_vecOrigin = Vector( 0, 0, 0 );
		int m_nFlags = 0;

		bool valid{ };
	} m_cSoundPlayers[ 64 ];

	CUtlVector<SoundInfo_t> m_utlvecSoundBuffer;
	CUtlVector<SoundInfo_t> utlCurSoundList;
	//std::vector<SoundPlayer> m_arRestorePlayers;
};

struct GrenadeSimulation_t {
	GrenadeSimulation_t( ) { };
	std::vector<Vector> vecPath;
	std::vector<Vector> vecBounces;
	std::vector< CBaseEntity* > vecIgnoredEntities;

	float m_flAlpha{ };
	bool m_bFrag{ };
	int m_iEntity{ };
};

class CGrenadePrediction {
public:
	void View( );
	void PaintLocal( );
	void Paint( );
	void SimulateThrownGrenade( CBasePlayer* player, CWeaponCSBase* weapon );

	std::unordered_map< CBaseHandle, GrenadeSimulation_t > m_vecGrenadeSimulations{ };
private:
	void Setup( Vector& vecSrc, Vector& vecThrow, const QAngle& angEyeAngles, CBasePlayer* player );
	void Simulate( QAngle& Angles, CBasePlayer* player, CWeaponCSBase* weapon );
	int Step( Vector& vecSrc, Vector& vecThrow, int& nextThinkTick, int tick, float interval, CWeaponCSBase* weapon, GrenadeSimulation_t& grenadeSim, bool thrown );
	void ResolveFlyCollisionCustom( CGameTrace& tr, Vector& vecVelocity, float interval, GrenadeSimulation_t& grenadeSim );
	void PushEntity( Vector& src, const Vector& move, CGameTrace& tr, GrenadeSimulation_t& grenadeSim );
	int PhysicsClipVelocity( const Vector& in, const Vector& normal, Vector& out, float overbounce );
	void TraceHull( Vector& src, Vector& end, CGameTrace& tr, GrenadeSimulation_t& grenadeSim );

	float flThrowVelocity = 0.f;
	float flThrowStrength = 0.f;

	GrenadeSimulation_t m_cLocalSimulation{ };
};

struct TracerData_t {
	TracerData_t( Vector start, Vector end, CBasePlayer* shooter ) 
		: m_vecStart( start ), m_vecEnd( end ), m_pShooter( shooter ), m_flStartTime( Interfaces::Globals->flRealTime ) { };

	float m_flStartTime{ };
	Vector m_vecStart{ };
	Vector m_vecEnd{ };
	CBasePlayer* m_pShooter{ };
};

inline bool PrecacheParticle( const char* szName ) {
	INetworkStringTable* g_pStringTableParticleEffectNames = Interfaces::NetworkContainer->FindTable( _( "ParticleEffectNames" ) );
	Interfaces::NetworkContainer->SetAllowClientSideAddString( g_pStringTableParticleEffectNames, true );


	if ( g_pStringTableParticleEffectNames ) {
		int idx = g_pStringTableParticleEffectNames->AddString( false, szName );
		if ( idx == INVALID_STRING_INDEX )
			return false;

		//g_pParticleSystemMgr->PrecacheParticleSystem( nIndex, pParticleSystemName );
	}
	return true;
}

inline bool PrecacheModel( const char* szModelName ) {
	INetworkStringTable* m_pModelPrecacheTable = Interfaces::NetworkContainer->FindTable( _( "modelprecache" ) );

	if ( m_pModelPrecacheTable ) {
		Interfaces::ModelInfo->FindOrLoadModel( szModelName );
		int idx = m_pModelPrecacheTable->AddString( false, szModelName );
		if ( idx == INVALID_STRING_INDEX )
			return false;
	}
	return true;
}

class CBulletTracers {
public:
	void Draw( );
	void AddTracer( Vector start, Vector end, CBasePlayer* shooter ) {
		const auto& col{ shooter == ctx.m_pLocal ? Config::Get<Color>( Vars.VisLocalBulletTracersCol ) : Config::Get<Color>( Vars.VisOtherBulletTracersCol ) };
		if ( !Config::Get<int>( Vars.VisBulletTracersType ) )
			Interfaces::DebugOverlay->AddLineOverlay( start, end, col.Get<COLOR_R>( ), col.Get<COLOR_G>( ), col.Get<COLOR_B>( ), true, 5.f );
		else {
			m_vecTracers.erase(
				std::remove_if(
					m_vecTracers.begin( ), m_vecTracers.end( ),
					[ & ]( const TracerData_t& tracer ) -> bool {
						return tracer.m_flStartTime == Interfaces::Globals->flRealTime
							&& shooter == tracer.m_pShooter;
					}
				),
				m_vecTracers.end( )
						);

			m_vecTracers.emplace_back( start, end, shooter );
		}
	};
private:
	std::vector<TracerData_t> m_vecTracers{ };
};

enum EPrecipitationType : int {
	PRECIPITATION_TYPE_RAIN = 0,
	PRECIPITATION_TYPE_SNOW,
	PRECIPITATION_TYPE_ASH,
	PRECIPITATION_TYPE_SNOWFALL,
	PRECIPITATION_TYPE_PARTICLERAIN,
	PRECIPITATION_TYPE_PARTICLEASH,
	PRECIPITATION_TYPE_PARTICLERAINSTORM,
	PRECIPITATION_TYPE_PARTICLESNOW,
	NUM_PRECIPITATION_TYPES
};

struct PrecipitationEntity_t {
	CBaseEntity* m_pEntity;
	IClientNetworkable* m_pNetworkable;
	Vector m_vecMins;
	Vector m_vecMaxs;
	vcollide_t m_cCollide;
	bool m_bCollideInit = false;
	int m_iModelIndex = 0;
	int m_iAbsModelIndex = 0;
	EPrecipitationType m_iType;
};

class CWeather {
public:
	std::vector< PrecipitationEntity_t > m_vecPrecipitationList;
	std::list< int > m_iModelIndexes;

	void CreatePrecipEntity( PrecipitationEntity_t* ent_info, EPrecipitationType precip_mode );
	void UnloadEntity( PrecipitationEntity_t& precip );
	void RemoveUnusedEntities( EPrecipitationType type );
	bool IsTypeActive( EPrecipitationType type );
	void AppleWeatherEffect( EPrecipitationType type );
	void CleanUp( );
	void Main( );
};

class CVisuals {
public:
	void Main( );
	void EntModulate( CBaseEntity* ent );
	void Watermark( );
	void SkyboxChanger( );
	void KeybindsList( );
	void ModelChanger( );

	FORCEINLINE void AddHit( hitmarker_t hit ) { hits.push_back( std::make_shared< hitmarker_t>( hit ) ); }

	CExtendedESP DormantESP;
	CChams Chams;
	CPlayerESP PlayerESP;
	CGrenadePrediction GrenadePrediction;
	CBulletTracers BulletTracers;
	CWeather Weather;

	std::vector<std::shared_ptr< hitmarker_t >> hits;

	Vector2D m_vec2KeyBindPos{ };
private:
	float m_flAutoPeekSize{ };

	Vector2D m_vec2KeyBindAbsSize{ };

	bool m_bKeybindDragging{ };

	void AutoPeekIndicator( );
	void OtherEntities( CBaseEntity* ent );
	void DrawGrenade( CBaseEntity* ent, int maxAlpha );
	void DrawWrappingRing( CBaseEntity* entity, float seconds, const char* name, float spawntime, float radius, int maxAlpha );
	void DrawHitMarker( const std::shared_ptr<hitmarker_t>& hit );
	void ManageHitmarkers( );

	FloatColor walls = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
	FloatColor props = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
	FloatColor skybox = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
};

#define CheckPlayerBoolFig( type, name )switch ( type ) { \
case 0: if ( !Config::Get<bool>( Vars.##name##Local ) ) return;break; \
case 1: if ( !Config::Get<bool>( Vars.##name##Team ) ) return;break; \
case 2: if ( !Config::Get<bool>( Vars.##name##Enemy ) ) return;break; }\

#define GetPlayerColorFig( type, name, var )switch ( type ) { \
case 0: var = Config::Get<Color>( Vars.##name##Local );break; \
case 1: var = Config::Get<Color>( Vars.##name##Team );break; \
case 2: var = Config::Get<Color>( Vars.##name##Enemy );break; }\

#define GetPlayerIntFig( type, name, var )switch ( type ) { \
case 0: var = Config::Get<int>( Vars.##name##Local );break; \
case 1: var = Config::Get<int>( Vars.##name##Team );break; \
case 2: var = Config::Get<int>( Vars.##name##Enemy );break; }\

#define GetPlayerBoolFig( type, name, var )switch ( type ) { \
case 0: var = Config::Get<bool>( Vars.##name##Local );break; \
case 1: var = Config::Get<bool>( Vars.##name##Team );break; \
case 2: var = Config::Get<bool>( Vars.##name##Enemy );break; }\

#define CheckIfPlayer( name, type ) 	if ( ( type == LOCAL && Config::Get<bool>( Vars.##name##Local ) ) \
|| ( type == TEAM && Config::Get<bool>( Vars.##name##Team ) ) \
|| ( type == ENEMY && Config::Get<bool>( Vars.##name##Enemy ) ) ) \

namespace Features { inline CVisuals Visuals; };