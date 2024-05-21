#pragma once
#include "../../context.h"
#include "../../utils/math.h"
#include "../../core/menu rework/menu.h"

struct OnsCreen_t {
	OnsCreen_t( std::string str ) {
		m_strText = str;
	}

	std::string m_strText;
	float m_flTimeOnScreen{ 5 };
	float m_fAlpha{ };
};

class CLogger {
private:
	std::vector<OnsCreen_t> m_cOnscreenText;
public:
	void PrintToScreen( );
	FORCEINLINE void Log( std::string text, bool show_on_screen ) {
		text.append( "\n" );
		Interfaces::ConVar->ConsoleColorPrintf( ACCENT, _( "[ HAVOC ] " ) );
		Interfaces::ConVar->ConsolePrintf( text.c_str( ) );

		for ( auto it{ text.begin( ) }; it != text.end( ); ++it ) {
			auto& character{ *it };

			if ( character > 255 )
				character = '*';
		}

		if ( show_on_screen )
			m_cOnscreenText.push_back( text );
	}
};

namespace Features { inline CLogger Logger; };