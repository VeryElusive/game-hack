#include "../core/hooks.h"

void FASTCALL Hooks::hkClipRayToCollideable( void* ecx, void* edx, const Ray_t& ray, uint32_t fMask, ICollideable* pCollide, CGameTrace* pTrace ) {
	static auto oClipRayToCollideable = DTR::ClipRayToCollideable.GetOriginal<decltype( &hkClipRayToCollideable )>( );

	if ( !pCollide )
		return oClipRayToCollideable( ecx, edx, ray, fMask, pCollide, pTrace );

	// extend the tracking
	const auto oldMax{ pCollide->OBBMaxs( ).z };

	pCollide->OBBMaxs( ).z += 5; // if the player is holding a knife and ducking in air we can still trace to this nerd and hit him

	oClipRayToCollideable( ecx, edx, ray, fMask, pCollide, pTrace );

	pCollide->OBBMaxs( ).z = oldMax;
}