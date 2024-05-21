#include <thread>
#include "havoc.h"
#include "context.h"
#include "utils/math.h"
#include "utils/render.h"
#include "utils/crash_dumper.h"
#include "core/prop_manager.h"
#include "core/config.h"
#include "core/hooks.h"
#include "core/menu rework/menu.h"
#include "core/displacement.h"
#include "core/event_listener.h"
#include "features/misc/logger.h"
#include "features/misc/skinchanger/kitparser.h"
#include <tchar.h>

__forceinline void memsetINLINED( unsigned char* dest, char src, size_t size ) {
	for ( size_t i = 0; i < size; ++i ) {
		dest[ i ] = src;
	}
}

__forceinline void XORBlock( unsigned char* dest, char src, size_t size ) {
	for ( size_t i = 0; i < size; ++i ) {
		dest[ i ] ^= src;
	}
}

__forceinline void eraseFunction( size_t startOf, size_t endFunction ) {
#ifndef _DEBUG
	auto sizeOf{ endFunction - startOf };
	DWORD oldProt{ };

	VirtualProtect( ( LPVOID ) startOf, sizeOf, PAGE_READWRITE, &oldProt );
	memsetINLINED( ( unsigned char* ) startOf, 0, sizeOf );
	VirtualProtect( ( LPVOID ) startOf, sizeOf, oldProt, &oldProt );
#endif // !_DEBUG
}

FILE* pStream;

void Entry( void* netvars ) {
	//SetUnhandledExceptionFilter( UnhandledExFilter );

	while ( !MEM::GetModuleBaseHandle( SERVERBROWSER_DLL ) )
		std::this_thread::sleep_for( 200ms );

	// interfaces
	bool end{ !Interfaces::Setup( ) };
	eraseFunction( reinterpret_cast< size_t >( Interfaces::Setup ), reinterpret_cast< size_t >( Interfaces::CheckInterfaces ) );
	if ( end )
		return;

	//initialize_kits( );

	Menu::Register( );
	// WAHAAHHAHAHAHAHAHHAHA
	eraseFunction( reinterpret_cast< size_t >( Menu::Register ), reinterpret_cast< size_t >( Menu::LerpToCol ) );

	Config::Setup( );

	Interfaces::GameConsole->Clear( );

	// netvar
	if ( !PropManager::Get( ).Create( ) )
		return;

	Displacement::Init( netvars );
	eraseFunction( reinterpret_cast< size_t >( Displacement::Init ), reinterpret_cast< size_t >( Displacement::FindInDataMap ) );

	// math exports
	end = !Math::Setup( );
	eraseFunction( reinterpret_cast< size_t >( Math::Setup ), reinterpret_cast< size_t >( Math::Stub ) );
	if ( end )
		return;
	
	Render::CreateFonts( );
	EventListener.Setup( { _( "bullet_impact" ), _( "round_start" ), _( "player_hurt" ),_( "weapon_fire" ), _( "player_death" ) } );

	// hook da funcs
	end = !Hooks::Setup( );
	eraseFunction( reinterpret_cast< size_t >( Hooks::Setup ), reinterpret_cast< size_t >( Hooks::Restore ) );
	if ( end )
		return;

	long long amongus = 0x69690004C201B0;
	for ( auto& mod : { CLIENT_DLL, ENGINE_DLL, STUDIORENDER_DLL, MATERIALSYSTEM_DLL } )
		WriteProcessMemory( GetCurrentProcess( ), ( LPVOID ) MEM::FindPattern( mod, "55 8B EC 56 8B F1 33 C0 57 8B 7D 08" ), &amongus, 5, 0 );

	const auto setupVelocityClamp{ MEM::FindPattern( CLIENT_DLL, _( "0F 2F 15 ? ? ? ? 0F 86 ? ? ? ? F3 0F 7E 4C 24" ) ) + 0x3 };
	auto p{ *reinterpret_cast< float** >( setupVelocityClamp ) };
	DWORD old{ };
	VirtualProtect( ( LPVOID ) p, 4, PAGE_READWRITE, &old );
	*p = FLT_MAX;
	VirtualProtect( ( LPVOID ) p, 4, old, &old );

	_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
	_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_ON );

	SetPriorityClass( GetCurrentProcess( ), HIGH_PRIORITY_CLASS );

	Features::Logger.Log( _( "Deployed Havoc." ), true );


	while ( !ctx.m_bUnload )
		std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

	// restore hooks
	Hooks::Restore( );

	EventListener.Destroy( );

	for ( int i{ 1 }; i < 64; ++i ) {
		const auto player{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
		if ( player )
			player->m_bClientSideAnimation( ) = true;
	}

#ifndef _RELEASE
	delete Displacement::Netvars;
#endif

	if ( netvars )
		delete netvars;

	//FreeLibraryAndExitThread( bundle->module, EXIT_SUCCESS );
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if ( dwReason == DLL_PROCESS_ATTACH ) {
		DisableThreadLibraryCalls( hModule );

		// welcome to artiehack/timhack/ETHEREAL/havoc AKA BEST HVH CHEAT
		if ( const HANDLE hThread = ::CreateThread( nullptr, 0U, LPTHREAD_START_ROUTINE( Entry ), lpReserved, 0UL, nullptr ); hThread != nullptr )
			::CloseHandle( hThread );

		return TRUE;
	}

	return FALSE;
}