#include "../menu.h"
#include "../../../sdk/datatypes/color.h"
#include "../../config.h"

// STATIC IMPLEMENTATION!
// TODO: (only if using it multiple times)- dynamically do it. for now idc :)
void CMenuItem::Listbox( ) {
	auto& value{ *reinterpret_cast< int* >( m_pValue ) };
	auto& scroll{ *reinterpret_cast< float* >( m_pArgs ) };


	//Render::Text( Fonts::Menu, Menu::m_vecDrawPos, Color( 255, 255, 255 ), FONT_LEFT, m_szName );
	//Menu::m_vecDrawPos.y += 16;

	static std::vector< ComboElement_t > configs{ };
	if ( !configs.size( ) )
		Config::m_bDueRefresh = true;

	bool refreshed{ };
	if ( Config::m_bDueRefresh ) {
		Config::Refresh( );

		configs.clear( );
		for ( auto& config : Config::vecFileNames )
			configs.push_back( config.c_str( ) );

		refreshed = true;
		Config::m_bDueRefresh = false;
	}

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 2 ) / 1 - PADDING * 4, Menu::m_vecSize.y - MARGIN * 2 - PADDING * 4 - PADDING * 6 - ITEM_HEIGHT * 2 * 6 ) };

	Render::RoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
	Render::RoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_vecDrawPos + Vector2D( 1, 1 ), size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

	Interfaces::Surface->SetClipRect( Menu::m_vecDrawPos.x + 2, Menu::m_vecDrawPos.y + 2, size.x - 4, size.y - 4 );

	const auto wholeHovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	for ( int i{ }; i < configs.size( ); ++i ) {
		auto& element{ configs.at( i ) };

		const auto pos{ Menu::m_vecDrawPos + Vector2D( PADDING, 2 + 15 * i - scroll ) };

		const auto hovered{ Inputsys::hovered( pos, Vector2D{ size.x, 15 } ) };

		if ( !refreshed )
			Menu::LerpToCol( element.m_cColor, value == i ? ( i & 1 ? ACCENT2 : ACCENT ) : hovered ? HOVERED_ELEMENT : DIM_ELEMENT );
		else
			element.m_cColor = value == i ? ( i & 1 ? ACCENT2 : ACCENT ) : hovered ? HOVERED_ELEMENT : DIM_ELEMENT;

		Render::Text( Fonts::Menu, pos, element.m_cColor, FONT_LEFT, element.m_szName );

		if ( wholeHovered && hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_pItem && !Menu::m_pFocusItem.m_bFrameAfterFocus )
			value = i;
	}

	Interfaces::Surface->SetClipRect( 0, 0, ctx.m_ve2ScreenSize.x, ctx.m_ve2ScreenSize.y );

	const float outside{ std::max( 0.f, PADDING + 15 * ( configs.size( ) ) - scroll - size.y + scroll ) };
	if ( outside ) {
		const float percent{ scroll / outside };
		const float barLength{ size.y * ( 1.f - ( outside / size.y ) ) };
		Render::FilledRectangle( Menu::m_vecDrawPos + Vector2D( size.x - 3, percent * ( size.y - barLength ) ),
			{ 2, barLength }, ACCENT );
	}

	if ( wholeHovered ) {
		scroll -= Inputsys::scroll * 12;
		scroll = std::min( outside, scroll );
		scroll = std::max( 0.f, scroll );
	}

	Menu::m_vecDrawPos.y += size.y + PADDING;
}