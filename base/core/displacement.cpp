#include "displacement.h"
#include "../core/interfaces.h"
#include "../utils/networking/networking.h"
#include <shlobj.h>

#define DEFCVAR( VAR ) Cvars.##VAR = Interfaces::ConVar->FindVar( _( #VAR ) );

#define DT_BaseEntity( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BaseEntity" ), _( #VAR ) );
#define DT_BasePlayer( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BasePlayer" ), _( #VAR ) );
#define DT_CSPlayer( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_CSPlayer" ), _( #VAR ) );
#define DT_BaseCombatCharacter( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BaseCombatCharacter" ), _( #VAR ) );
#define DT_BaseCombatWeapon( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BaseCombatWeapon" ), _( #VAR ) );
#define DT_BaseWeaponWorldModel( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BaseWeaponWorldModel" ), _( #VAR ) );
#define DT_WeaponCSBaseGun( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_WeaponCSBaseGun" ), _( #VAR ) );
#define DT_WeaponCSBase( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_WeaponCSBase" ), _( #VAR ) );
#define DT_BreakableSurface( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BreakableSurface" ), _( #VAR ) );
#define DT_BaseCSGrenadeProjectile( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BaseCSGrenadeProjectile" ), _( #VAR ) );
#define DT_SmokeGrenadeProjectile( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_SmokeGrenadeProjectile" ), _( #VAR ) );
#define DT_BaseAnimating( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BaseAnimating" ), _( #VAR ) );
#define DT_EnvTonemapController( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_EnvTonemapController" ), _( #VAR ) );
#define DT_BaseCSGrenade( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BaseCSGrenade" ), _( #VAR ) );
#define DT_BaseViewModel( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_BaseViewModel" ), _( #VAR ) );
#define DT_CSPlayerResource( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_CSPlayerResource" ), _( #VAR ) );
#define DT_Precipitation( VAR ) Netvars->##VAR = PropManager::Get( ).GetOffset( _( "DT_Precipitation" ), _( #VAR ) );

void Displacement::Init( void* netvarsPTR ) {
	/* offsets */
#ifdef _RELEASE
	Netvars = reinterpret_cast< netvars* >( netvarsPTR );
#else
	Netvars = new netvars;
	DT_BaseEntity( m_vecMins );
	DT_BaseEntity( m_vecMaxs );
	DT_BaseEntity( m_vecOrigin );
	DT_BaseEntity( m_iTeamNum );
	DT_BaseEntity( m_CollisionGroup );
	DT_BaseEntity( m_nRenderMode );
	DT_BaseEntity( m_flSimulationTime );
	DT_BaseEntity( m_flAnimTime );
	DT_BaseEntity( m_hOwnerEntity );

	Netvars->m_vecViewOffset = PropManager::Get( ).GetOffset( _( "DT_BasePlayer" ), _( "m_vecViewOffset[0]" ) );// one off cuz diff name
	Netvars->m_hViewModel = PropManager::Get( ).GetOffset( _( "DT_BasePlayer" ), _( "m_hViewModel[0]" ) );// one off cuz diff name
	DT_BasePlayer( m_lifeState );
	DT_BasePlayer( m_iHealth );
	DT_BasePlayer( m_fFlags );
	DT_BasePlayer( m_aimPunchAngle );
	DT_BasePlayer( m_viewPunchAngle );
	DT_BasePlayer( pl );
	DT_BasePlayer( m_nTickBase );
	DT_BasePlayer( m_hViewEntity );
	DT_BasePlayer( m_vphysicsCollisionState );
	DT_BasePlayer( m_aimPunchAngleVel );
	DT_BasePlayer( deadflag );
	DT_BasePlayer( m_flDuckSpeed );
	DT_BasePlayer( m_flMaxSpeed );
	DT_BasePlayer( m_hGroundEntity );
	DT_BasePlayer( m_bStrafing );
	DT_BasePlayer( m_nModelIndex );
	DT_BasePlayer( m_vecBaseVelocity );
	DT_BasePlayer( pl );
	Netvars->m_vecVelocity = PropManager::Get( ).GetOffset( _( "DT_BasePlayer" ), _( "m_vecVelocity[0]" ) );// one off cuz diff name

	Netvars->m_angEyeAngles = PropManager::Get( ).GetOffset( _( "DT_CSPlayer" ), _( "m_angEyeAngles[0]" ) );// one off cuz diff name
	DT_CSPlayer( m_flFlashDuration );
	DT_CSPlayer( m_bIsScoped );
	DT_CSPlayer( m_ArmorValue );
	DT_CSPlayer( m_bHasHelmet );
	DT_CSPlayer( m_bHasHeavyArmor );
	DT_CSPlayer( m_flVelocityModifier );
	DT_CSPlayer( m_bWaitForNoAttack );
	DT_CSPlayer( m_iPlayerState );
	DT_CSPlayer( m_bIsDefusing );
	DT_CSPlayer( m_flNextAttack );
	DT_CSPlayer( m_iMoveState );
	DT_CSPlayer( m_iAddonBits );
	DT_CSPlayer( m_flPoseParameter );
	DT_CSPlayer( m_flDuckAmount );
	DT_CSPlayer( m_flLowerBodyYawTarget );
	DT_CSPlayer( m_bGunGameImmunity );
	DT_CSPlayer( m_flThirdpersonRecoil );
	DT_CSPlayer( m_bIsWalking );
	DT_CSPlayer( m_bIsPlayerGhost );

	DT_BaseCombatCharacter( m_hActiveWeapon );
	DT_BaseCombatCharacter( m_hMyWeapons );

	DT_BaseCombatWeapon( m_iItemDefinitionIndex );
	DT_BaseCombatWeapon( m_iClip1 );
	DT_BaseCombatWeapon( m_flNextPrimaryAttack );
	DT_BaseCombatWeapon( m_flNextSecondaryAttack );
	DT_BaseCombatWeapon( m_hWeaponWorldModel );
	DT_BaseCombatWeapon( m_hOwner );

	DT_BaseWeaponWorldModel( m_hCombatWeaponParent );

	DT_WeaponCSBaseGun( m_zoomLevel );
	DT_WeaponCSBaseGun( m_iBurstShotsRemaining );
	DT_WeaponCSBaseGun( m_fNextBurstShot );
	DT_WeaponCSBaseGun( m_fLastShotTime );

	DT_WeaponCSBase( m_fAccuracyPenalty );
	DT_WeaponCSBase( m_flRecoilIndex );
	DT_WeaponCSBase( m_flPostponeFireReadyTime );

	DT_BreakableSurface( m_bIsBroken );

	DT_BaseAnimating( m_nSequence );
	DT_BaseAnimating( m_nHitboxSet );
	DT_BaseAnimating( m_bClientSideAnimation );
	DT_BaseAnimating( m_flCycle );
	DT_BaseAnimating( m_flPlaybackRate );
	DT_BaseAnimating( m_flEncodedController );

	DT_BaseCSGrenadeProjectile( m_nExplodeEffectTickBegin );

	DT_BaseViewModel( m_hWeapon );
	DT_BaseViewModel( m_nAnimationParity );

	DT_EnvTonemapController( m_bUseCustomBloomScale );
	DT_EnvTonemapController( m_bUseCustomAutoExposureMax );
	DT_EnvTonemapController( m_bUseCustomAutoExposureMin );
	DT_EnvTonemapController( m_flCustomAutoExposureMin );
	DT_EnvTonemapController( m_flCustomAutoExposureMax );
	DT_EnvTonemapController( m_flCustomBloomScale );

	DT_CSPlayerResource( m_iPing );
	DT_CSPlayerResource( m_iPlayerC4 );

	Netvars->m_vecVelocityGRENADE = PropManager::Get( ).GetOffset( _( "DT_BaseCSGrenadeProjectile" ), _( "m_vecVelocity" ) );// one off cuz diff name

	DT_BaseCSGrenade( m_fThrowTime );
	DT_BaseCSGrenade( m_flThrowStrength );
	DT_BaseCSGrenade( m_bPinPulled );

	DT_SmokeGrenadeProjectile( m_nSmokeEffectTickBegin );

	DT_Precipitation( m_nPrecipType );

	Netvars->m_fog_enable = PropManager::Get( ).GetOffset( _( "DT_FogController" ), _( "m_fog.enable" ) );

	{
		std::ofstream write{ "C:\\Users\\Admin\\Documents\\GitHub\\MINE\\csgo-recode\\build\\Release\\netvars.txt" };
		std::string sigsStruct;

		for ( int i = 0; i < sizeof( netvars ); i++ ) {
			const auto currentByte{ *reinterpret_cast< char* >( uintptr_t( Netvars ) + i ) };
			for ( int x = 0; x < 8; x++ )
				write << ( int ) ( ( currentByte >> x ) & 1 );
		}

		write.close( );
	}
#endif

	/* cvars */
	DEFCVAR( mp_teammates_are_enemies );
	DEFCVAR( cl_foot_contact_shadows );
	DEFCVAR( weapon_recoil_scale );
	DEFCVAR( view_recoil_tracking );
	DEFCVAR( ff_damage_reduction_bullets );
	DEFCVAR( ff_damage_bullet_penetration );
	DEFCVAR( r_drawspecificstaticprop );
	DEFCVAR( r_modelAmbientMin );
	DEFCVAR( r_jiggle_bones );
	DEFCVAR( sv_maxunlag );
	DEFCVAR( cl_interp );
	DEFCVAR( cl_interp_ratio );
	DEFCVAR( cl_updaterate );
	DEFCVAR( sv_clockcorrection_msecs );
	DEFCVAR( sv_accelerate_use_weapon_speed );
	DEFCVAR( sv_accelerate );
	DEFCVAR( mat_ambient_light_r );
	DEFCVAR( mat_ambient_light_g );
	DEFCVAR( mat_ambient_light_b );
	DEFCVAR( sv_gravity );
	DEFCVAR( weapon_molotov_maxdetonateslope );
	DEFCVAR( molotov_throw_detonate_time );
	DEFCVAR( sv_showimpacts );
	DEFCVAR( cl_clock_correction );
	DEFCVAR( r_drawmodelstatsoverlay );
	DEFCVAR( cl_mouseenable );
	DEFCVAR( weapon_debug_spread_show );
	DEFCVAR( cl_csm_shadows );
	DEFCVAR( sv_stopspeed );
	DEFCVAR( sv_maxvelocity );
	DEFCVAR( sv_friction );
	DEFCVAR( cl_ignorepackets );
	DEFCVAR( sv_enablebunnyhopping );
	DEFCVAR( sv_jump_impulse );
	DEFCVAR( mp_damage_scale_ct_head );
	DEFCVAR( mp_damage_scale_t_head );
	DEFCVAR( mp_damage_scale_ct_body );
	DEFCVAR( mp_damage_scale_t_body );
	DEFCVAR( sv_client_min_interp_ratio );
	DEFCVAR( sv_client_max_interp_ratio );
	DEFCVAR( sv_minupdaterate );
	DEFCVAR( sv_maxupdaterate );
	DEFCVAR( r_3dsky );
	DEFCVAR( sv_skyname );
	DEFCVAR( weapon_accuracy_shotgun_spread_patterns );
	DEFCVAR( sv_penetration_type );
	DEFCVAR( sv_showimpacts_time );
	DEFCVAR( cl_predict );


	DEFCVAR( fog_override );
	DEFCVAR( fog_start );
	DEFCVAR( fog_end );
	DEFCVAR( fog_maxdensity );
	DEFCVAR( fog_hdrcolorscale );
	DEFCVAR( fog_color );

	//Interfaces::ConVar->FindVar( _( "r_occlusion" ) )->SetValue( 0 );
	Interfaces::ConVar->FindVar( _( "rate" ) )->SetValue( 786432 );
	Interfaces::ConVar->FindVar( _( "cl_interp" ) )->SetValue( 0.015625f );
	Interfaces::ConVar->FindVar( _( "cl_interp_ratio" ) )->SetValue( 2 );
	//Interfaces::ConVar->FindVar( _( "cl_cmdrate" ) )->SetValue( 128 );
	//Interfaces::ConVar->FindVar( _( "cl_updaterate" ) )->SetValue( 128 );

	Interfaces::ConVar->FindVar( _( "r_jiggle_bones" ) )->SetValue( 0 );
	Interfaces::ConVar->FindVar( _( "engine_no_focus_sleep" ) )->SetValue( 0 );
	Interfaces::ConVar->FindVar( _( "cl_foot_contact_shadows" ) )->SetValue( 0 );
	Interfaces::ConVar->FindVar( _( "cl_csm_shadows" ) )->SetValue( 0 );
	Interfaces::ConVar->FindVar( _( "r_drawspecificstaticprop" ) )->SetValue( 0 );
	Interfaces::ConVar->FindVar( _( "developer" ) )->SetValue( 1 );
	Interfaces::ConVar->FindVar( _( "con_filter_enable" ) )->SetValue( 2 );
	Interfaces::ConVar->FindVar( _( "con_filter_text" ) )->SetValue( _( "[ HAVOC ]" ) );
	Interfaces::ConVar->FindVar( _( "cl_predict" ) )->nFlags &= ~FCVAR_NOT_CONNECTED;
	//Interfaces::ConVar->FindVar( _( "cl_pred_optimize" ) )->SetValue( 0 );
	//Interfaces::ConVar->FindVar( _( "cl_pred_doresetlatch" ) )->nFlags &= ~FCVAR_HIDDEN;
	//Interfaces::ConVar->FindVar( _( "cl_pred_doresetlatch" ) )->SetValue( 0 );
	//Interfaces::ConVar->FindVar( _( "cl_extrapolate" ) )->SetValue( 0 );
	//Interfaces::ConVar->FindVar( _( "cl_extrapolate_amount" ) )->SetValue( 0 );
	// well... i think m_bDisableSimulationFix will be set.. i havent checked tho. schizophrenia change tbh
	Interfaces::ConVar->FindVar( _( "cl_simulationtimefix" ) )->nFlags &= ~FCVAR_HIDDEN;
	Interfaces::ConVar->FindVar( _( "cl_simulationtimefix" ) )->SetValue( 0 );
	//Interfaces::ConVar->FindVar( _( "viewmodel_fov" ) )->fnChangeCallbacks.Size( ) = 0;
	//Interfaces::ConVar->FindVar( _( "viewmodel_offset_x" ) )->fnChangeCallbacks.Size( ) = 0;
	//Interfaces::ConVar->FindVar( _( "viewmodel_offset_y" ) )->fnChangeCallbacks.Size( ) = 0;
	//Interfaces::ConVar->FindVar( _( "viewmodel_offset_z" ) )->fnChangeCallbacks.Size( ) = 0;
	
	Sigs.CBaseEntity__PrecacheModel = MEM::FindPattern( SERVER_DLL, _( "56 8B F1 85 F6 74 4F" ) );// WARNING: THIS IS NOT UNIQUE SIG!

	Sigs.uPredictionRandomSeed = MEM::FindPattern( CLIENT_DLL, _( "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04" ) ) + 0x2;
	Sigs.pPredictionPlayer = MEM::FindPattern( CLIENT_DLL, _( "89 35 ? ? ? ? F3 0F 10 48 20" ) ) + 0x2;


	Sigs.ReInitPredictables = MEM::FindPattern( CLIENT_DLL, _( "A1 ? ? ? ? B9 ? ? ? ? 53 56 FF 50 ? 8B D8" ) );
	Sigs.ShutDownPredictables = MEM::FindPattern( CLIENT_DLL, _( "53 56 8B 35 ? ? ? ? 33 DB 57 33" ) );

	Sigs.InitKeyValues = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 56 8B F1 33 C0 8B 4D 0C 81" ) );// @xref: "OldParticleSystem_Destroy"
	Sigs.DestructKeyValues = MEM::FindPattern( CLIENT_DLL, _( "56 8B F1 E8 ? ? ? ? 8B 4E 14" ) );// @xref: "OldParticleSystem_Destroy"
	Sigs.oFromString = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 81 EC ? ? ? ? 85 D2 53" ) );// @xref: "#empty#", "#int#"
	Sigs.oLoadFromBuffer = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89" ) );// @xref: "KeyValues::LoadFromBuffer(%s%s%s): Begin"
	Sigs.oLoadFromFile = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 14 53 56 8B 75 08 57 FF" ) );// @xref: "rb"
	Sigs.oFindKey = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 1C 53 8B D9 85 DB" ) );// @xref: "rb"
	Sigs.oSetString = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC A1 ? ? ? ? 53 56 57 8B F9 8B 08 8B 01" ) );// @xref: "rb"
	Sigs.oGetString = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 8B 5D 08" ) );

	Sigs.oCreateAnimationState = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46" ) );// @xref: "ggprogressive_player_levelup"
	Sigs.oUpdateAnimationState = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24" ) );// @xref: "%s_aim"
	Sigs.oResetAnimationState = MEM::FindPattern( CLIENT_DLL, _( "56 6A 01 68 ? ? ? ? 8B F1" ) );// @xref: "player_spawn"


	Sigs.ReturnToClampBonesInBBox = MEM::FindPattern( CLIENT_DLL, _( "84 C0 0F 84 ? ? ? ? 8B 06 8B CE 8B 40 20" ) );

	Sigs.ReturnToDrawCrosshair = MEM::FindPattern( CLIENT_DLL, _( "83 F8 05 75 17 A1 ? ? ? ? B9 ? ? ? ? 8B 40 34" ) );
	Sigs.ReturnToWantReticleShown = MEM::FindPattern( CLIENT_DLL, _( "83 F8 05 75 0D 80 BF ? ? ? ? ? 75 5C 84 FF 74 58" ) );

	Sigs.ReturnToProcessInputIsBoneAvailableForRead = MEM::FindPattern( CLIENT_DLL, _( "84 C0 0F 84 ? ? ? ? 8B 44 24 14 8B 4C 24 28 8B 78 7C 8D 84 24" ) );
	Sigs.ReturnToProcessInputGetAbsOrigin = MEM::FindPattern( CLIENT_DLL, _( "F3 0F 10 44 24 ? F3 0F 10 54 24 ? F3 0F 10 4C 24 ? F3 0F 5C 40 ? 8B 7C 24 14 F3" ) );


	Sigs.ClipRayToHitbox = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 F3 0F 10 42" ) );

	Sigs.MD5PseudoRandom = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 70 6A 58 8D 44 24 1C 89 4C 24 08 6A 00 50" ) );


	Sigs.FindMapping = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 56 57 8B F9 8B 17" ) );
	Sigs.SelectWeightedSequenceFromModifiers = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 34 53 56 8B 75 08 8B D9 57 89" ) );

	// thx slazy
	const auto calc_shotgun_spread_rel = MEM::FindPattern( CLIENT_DLL, _( "E8 ? ? ? ? EB 38 83 EC 08" ) );
	Sigs.CalcShotgunSpread = calc_shotgun_spread_rel + 0x1u + sizeof( std::uintptr_t )
		+ *reinterpret_cast< std::ptrdiff_t* >( calc_shotgun_spread_rel + 0x1u );


	Sigs.uDisableRenderTargetAllocationForever = MEM::FindPattern( MATERIALSYSTEM_DLL, _( "80 B9 ? ? ? ? ? 74 0F" ) );// @xref: "Tried BeginRenderTargetAllocation after game startup. If I let you do this, all users would suffer.\n"

	Sigs.SetAbsOrigin = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8 ? ? ? ? 8B 7D 08 F3 0F 10 07 0F 2E 86" ) );
	Sigs.SetAbsAngles = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8" ) );
	Sigs.SetAbsVelocity = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3" ) );

	Sigs.PostProcess = MEM::FindPattern( CLIENT_DLL, _( "80 3D ? ? ? ? ? 53 56 57 0F 85" ) );

	Sigs.SmokeCount = MEM::FindPattern( CLIENT_DLL, _( "A3 ? ? ? ? 57 8B CB" ) );

	Sigs.TakeDamageOffset = MEM::FindPattern( CLIENT_DLL, _( "80 BE ? ? ? ? ? 75 46 8B 86" ) );

	Sigs.LookupBone = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 53 56 8B F1 57 83 BE ? ? ? ? ? 75 14" ) );


	Sigs.InvalidatePhysicsRecursive = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56 83 E0 04" ) );

	Sigs.ClearNotices = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 0C 53 56 8B 71 58" ) );

	Sigs.StartDrawing = MEM::FindPattern( VGUI_DLL, _( "55 8B EC 83 E4 C0 83 EC 38" ) );
	Sigs.FinishDrawing = MEM::FindPattern( VGUI_DLL, _( "8B 0D ? ? ? ? 56 C6 05" ) );

	Sigs.ReturnToInterpolateServerEntitiesExtrap = ( MEM::FindPattern( CLIENT_DLL, _( "0F B6 0D ? ? ? ? 84 C0 0F 44" ) ) );

	Sigs.SetupVelocityReturn = MEM::FindPattern( CLIENT_DLL, _( "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80" ) );
	//Sigs.AccumulateLayersReturn = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 57 8B F9 8B 0D ? ? ? ? 8B 01 8B 80" ) + 22 );

	Sigs.uInsertIntoTree = ( MEM::FindPattern( CLIENT_DLL, _( "56 52 FF 50 18" ) ) + 0x5 );// @xref: "<unknown renderable>"

	Sigs.uCAM_ThinkReturn = MEM::FindPattern( CLIENT_DLL, _( "85 C0 75 30 38 87" ) );
	Sigs.DoResetLatchReturn = MEM::FindPattern( CLIENT_DLL, _( "85 C0 75 28 8B 0D ? ? ? ? 81" ) );

	Sigs.ReturnToEyePosAndVectors = MEM::FindPattern( CLIENT_DLL, _( "8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5" ) );

	Sigs.InvalidateBoneCache = MEM::FindPattern( CLIENT_DLL, _( "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81" ) );

	Sigs.SetCollisionBounds = MEM::FindPattern( CLIENT_DLL, _( "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 18 56 57 8B 7B" ) );

	Sigs.m_pStudioHdr = ( MEM::FindPattern( CLIENT_DLL, _( "8B B7 ? ? ? ? 89 74 24 20" ) ) + 0x2 );

	Sigs.WriteUsercmd = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" ) );

	Sigs.AddBoxOverlayReturn = MEM::FindPattern( CLIENT_DLL, _( "3B 3D ? ? ? ? 75 4C" ) );

	Sigs.IsBreakable = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE" ) );


	Sigs.GetSequenceActivity = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 53 8B 5D ? 56 8B F1 83 FB" ) );// @xref: "Need to handle the activity %d\n"

	Sigs.SetupBones_AttachmentHelper = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 48 53 8B 5D" ) );
	Sigs.ClampBonesInBBox = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 38 83 BF" ) );
	Sigs.StandardBlendingRules = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6" ) );
	Sigs.C_BaseAnimating__BuildTransformations = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 89 7C 24 28 8B" ) );// C_CSPlayer: 55 8B EC 53 56 57 FF 75 1C

	Sigs.CL_FireEvents = MEM::FindPattern( ENGINE_DLL, _( "55 8B EC 83 EC 08 53 8B 1D ? ? ? ? 56 57 83 BB" ) );
	Sigs.NET_ProcessSocket = MEM::FindPattern( ENGINE_DLL, _( "55 8B EC 83 E4 F8 83 EC 4C 53 56 8B D9 89 54 24 10 57 89 5C 24 10 E8" ) );

	Sigs.TraceFilterSkipTwoEntities = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 81 EC BC 00 00 00 56 8B F1 8B 86" ) ) + 0x21E;

	Sigs.ReturnToPerformPrediction = MEM::FindPattern( CLIENT_DLL, _( "89 45 EC 85 C0 0F ? ? ? ? ? 80 78" ) );
	Sigs.ReturnToInterpolateServerEntities = MEM::FindPattern( CLIENT_DLL, _( "84 C0 74 07 C6 05 ? ? ? ? ? 8B" ) );

	Sigs.GetLayerIdealWeightFromSeqCycle = MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC ? 53 56 8B 35 ? ? ? ? 57 8B F9 8B CE 8B 06 FF 90 ? ? ? ? 8B 7F ? 0F 57 DB" ) );


	Sigs.CL_SendMove = MEM::FindPattern( ENGINE_DLL, _( "55 8B EC 8B 4D 04 81 EC ? ? ? ? 53 56 57 E8" ) );
	Sigs.ClanTag = MEM::FindPattern( ENGINE_DLL, _( "53 56 57 8B DA 8B F9 FF 15" ) );
	Sigs.CL_ReadPackets = MEM::FindPattern( ENGINE_DLL, _( "53 8A D9 8B 0D ? ? ? ? 56 57 8B B9" ) );
	Sigs.ReturnToCl_ReadPackets = MEM::FindPattern( ENGINE_DLL, _( "84 C0 75 57 ? ? ? ? ? ? 83" ) );
	Sigs.LoadNamedSkys = MEM::FindPattern( ENGINE_DLL, _( "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45" ) );
	Sigs.numticks = ( MEM::FindPattern( ENGINE_DLL, _( "03 05 ? ? ? ? 83 CF 10" ) ) + 2 );// @xref: "SV_StartSound: invalid sentence number: %s" ( dword_13903F54 )

	Sigs.current_tickcount = ( MEM::FindPattern( ENGINE_DLL, _( "48 39 05 ? ? ? ? 74 C6" ) ) + 3 );// in host_shouldrun
	Sigs.host_currentframetick = ( MEM::FindPattern( ENGINE_DLL, _( "FF 05 ? ? ? ? 8B 0D ? ? ? ? A3" ) ) + 2 );// in host_shouldrun



	// setupbones rebuild
	Sigs.CIKContext__Construct = ( MEM::FindPattern( CLIENT_DLL, _( "53 8B D9 F6 C3 03 74 0B FF 15 ? ? ? ? 84 C0 74 01 CC C7 83 ? ? ? ? ? ? ? ? 8B CB" ) ) );
	Sigs.CIKContext__Init = ( MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D 8F" ) ) );
	Sigs.CIKContext__UpdateTargets = { MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 33 D2 89 4C" ) ) };
	Sigs.CIKContext__SolveDependencies = { MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 8B 81" ) ) };
	Sigs.CIKContext__AddDependencies = { MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B F9 0F 28 CB F3 0F 11 4D" ) ) };
	Sigs.CIKContext__CopyTo = { MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 24 8B 45 08 57 8B F9 89 7D F4 85 C0" ) ) };

	Sigs.CBoneMergeCache__Init = { MEM::CallableFromRelative( MEM::FindPattern( CLIENT_DLL, _( "56 8B C8 89 86 ? ? ? ? E8" ) ) + 9 ) };// 10 mb?
	Sigs.CBoneMergeCache__Construct = { MEM::FindPattern( CLIENT_DLL, _( "56 8B F1 0F 57 C0 C7 86 ? ? ? ? ? ? ? ? C7 86" ) ) };
	Sigs.CBoneMergeCache__MergeMatchingPoseParams = { MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 0C 53 56 8B F1 57 89 75 F8 E8" ) ) };
	Sigs.CBoneMergeCache__CopyFromFollow = { MEM::CallableFromRelative( MEM::FindPattern( CLIENT_DLL, _( "E8 ? ? ? ? F3 0F 10 45 ? 8D 84 24" ) ) ) };
	Sigs.CBoneMergeCache__CopyToFollow = { MEM::CallableFromRelative( MEM::FindPattern( CLIENT_DLL, _( "E8 ? ? ? ? 8B 87 ? ? ? ? 8D 8C 24 ? ? ? ? 8B 7C 24" ) ) ) };

	Sigs.CBoneSetup__AccumulatePose = { MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? A1" ) ) };
	Sigs.CBoneSetup__CalcAutoplaySequences = { MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 EC 10 53 56 57 8B 7D 10" ) ) };
	Sigs.CBoneSetup__CalcBoneAdj = { MEM::FindPattern( CLIENT_DLL, _( "55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B C1 89" ) ) };
}

std::uintptr_t Displacement::FindInDataMap( DataMap_t* pMap, const char* name ) {
	while ( pMap ) {
		for ( int i = 0; i < pMap->nDataFields; i++ ) {
			if ( pMap->pDataDesc[ i ].szFieldName == nullptr )
				continue;

			if ( strcmp( name, pMap->pDataDesc[ i ].szFieldName ) == 0 )
				return pMap->pDataDesc[ i ].iFieldOffset;

			if ( pMap->pDataDesc[ i ].iFieldType == FIELD_EMBEDDED ) {
				if ( pMap->pDataDesc[ i ].pTypeDescription ) {
					unsigned int offset;

					if ( ( offset = FindInDataMap( pMap->pDataDesc[ i ].pTypeDescription, name ) ) != 0 )
						return offset;
				}
			}
		}
		pMap = pMap->pBaseMap;
	}

	return 0;
}