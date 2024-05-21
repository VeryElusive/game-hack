#include "autowall.h"

PenetrationData CAutowall::FireBullet( CBasePlayer* const shooter, CBasePlayer* const target, 
	const CCSWeaponData* const weaponData, const bool isTaser, Vector src, const Vector& dst, bool penetrate ) {
	const auto pen_modifier = std::max( ( 3.f / weaponData->flPenetration ) * 1.25f, 0.f );

	PenetrationData data{ };
	float maxDistance{ weaponData->flRange };
	float curDistance{ };

	data.dmg = static_cast< float >( weaponData->iDamage );

	CGameTrace trace{ };

	CTraceFilterSkipTwoEntities traceFilter{ shooter };
	CBasePlayer* lastHitEntity{ };
	//CTraceFilter traceFilter{ shooter };

	const auto dir{ ( dst - src ).Normalized( ) };

	while ( data.dmg > 0.f ) {
		const auto remainingDist{ maxDistance - curDistance };

		const auto end{ src + dir * remainingDist };

		traceFilter.pSkip2 = lastHitEntity;

		Interfaces::EngineTrace->TraceRay(
			{ src, end }, MASK_SHOT_PLAYER,
			&traceFilter, &trace
		);

		if ( target && traceFilter.ShouldHitEntity( target, MASK_SHOT_PLAYER ) )
			ClipTraceToPlayer( end + ( dir * 40.f ), src, &trace, target );

		if ( trace.flFraction == 1.f )
			break;

		curDistance += remainingDist * trace.flFraction;
		data.dmg *= std::pow( weaponData->flRangeModifier, curDistance * 0.002f );

		const auto hitPlayer{ static_cast< CBasePlayer* >( trace.pHitEntity ) };
		if ( hitPlayer
			&& ( hitPlayer == target || ( !target && hitPlayer->IsPlayer( ) && !hitPlayer->IsTeammate( shooter ) ) ) ) {
			if ( ( trace.iHitGroup >= HITGROUP_HEAD && trace.iHitGroup <= HITGROUP_NECK )
				|| trace.iHitGroup == HITGROUP_GEAR ) {
				data.target = hitPlayer;
				data.hitbox = trace.iHitbox;
				data.hitgroup = trace.iHitGroup;

				if ( isTaser )
					data.hitgroup = 0;

				// TODO: CalculateBulletDamageForce
				ScaleDamage( hitPlayer, data.dmg, weaponData->flArmorRatio, data.hitgroup, weaponData->flHeadShotMultiplier );

				return data;
			}
		}

		const auto enterSurfaceData{ Interfaces::PhysicsProps->GetSurfaceData( trace.surface.nSurfaceProps ) };
		if ( !enterSurfaceData
			|| enterSurfaceData->game.flPenetrationModifier < 0.1f )
			break;

		if ( curDistance > 3000.f && weaponData->flPenetration > 0.f )
			break;

		lastHitEntity = trace.pHitEntity && trace.pHitEntity->IsPlayer( ) ? static_cast< CBasePlayer* >( trace.pHitEntity ) : nullptr;

		// ghetto window fix
		if ( !penetrate && !( trace.iContents & CONTENTS_WINDOW ) )
			break;

		if ( isTaser
			|| HandleBulletPenetration( shooter, weaponData, trace, src, dir, data.penetrationCount, data.dmg, pen_modifier ) )
			break;
	}

	return { };
}

void CAutowall::ScaleDamage( CBasePlayer* player, float& damage, float ArmourRatio, int hitgroup, float headshotMultiplier ) {
	const auto CT{ player->m_iTeamNum( ) == TEAM_CT },
		heavyArmor{ player->m_bHasHeavyArmor( ) };

	auto headDmgScale{ CT ? Displacement::Cvars.mp_damage_scale_ct_head->GetFloat( ) : Displacement::Cvars.mp_damage_scale_t_head->GetFloat( ) };
	if ( heavyArmor ) 
		headDmgScale *= 0.5f;

	const auto bodyDmgScale{ CT ? Displacement::Cvars.mp_damage_scale_ct_body->GetFloat( ) : Displacement::Cvars.mp_damage_scale_t_body->GetFloat( ) };

	switch ( hitgroup ) {
	case HITGROUP_HEAD:
		damage *= headDmgScale * headshotMultiplier;
		break;
	case HITGROUP_CHEST:
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
	case HITGROUP_NECK:
		damage *= bodyDmgScale;
		break;
	case HITGROUP_STOMACH:
		damage *= bodyDmgScale * 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		damage *= bodyDmgScale * 0.75f;
		break;
	default:
		break;
	}

	auto armored = [ heavyArmor ]( CBasePlayer* pl, int hitgroup ) -> bool {
		switch ( hitgroup ) {
		case HITGROUP_HEAD:
			return pl->m_bHasHelmet( ) || heavyArmor;
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
		case HITGROUP_NECK:
			return true;
		default:
			return heavyArmor;
		}
	};

	int armorVal{ player->m_ArmorValue( ) };

	if ( armorVal > 0 && armored( player, hitgroup ) ) {
		auto armorBonus{ 0.5f },
			armorRatio{ 0.5f * ArmourRatio },
			heavyArmorBonus{ 1.f };


		if ( heavyArmor ) {
			damage *= 0.85f;
			armorRatio *= 0.5f;
			armorBonus = heavyArmorBonus = 0.33f;
		}

		auto dmg2health{ damage * armorRatio };

		if ( ( damage - dmg2health ) * armorBonus * heavyArmorBonus > armorVal )
			dmg2health = damage - armorVal / armorBonus;

		damage = dmg2health;
	}
}

void CAutowall::ClipTraceToPlayer( Vector dst, Vector src, CGameTrace* oldtrace, CBasePlayer* ent ) {
	const auto pos{ ent->GetAbsOrigin( ) + ( ent->m_vecMins( ) + ent->m_vecMaxs( ) ) * 0.5f };
	const auto to{ pos - src };

	auto dir{ src - dst };
	const auto len{ dir.NormalizeInPlace( ) };
	const auto range_along{ dir.DotProduct( to ) };

	const auto range{ 
		range_along < 0.f ? -( to ).Length( )
		: range_along > len ? -( pos - dst ).Length( )
		: ( pos - ( src + dir * range_along ) ).Length( ) };

	if ( range > 60.f )
		return;

	CGameTrace new_trace{ };

	Ray_t Ray{ src, dst };

	Interfaces::EngineTrace->ClipRayToEntity( Ray, MASK_SHOT_PLAYER, ent, &new_trace );

	if ( new_trace.flFraction > oldtrace->flFraction )
		return;

	*oldtrace = new_trace;
}

bool CAutowall::TraceToExit( CBasePlayer* shooter, const Vector& start, const Vector& dir, const CGameTrace& trEnter, CGameTrace& trExit ) {
	float flDistance = 0;
	Vector last = start;
	int nStartContents = 0;
	CTraceFilterSkipTwoEntities filter{ shooter };

	while ( flDistance <= 90.f )
	{
		flDistance += 4.f;

		const auto end = start + ( dir * flDistance );

		Vector vecTrEnd = end - ( dir * 4.f );

		const auto nCurrentContents{ Interfaces::EngineTrace->GetPointContents( end, MASK_SHOT_PLAYER ) };

		if ( !nStartContents )
			nStartContents = nCurrentContents;

		if ( nCurrentContents & MASK_SHOT_HULL
			&& ( !( nCurrentContents & CONTENTS_HITBOX ) || nCurrentContents == nStartContents ) )
			continue;

		if ( ( nCurrentContents & CS_MASK_SHOOT ) == 0 || ( ( nCurrentContents & CONTENTS_HITBOX ) && nStartContents != nCurrentContents ) ) {
			// this gets a bit more complicated and expensive when we have to deal with displacements
			Interfaces::EngineTrace->TraceRay( { end, vecTrEnd }, MASK_SHOT_PLAYER, reinterpret_cast< ITraceFilter* >( &filter ), &trExit );

			// we exited the wall into a player's hitbox
			if ( trExit.bStartSolid && ( trExit.surface.uFlags & SURF_HITBOX )/*( nStartContents & CONTENTS_HITBOX ) == 0 && (nCurrentContents & CONTENTS_HITBOX)*/ ) {
				// do another trace, but skip the player to get the actual exit surface 
				filter.pSkip2 = trExit.pHitEntity;
				Interfaces::EngineTrace->TraceRay( { end, start }, MASK_SHOT_PLAYER, reinterpret_cast< ITraceFilter* >( &filter ), &trExit );

				if ( trExit.DidHit( ) && !trExit.bStartSolid )
					return true;
			}
			else if ( trExit.DidHit( ) && !trExit.bStartSolid ) {
				bool bStartIsNodraw = !!( trEnter.surface.uFlags & ( SURF_NODRAW ) );
				bool bExitIsNodraw = !!( trExit.surface.uFlags & ( SURF_NODRAW ) );
				if ( bExitIsNodraw && trExit.pHitEntity->IsBreakable( ) && trEnter.pHitEntity->IsBreakable( ) )
					// we have a case where we have a breakable object, but the mapper put a nodraw on the backside
					return true;

				else if ( bExitIsNodraw == false || ( bStartIsNodraw && bExitIsNodraw ) ) // exit nodraw is only valid if our entrace is also nodraw
					return true;
			}
			//trEnter.DidHitNonWorldEntity( )
			else if ( trEnter.pHitEntity && trEnter.pHitEntity->Index( ) && trEnter.pHitEntity->IsBreakable( ) ) {
				// if we hit a breakable, make the assumption that we broke it if we can't find an exit (hopefully..)
				// fake the end pos
				trExit = trEnter;
				trExit.vecEnd = start + ( dir * 1.0f );
				return true;
			}
		}
	}

	return false;

}

bool CAutowall::HandleBulletPenetration( CBasePlayer* const shooter, const CCSWeaponData* const weaponData, 
	const CGameTrace& enterTrace, Vector& src, const Vector& dir, int& penCount, float& curDmg, 
	const float pen_modifier ) {

	if ( penCount <= 0
		|| weaponData->flPenetration <= 0.f )
		return true;

	CGameTrace exitTrace{ };

	if ( !TraceToExit( shooter, enterTrace.vecEnd, dir, enterTrace, exitTrace )
		&& !( Interfaces::EngineTrace->GetPointContents( enterTrace.vecEnd, MASK_SHOT_HULL ) & MASK_SHOT_HULL ) )
		return true;

	const auto hitGrate{ enterTrace.iContents & CONTENTS_GRATE };
	const auto IsNodraw{ enterTrace.surface.uFlags & SURF_NODRAW };


	const auto exitSurfaceData{ Interfaces::PhysicsProps->GetSurfaceData( exitTrace.surface.nSurfaceProps ) };
	const auto enterSurfaceData{ Interfaces::PhysicsProps->GetSurfaceData( enterTrace.surface.nSurfaceProps ) };

	auto penetrationModifier{ enterSurfaceData->game.flPenetrationModifier };
	auto damageModifier{ enterSurfaceData->game.flDamageModifier };

	const auto enterMaterial{ enterSurfaceData->game.hMaterial };
	const auto exitMaterial{ exitSurfaceData->game.hMaterial };

	if ( Displacement::Cvars.sv_penetration_type->GetInt( ) != 1 ) {
		float damageModifier{ };

		if ( hitGrate || IsNodraw ) {
			penetrationModifier = 1.f;
			damageModifier = 0.99f;
		}
		else {
			if ( exitSurfaceData->game.flPenetrationModifier < penetrationModifier )
				penetrationModifier = exitSurfaceData->game.flPenetrationModifier;

			if ( exitSurfaceData->game.flDamageModifier < damageModifier )
				damageModifier = exitSurfaceData->game.flDamageModifier;
		}

		if ( enterMaterial == exitMaterial && ( exitMaterial == 87 || exitMaterial == 77 ) )
			penetrationModifier *= 2.f;

		if ( ( exitTrace.vecEnd - enterTrace.vecEnd ).Length( ) > weaponData->flPenetration * penetrationModifier )
			return true;

		curDmg *= damageModifier;
	}
	else {
		auto dmgLostPercent{ 0.16f };

		if ( hitGrate || IsNodraw 
			|| enterMaterial == CHAR_TEX_GLASS || enterMaterial == CHAR_TEX_GRATE ) {
			if ( enterMaterial == CHAR_TEX_GLASS || enterMaterial == CHAR_TEX_GRATE ) {
				penetrationModifier = 3.f;
				dmgLostPercent = 0.05f;
			}
			else
				penetrationModifier = 1.f;
		}
		else if ( enterMaterial == CHAR_TEX_FLESH && !Displacement::Cvars.ff_damage_reduction_bullets->GetFloat( ) 
			&& enterTrace.pHitEntity && enterTrace.pHitEntity->IsPlayer( ) 
			&& !static_cast< CBasePlayer* >( enterTrace.pHitEntity )->IsTeammate( shooter ) ) {
			const auto ff_damage_reduction_bullets{ Displacement::Cvars.ff_damage_reduction_bullets->GetFloat( ) };
			if ( !ff_damage_reduction_bullets )
				return true;

			penetrationModifier = ff_damage_reduction_bullets;
		}
		else
			penetrationModifier = ( penetrationModifier + exitSurfaceData->game.flPenetrationModifier ) * 0.5f;

		if ( enterMaterial == exitMaterial ) {
			if ( exitMaterial == CHAR_TEX_WOOD || exitMaterial == CHAR_TEX_CARDBOARD )
				penetrationModifier = 3.f;
			else if ( exitMaterial == CHAR_TEX_PLASTIC )
				penetrationModifier = 2.f;
		}

		const auto traceDist{ ( exitTrace.vecEnd - enterTrace.vecEnd ).Length( ) };
		const auto penetrationModifier_frac{ std::max( 0.f, 1.f / penetrationModifier ) };

		const auto penWepMod{ curDmg * dmgLostPercent + std::max( 0.f, ( 3.f / weaponData->flPenetration ) * 1.25f ) * penetrationModifier_frac * 3.f };

		const auto totaLostDmg{ penWepMod + traceDist * traceDist * penetrationModifier_frac / 24.f };

		curDmg -= std::max( 0.f, totaLostDmg );

		if ( curDmg < 1.f )
			return true;
	}

	--penCount;

	src = exitTrace.vecEnd;

	return false;
}

bool CAutowall::CanPenetrate( ) {
	if ( ctx.m_pLocal->IsDead( ) )
		return false;

	if ( !ctx.m_pWeapon || ctx.m_pWeapon->IsKnife( ) || 
		ctx.m_pWeapon->IsGrenade( ) || !ctx.m_pWeaponData )
		return false;

	Vector direction;
	Math::AngleVectors( ctx.m_angOriginalViewangles, &direction );

	CGameTrace trace;
	CTraceFilter filter{ ctx.m_pLocal };

	Interfaces::EngineTrace->TraceRay(
		{ ctx.m_vecEyePos, ctx.m_vecEyePos + direction * ctx.m_pWeaponData->flRange }, MASK_SHOT_HULL,
		reinterpret_cast< ITraceFilter* >( &filter ), &trace
	);

	if ( trace.flFraction == 1.0f )
		return false;

	int penCount{ 1 };
	float dmg = static_cast< float >( ctx.m_pWeaponData->iDamage );
	const auto penetrationModifierifier = std::max( ( 3.f / ctx.m_pWeaponData->flPenetration ) * 1.25f, 0.f );

	const bool e = !HandleBulletPenetration( ctx.m_pLocal, ctx.m_pWeaponData, trace, ctx.m_vecEyePos, direction, penCount, dmg, penetrationModifierifier );
	ctx.m_iPenetrationDamage = static_cast<int>( dmg ) == ctx.m_pWeaponData->iDamage ? 0 : dmg;

	return e;
}

PenetrationData CAutowall::FireEmulated( CBasePlayer* const shooter, CBasePlayer* const target, Vector src, const Vector& dst ) {
	static const auto wpn_data = [ ]( ) {
		CCSWeaponData wpn_data{ };

		wpn_data.iDamage = 115;
		wpn_data.flRange = 8192.f;
		wpn_data.flPenetration = 2.5f;
		wpn_data.flRangeModifier = 0.99f;
		wpn_data.flArmorRatio = 1.95f;

		return wpn_data;
	}( );

	const auto pen_modifier = std::max( ( 3.f / wpn_data.flPenetration ) * 1.25f, 0.f );

	float cur_dist{ };

	PenetrationData data{ };

	data.penetrationCount = 4;

	auto cur_dmg = static_cast< float >( wpn_data.iDamage );

	auto dir = dst - src;

	const auto max_dist = dir.NormalizeInPlace( );

	CGameTrace trace{ };
	CTraceFilterSkipTwoEntities traceFilter{ shooter };

	while ( cur_dmg > 0.f ) {
		const auto dist_remaining = wpn_data.flRange - cur_dist;

		const auto cur_dst = src + dir * dist_remaining;

		traceFilter.pSkip2 = trace.pHitEntity && trace.pHitEntity->IsPlayer( ) ? trace.pHitEntity : nullptr;

		Interfaces::EngineTrace->TraceRay(
			{ src, cur_dst }, MASK_SHOT_PLAYER,
			reinterpret_cast< ITraceFilter* >( &traceFilter ), &trace
		);

		Vector ExEnd{ cur_dst + dir * 40.f };

		if ( target )
			ClipTraceToPlayer( ExEnd, src, &trace, target );

		if ( trace.flFraction == 1.f
			|| ( trace.vecEnd - src ).Length( ) > max_dist )
			break;

		cur_dist += trace.flFraction * dist_remaining;
		cur_dmg *= std::pow( wpn_data.flRangeModifier, cur_dist / 500.f );

		if ( cur_dist > 3000.f
			&& wpn_data.flPenetration > 0.f )
			break;

		if ( trace.pHitEntity ) {
			const auto is_player = trace.pHitEntity->IsPlayer( );
			if ( trace.pHitEntity == target ) {
				data.target = static_cast< CBasePlayer* >( trace.pHitEntity );
				data.hitbox = trace.iHitbox;
				data.hitgroup = trace.iHitGroup;
				data.dmg = static_cast< int >( cur_dmg );

				return data;
			}
		}

		const auto enter_surface = Interfaces::PhysicsProps->GetSurfaceData( trace.surface.nSurfaceProps );
		if ( enter_surface->game.flPenetrationModifier < 0.1f
			|| HandleBulletPenetration( shooter, &wpn_data, trace, src, dir, data.penetrationCount, cur_dmg, pen_modifier ) )
			break;
	}

	return data;
}