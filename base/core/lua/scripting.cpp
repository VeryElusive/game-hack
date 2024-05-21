/*#include "scripting.h"
#include "../../features/misc/logger.h"
#include "../../sdk/hash/fnv1a.h"

void Scripting::Unload( std::string name ) {
	const auto it = std::find_if( m_mapStates.begin( ), m_mapStates.end( ), [ &name ]( const auto& it ) -> bool {
		return it.first == name;
		} );

	if ( it != m_mapStates.end( ) )
		m_mapStates.erase( it );
}

void Scripting::AddCallback( std::string name, sol::protected_function callback, sol::this_state state ) {
	auto lua = sol::state_view( state.lua_state( ) );
	auto state_ptr = lua[ _( "__lua_state" ) ].get<std::shared_ptr<LuaState_t>>( );

	state_ptr->m_cCallbacks.insert_or_assign( FNV1A::Hash( name.data( ), name.length( ) ), callback );
}


void Scripting::Log( std::string text, bool onscreen ) {
	Features::Logger.Log( text, onscreen );
}

void Scripting::Load( std::string fileName ) {
	m_bLoadingScript = true;

	Unload( fileName );

	const auto state{ std::make_shared<LuaState_t>( ) };

	state->m_cState.open_libraries(
		sol::lib::base,
		sol::lib::package,
		sol::lib::string,
		sol::lib::math,
		sol::lib::table,
		sol::lib::bit32,
		sol::lib::utf8 );

	state->m_cState[ _( "__lua_state" ) ] = state;


	// initialisation
	const auto callbackTable{ state->m_cState[ _( "callbacks" ) ].get_or_create<sol::table>( ) };
	const auto renderTable{ state->m_cState[ _( "render" ) ].get_or_create<sol::table>( ) };


	// logging
	state->m_cState[ _( "log" ) ] = sol::overload( &Log );


	// callbacks
	callbackTable[ _( "register" ) ] = sol::overload( &AddCallback );


	// render
	renderTable[ _( "rectangle" ) ] = sol::overload( &Wrappers::Renderer::Rect );

	auto pathToFolder = Config::fsLuaPath;
	auto pathToFile = pathToFolder.append( fileName );


	auto result = state->m_cState.load_file( pathToFile );

	if ( !result.valid( ) ) {
		FAILED:
		sol::error err = result;
		Features::Logger.Log( err.what( ), true );
	}
	else {
		result = result.call( );

		if ( !result.valid( ) )
			goto FAILED;

		m_mapStates.insert_or_assign( fileName, state );
	}
	m_bLoadingScript = false;
}*/