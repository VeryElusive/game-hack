#pragma once
// used: surfacedata
#include "../../sdk/interfaces/iphysicssurfaceprops.h"
// used: baseentity, cliententity, baseweapon, weapondata classes
#include "../../sdk/entity.h"
#include "../../context.h"
#include "../../utils/math.h"

struct PenetrationData {
	CBasePlayer* target{ };

	int penetrationCount{ 4 };
	float dmg{ -1.f };
	int hitgroup{ -1 };
	int hitbox { -1 };
};

class CAutowall {
public:
	PenetrationData FireBullet( CBasePlayer* const shooter, CBasePlayer* const target, const CCSWeaponData* const wpn_data, const bool is_taser, Vector src, const Vector& dst, bool penetrate );
	void ClipTraceToPlayer( Vector end, Vector start, CGameTrace* oldtrace, CBasePlayer* ent );
	void ScaleDamage( CBasePlayer* player, float& damage, float ArmourRatio, int hitgroup, float headshotMultiplier );
	bool HandleBulletPenetration( CBasePlayer* const shooter, const CCSWeaponData* const wpn_data, const CGameTrace& enter_trace, Vector& src, const Vector& dir, int& pen_count, float& cur_dmg, const float pen_modifier );
	bool TraceToExit( CBasePlayer* shooter, const Vector& src, const Vector& dir, const CGameTrace& enter_trace, CGameTrace& exit_trace );

	PenetrationData FireEmulated( CBasePlayer* const shooter, CBasePlayer* const target, Vector src, const Vector& dst );

	bool CanPenetrate( );
};

namespace Features { inline CAutowall Autowall; };