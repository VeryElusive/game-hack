#pragma once
#include "prop_manager.h"

struct signatures {
	uintptr_t LocalPlayer;
	uintptr_t CBaseEntity__PrecacheModel;

	uintptr_t uPredictionRandomSeed;
	uintptr_t pPredictionPlayer;

	uintptr_t ReInitPredictables;
	uintptr_t ShutDownPredictables;

	uintptr_t InitKeyValues;
	uintptr_t DestructKeyValues;
	uintptr_t oFromString;
	uintptr_t oLoadFromBuffer;
	uintptr_t oLoadFromFile;
	uintptr_t oFindKey;
	uintptr_t oSetString;
	uintptr_t oGetString;

	uintptr_t oCreateAnimationState;
	uintptr_t oUpdateAnimationState;
	uintptr_t oResetAnimationState;

	uintptr_t ClipRayToHitbox;
	uintptr_t CalcShotgunSpread;

	uintptr_t MD5PseudoRandom;

	uintptr_t FindMapping;
	uintptr_t SelectWeightedSequenceFromModifiers;

	uintptr_t uDisableRenderTargetAllocationForever;

	uintptr_t SetAbsOrigin;
	uintptr_t SetAbsAngles;
	uintptr_t InvalidatePhysicsRecursive;

	uintptr_t PostProcess;

	uintptr_t SmokeCount;
	uintptr_t TakeDamageOffset;
	uintptr_t LookupBone;

	uintptr_t SetAbsVelocity;

	uintptr_t LoadNamedSkys;
	uintptr_t CL_ReadPackets;
	uintptr_t ClanTag;

	uintptr_t ClearNotices;

	uintptr_t StartDrawing;
	uintptr_t FinishDrawing;

	uintptr_t ReturnToExtrapolate;
	uintptr_t ReturnToCl_ReadPackets;

	uintptr_t SetupVelocityReturn;

	uintptr_t uInsertIntoTree;

	uintptr_t uAllocKeyValuesEngine;
	uintptr_t uAllocKeyValuesClient;

	uintptr_t uCAM_ThinkReturn;
	uintptr_t DoResetLatchReturn;

	uintptr_t ReturnToEyePosAndVectors;

	uintptr_t InvalidateBoneCache;

	uintptr_t SetCollisionBounds;

	uintptr_t m_pStudioHdr;

	uintptr_t WriteUsercmd;

	uintptr_t AddBoxOverlayReturn;

	uintptr_t IsBreakable;

	uintptr_t CL_SendMove;

	uintptr_t GetSequenceActivity;

	uintptr_t SetupBones_AttachmentHelper;

	uintptr_t ReturnToProcessInputIsBoneAvailableForRead;
	uintptr_t ReturnToProcessInputGetAbsOrigin;

	uintptr_t ReturnToClampBonesInBBox;
	uintptr_t ClampBonesInBBox;
	uintptr_t C_BaseAnimating__BuildTransformations;
	uintptr_t StandardBlendingRules;

	uintptr_t CL_FireEvents;
	uintptr_t NET_ProcessSocket;

	uintptr_t TraceFilterSkipTwoEntities;
	uintptr_t numticks;

	uintptr_t ReturnToPerformPrediction;
	uintptr_t ReturnToInterpolateServerEntities;
	uintptr_t ReturnToInterpolateServerEntitiesExtrap;

	uintptr_t GetLayerIdealWeightFromSeqCycle;

	uintptr_t ReturnToDrawCrosshair;
	uintptr_t ReturnToWantReticleShown;

	uintptr_t current_tickcount;
	uintptr_t host_currentframetick;


	uintptr_t CIKContext__Construct;
	uintptr_t CIKContext__Init;
	uintptr_t CIKContext__UpdateTargets;
	uintptr_t CIKContext__SolveDependencies;
	uintptr_t CIKContext__AddDependencies;
	uintptr_t CIKContext__CopyTo;

	uintptr_t CBoneMergeCache__Init;
	uintptr_t CBoneMergeCache__Construct;
	uintptr_t CBoneMergeCache__MergeMatchingPoseParams;
	uintptr_t CBoneMergeCache__CopyFromFollow;
	uintptr_t CBoneMergeCache__CopyToFollow;

	uintptr_t CBoneSetup__AccumulatePose;
	uintptr_t CBoneSetup__CalcAutoplaySequences;
	uintptr_t CBoneSetup__CalcBoneAdj;
};

#include "../sdk/convar.h"

struct cvars {
	CConVar* mp_teammates_are_enemies;
	CConVar* cl_foot_contact_shadows;
	CConVar* weapon_recoil_scale;
	CConVar* view_recoil_tracking;
	CConVar* ff_damage_reduction_bullets;
	CConVar* ff_damage_bullet_penetration;
	CConVar* r_drawspecificstaticprop;
	CConVar* cl_updaterate;
	CConVar* r_modelAmbientMin;
	CConVar* r_jiggle_bones;
	CConVar* sv_maxunlag;
	CConVar* cl_interp;
	CConVar* cl_interp_ratio;
	CConVar* sv_clockcorrection_msecs;
	CConVar* sv_accelerate_use_weapon_speed;
	CConVar* sv_accelerate;
	CConVar* mat_ambient_light_r;
	CConVar* mat_ambient_light_g;
	CConVar* mat_ambient_light_b;
	CConVar* sv_gravity;
	CConVar* sv_stopspeed;
	CConVar* sv_maxvelocity;
	CConVar* weapon_molotov_maxdetonateslope;
	CConVar* molotov_throw_detonate_time;
	CConVar* sv_showimpacts;
	CConVar* cl_clock_correction;
	CConVar* r_drawmodelstatsoverlay;
	CConVar* cl_mouseenable;
	CConVar* weapon_debug_spread_show;
	CConVar* cl_csm_shadows;
	CConVar* sv_friction;
	CConVar* cl_ignorepackets;
	CConVar* sv_enablebunnyhopping;
	CConVar* sv_jump_impulse;
	CConVar* mp_damage_scale_ct_head;
	CConVar* mp_damage_scale_t_head;
	CConVar* mp_damage_scale_ct_body;
	CConVar* mp_damage_scale_t_body;
	CConVar* sv_client_min_interp_ratio;
	CConVar* sv_client_max_interp_ratio;
	CConVar* sv_minupdaterate;
	CConVar* sv_maxupdaterate;
	CConVar* r_3dsky;
	CConVar* sv_skyname;
	CConVar* weapon_accuracy_shotgun_spread_patterns;
	CConVar* sv_penetration_type;
	CConVar* sv_showimpacts_time;
	CConVar* cl_predict;


	CConVar* fog_override;
	CConVar* fog_start;
	CConVar* fog_end;
	CConVar* fog_maxdensity;
	CConVar* fog_hdrcolorscale;
	CConVar* fog_color;
};

struct netvars {
	int m_iClip1;
	int m_flNextPrimaryAttack;
	int m_flNextSecondaryAttack;
	int m_hWeaponWorldModel;
	int m_iItemDefinitionIndex;
	int m_hActiveWeapon;
	int m_hMyWeapons;
	int m_angEyeAngles;
	int m_vecMins;
	int m_vecMaxs;
	int m_vecOrigin;
	int m_iTeamNum;
	int m_CollisionGroup;
	int m_nRenderMode;
	int m_flSimulationTime;
	int m_flAnimTime;
	int m_lifeState;
	int m_iHealth;
	int m_fFlags;
	int m_aimPunchAngle;
	int m_viewPunchAngle;
	int pl;
	int m_hOwnerEntity;
	int m_hOwner;
	int m_nTickBase;
	int m_hViewEntity;
	int m_hViewModel;
	int m_vphysicsCollisionState;
	int m_aimPunchAngleVel;
	int m_vecVelocity;
	int m_flFlashDuration;
	int m_bIsScoped;
	int m_ArmorValue;
	int m_zoomLevel;
	int m_iBurstShotsRemaining;
	int m_fNextBurstShot;
	int m_fLastShotTime;
	int m_fAccuracyPenalty;
	int m_flRecoilIndex;
	int m_flPostponeFireReadyTime;
	int m_bIsBroken;
	int m_nSequence;
	int m_flPlaybackRate;
	int m_nHitboxSet;
	int m_bClientSideAnimation;
	int m_flCycle;
	int m_flEncodedController;
	int m_bHasHelmet;
	int m_bHasHeavyArmor;
	int m_vecViewOffset;
	int m_flVelocityModifier;
	int m_bWaitForNoAttack;
	int m_iPlayerState;
	int m_bIsDefusing;
	int m_flNextAttack;
	int m_iMoveState;
	int m_iAddonBits;
	int m_flPoseParameter;
	int m_flDuckAmount;
	int m_flLowerBodyYawTarget;
	int m_bGunGameImmunity;
	int m_flThirdpersonRecoil;
	int m_bIsWalking;
	int m_bIsPlayerGhost;
	int deadflag;
	int m_flDuckSpeed;
	int m_flMaxSpeed;
	int m_hGroundEntity;
	int m_vecVelocityGRENADE;
	int m_nSmokeEffectTickBegin;
	int m_nExplodeEffectTickBegin;
	int m_bUseCustomBloomScale;
	int m_bUseCustomAutoExposureMax;
	int m_bUseCustomAutoExposureMin;
	int m_flCustomAutoExposureMin;
	int m_flCustomAutoExposureMax;
	int m_flCustomBloomScale;
	int m_iPing;
	int m_iPlayerC4;
	int m_fThrowTime;
	int m_flThrowStrength;
	int m_bPinPulled;
	int m_fog_enable;
	int m_bStrafing;
	int m_hWeapon;
	int m_nAnimationParity;
	int m_nModelIndex;
	int m_vecBaseVelocity;
	int m_hCombatWeaponParent;
	int m_nPrecipType;
};

namespace Displacement {
	void Init( void* netvars );
	std::uintptr_t FindInDataMap( DataMap_t* pMap, const char* name );

	inline netvars* Netvars;
	inline signatures Sigs;
	inline cvars Cvars;
}