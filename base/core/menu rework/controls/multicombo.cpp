#include "../menu.h"
#include "../../../sdk/datatypes/color.h"

#define MAX_ALPHA( col ) col.Set<COLOR_A>( Menu::m_pFocusItem.m_flFocusAnim * 255 )

void CMenuItem::MultiComboFocus( ) {
	auto& value{ *reinterpret_cast< int* >( m_pValue ) };
	const auto& args{ *reinterpret_cast< MultiComboArgs_t* >( m_pArgs ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 4, ( 15 * args.m_iSize + PADDING * 2 ) * Menu::m_pFocusItem.m_flFocusAnim ) };
	const auto pos{ Menu::m_pFocusItem.m_vecDrawPos + Vector2D{ 0, ITEM_HEIGHT * 1.5f + PADDING } };

	const auto sizeOG{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 4, ITEM_HEIGHT * 1.5f ) };
	Render::RoundedBox( Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 1, 1 ), sizeOG + Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
	Render::RoundedBox( Menu::m_pFocusItem.m_vecDrawPos, sizeOG, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos + Vector2D( 1, 1 ), sizeOG - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
	Render::Text( Fonts::Menu, Menu::m_pFocusItem.m_vecDrawPos - Vector2D{ 0, 16 }, Color( 255, 255, 255 ), FONT_LEFT, m_szName );

	std::string accumulatedValues{ };
	for ( int i{ }; i < args.m_iSize; ++i ) {
		if ( *args.m_pArguments[ i ].m_pValue ) {
			accumulatedValues += args.m_pArguments[ i ].m_szName;
			accumulatedValues += ", ";
		}
	}

	if ( accumulatedValues.size( ) ) {
		accumulatedValues.pop_back( );
		accumulatedValues.pop_back( );
	}
	else
		accumulatedValues = _( "None" );

	if ( accumulatedValues.length( ) * 7 > size.x - PADDING ) {
		accumulatedValues.erase( ( size.x - 30 ) / 7 + 4, accumulatedValues.length( ) - ( size.x - 30 ) / 7 + 4 );
		accumulatedValues.append( "..." );
	}

	Render::Text( Fonts::Menu, Menu::m_pFocusItem.m_vecDrawPos + Vector2D{ PADDING, 2 }, Color( 255, 255, 255 ), FONT_LEFT, accumulatedValues.c_str( ) );

	Interfaces::Surface->SetClipRect( pos.x - 1, pos.y, size.x + 2, size.y );

	Render::RoundedBox( pos - Vector2D( 1, 1 ), size + Vector2D( 2, 2 ), 4, 4, MAX_ALPHA( OUTLINE_DARK ) );
	Render::RoundedBox( pos, size, 4, 4, MAX_ALPHA( OUTLINE_LIGHT ) );
	Render::FilledRoundedBox( pos + Vector2D( 1, 1 ), size - Vector2D( 2, 2 ), 4, 4, MAX_ALPHA( OUTLINE_DARK ) );

	for ( int i{ }; i < args.m_iSize; ++i ) {
		auto& element{ args.m_pArguments[ i ] };

		const auto hovered{ Inputsys::hovered( pos + Vector2D( PADDING, PADDING + 15 * i ), Vector2D{ size.x, 15 } ) };

		Menu::LerpToCol( element.m_cColor, *element.m_pValue ? ( i & 1 ? ACCENT2 : ACCENT ) : hovered ? HOVERED_ELEMENT : DIM_ELEMENT );

		Render::Text( Fonts::Menu, pos + Vector2D( PADDING, PADDING + 15 * i ),
			MAX_ALPHA( element.m_cColor ), FONT_LEFT, element.m_szName );

		if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !Menu::m_pFocusItem.m_bFadeOut )
			*element.m_pValue = !*element.m_pValue;
	}

	Interfaces::Surface->SetClipRect( 0, 0, ctx.m_ve2ScreenSize.x, ctx.m_ve2ScreenSize.y );

	const auto hovered{ Inputsys::hovered( pos, size ) };

	if ( Inputsys::pressed( VK_LBUTTON ) && !hovered && !Menu::m_pFocusItem.m_bFrameAfterFocus ) {
		Menu::m_pFocusItem.m_bFrameAfterFocus = true;
		Menu::m_pFocusItem.m_bFadeOut = true;
	}
}

#undef MAX_ALPHA

void CMenuItem::MultiCombo( ) {
	if ( !m_fnShow( ) )
		return;

	const auto& value{ *reinterpret_cast< int* >( m_pValue ) };
	const auto& args{ *reinterpret_cast< MultiComboArgs_t* >( m_pArgs ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 4, ITEM_HEIGHT * 1.5f ) };

	Render::Text( Fonts::Menu, Menu::m_vecDrawPos, Color( 255, 255, 255 ), FONT_LEFT, m_szName );
	Menu::m_vecDrawPos.y += 16;
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	Render::RoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
	Render::RoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_vecDrawPos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

	std::string accumulatedValues{ };
	for ( int i{ }; i < args.m_iSize; ++i ) {
		if ( *args.m_pArguments[ i ].m_pValue ) {
			accumulatedValues += args.m_pArguments[ i ].m_szName;
			accumulatedValues += ", ";
		}
	}

	if ( accumulatedValues.size( ) ) {
		accumulatedValues.pop_back( );
		accumulatedValues.pop_back( );
	}
	else
		accumulatedValues = _( "None" );

	if ( accumulatedValues.length( ) * 7 > size.x - PADDING ) {
		accumulatedValues.erase( ( size.x - 30 ) / 7 + 4, accumulatedValues.length( ) - ( size.x - 30 ) / 7 + 4 );
		accumulatedValues.append( "..." );
	}

	Render::Text( Fonts::Menu, Menu::m_vecDrawPos + Vector2D{ PADDING, 2 }, Color( 255, 255, 255 ), FONT_LEFT, accumulatedValues.c_str( ) );

	if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !Menu::m_pFocusItem.m_pItem )
		Menu::m_pFocusItem.Update( this );

	Menu::m_vecDrawPos.y += ITEM_HEIGHT * 1.5f + PADDING;
}