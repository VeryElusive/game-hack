#include "math.h"

// used: modules definitons, convar, globals interfaces
#include "../core/interfaces.h"

bool Math::Setup()
{
	const void* hVstdLib = MEM::GetModuleBaseHandle(VSTDLIB_DLL);

	if (hVstdLib == nullptr)
		return false;

	RandomSeed = reinterpret_cast<RandomSeedFn>(MEM::GetExportAddress(hVstdLib, _("RandomSeed")));
	if (RandomSeed == nullptr)
		return false;

	RandomFloat = reinterpret_cast<RandomFloatFn>(MEM::GetExportAddress(hVstdLib, _("RandomFloat")));
	if (RandomFloat == nullptr)
		return false;

	RandomFloatExp = reinterpret_cast<RandomFloatExpFn>(MEM::GetExportAddress(hVstdLib, _("RandomFloatExp")));
	if (RandomFloatExp == nullptr)
		return false;

	RandomInt = reinterpret_cast<RandomIntFn>(MEM::GetExportAddress(hVstdLib, _("RandomInt")));
	if (RandomInt == nullptr)
		return false;

	RandomGaussianFloat = reinterpret_cast<RandomGaussianFloatFn>(MEM::GetExportAddress(hVstdLib, _("RandomGaussianFloat")));
	if (RandomGaussianFloat == nullptr)
		return false;

	return true;
}
bool Math::Stub( ) {
	const void* hVstdLib = MEM::GetModuleBaseHandle( VSTDLIB_DLL );

	if ( hVstdLib == nullptr )
		return false;

	RandomInt = reinterpret_cast< RandomIntFn >( MEM::GetExportAddress( hVstdLib, _( "RandomInt" ) ) );
	if ( RandomInt == nullptr )
		return false;

	RandomGaussianFloat = reinterpret_cast< RandomGaussianFloatFn >( MEM::GetExportAddress( hVstdLib, _( "RandomGaussianFloat" ) ) );
	if ( RandomGaussianFloat == nullptr )
		return false;

	return false;
}

bool Math::ScreenTransform( const Vector& point, Vector& screen )
{
	float w;
	const ViewMatrix_t& worldToScreen = Interfaces::Engine->WorldToScreenMatrix( );

	screen.x = worldToScreen[ 0 ][ 0 ] * point[ 0 ] + worldToScreen[ 0 ][ 1 ] * point[ 1 ] + worldToScreen[ 0 ][ 2 ] * point[ 2 ] + worldToScreen[ 0 ][ 3 ];
	screen.y = worldToScreen[ 1 ][ 0 ] * point[ 0 ] + worldToScreen[ 1 ][ 1 ] * point[ 1 ] + worldToScreen[ 1 ][ 2 ] * point[ 2 ] + worldToScreen[ 1 ][ 3 ];
	w = worldToScreen[ 3 ][ 0 ] * point[ 0 ] + worldToScreen[ 3 ][ 1 ] * point[ 1 ] + worldToScreen[ 3 ][ 2 ] * point[ 2 ] + worldToScreen[ 3 ][ 3 ];
	screen.z = 0.0f;

	bool behind = false;

	if ( w < 0.001f )
	{
		behind = true;
		screen.x *= 100000;
		screen.y *= 100000;
	}
	else
	{
		behind = false;
		float invw = 1.0f / w;
		screen.x *= invw;
		screen.y *= invw;
	}

	return behind;
}

bool Math::ScreenTransform( const Vector& in, Vector2D& out ) {
	static auto& w2sMatrix = Interfaces::Engine->WorldToScreenMatrix( );

	out.x = w2sMatrix.arrData[ 0 ][ 0 ] * in.x + w2sMatrix.arrData[ 0 ][ 1 ] * in.y + w2sMatrix.arrData[ 0 ][ 2 ] * in.z + w2sMatrix.arrData[ 0 ][ 3 ];
	out.y = w2sMatrix.arrData[ 1 ][ 0 ] * in.x + w2sMatrix.arrData[ 1 ][ 1 ] * in.y + w2sMatrix.arrData[ 1 ][ 2 ] * in.z + w2sMatrix.arrData[ 1 ][ 3 ];

	float w = w2sMatrix.arrData[ 3 ][ 0 ] * in.x + w2sMatrix.arrData[ 3 ][ 1 ] * in.y + w2sMatrix.arrData[ 3 ][ 2 ] * in.z + w2sMatrix.arrData[ 3 ][ 3 ];

	if ( w < 0.001f ) {
		return false;
	}

	out.x /= w;
	out.y /= w;

	return true;
}