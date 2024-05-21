#include "../core/hooks.h"
#include "../core/variables.h"
#include <intrin.h>

/*void FASTCALL Hooks::hkAddBoxOverlay( void* ecx, void* edx, const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration ) {
	static auto oAddBoxOverlay = DTR::AddBoxOverlay.GetOriginal<decltype( &hkAddBoxOverlay )>( );

	if ( !Config::Get<bool>( Vars.VisClientBulletImpacts ) || uintptr_t( _ReturnAddress( ) ) != Displacement::Sigs.AddBoxOverlayReturn )
		return oAddBoxOverlay( ecx, edx, origin, mins, max, orientation, r, g, b, a, duration );

	const auto& col{ Config::Get<Color>( Vars.VisClientBulletImpactsCol ) };

	return oAddBoxOverlay( ecx, edx, origin, mins, max, orientation,
		col.Get<COLOR_R>( ) * 255, col.Get<COLOR_G>( ) * 255, col.Get<COLOR_B>( ) * 255, col.Get<COLOR_A>( ) * 255,
		duration );
}*/