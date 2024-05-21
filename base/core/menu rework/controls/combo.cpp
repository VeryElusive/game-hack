#include "../menu.h"
#include "../../../sdk/datatypes/color.h"

#define MAX_ALPHA( col ) col.Set<COLOR_A>( Menu::m_pFocusItem.m_flFocusAnim * 255 )

void CMenuItem::ComboFocus( ) {
	auto& value{ *reinterpret_cast< int* >( m_pValue ) };
	const auto& args{ *reinterpret_cast< ComboArgs_t* >( m_pArgs ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 4, ( 15 * args.m_iSize + PADDING * 2 ) * Menu::m_pFocusItem.m_flFocusAnim ) };
	const auto pos{ Menu::m_pFocusItem.m_vecDrawPos + Vector2D{ 0, ITEM_HEIGHT * 1.5f + PADDING } };

	const auto sizeOG{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 4, ITEM_HEIGHT * 1.5f ) };
	Render::RoundedBox( Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 1, 1 ), sizeOG + Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
	Render::RoundedBox( Menu::m_pFocusItem.m_vecDrawPos, sizeOG, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos + Vector2D( 1, 1 ), sizeOG - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
	Render::Text( Fonts::Menu, Menu::m_pFocusItem.m_vecDrawPos - Vector2D{ 0, 16 }, Color( 255, 255, 255 ), FONT_LEFT, m_szName );

	const auto& element{ args.m_pArguments[ value ] };
	Render::Text( Fonts::Menu, Menu::m_pFocusItem.m_vecDrawPos + Vector2D{ PADDING, 2 }, Color( 255, 255, 255 ), FONT_LEFT, element.m_szName );

	Interfaces::Surface->SetClipRect( pos.x - 1, pos.y, size.x + 2, size.y );

	Render::RoundedBox( pos - Vector2D( 1, 1 ), size + Vector2D( 2, 2 ), 4, 4, MAX_ALPHA( OUTLINE_DARK ) );
	Render::RoundedBox( pos, size, 4, 4, MAX_ALPHA( OUTLINE_LIGHT ) );
	Render::FilledRoundedBox( pos + Vector2D( 1, 1 ), size - Vector2D( 2, 2 ), 4, 4, MAX_ALPHA( OUTLINE_DARK ) );

	for ( int i{ }; i < args.m_iSize; ++i ) {
		auto& element{ args.m_pArguments[ i ] };

		const auto hovered{ Inputsys::hovered( pos + Vector2D( PADDING, PADDING + 15 * i ), Vector2D{ size.x, 15 } ) };

		Menu::LerpToCol( element.m_cColor, value == i ? ( i & 1 ? ACCENT2 : ACCENT ) : hovered ? HOVERED_ELEMENT : DIM_ELEMENT );

		Render::Text( Fonts::Menu, pos + Vector2D( PADDING, PADDING + 15 * i ),
			MAX_ALPHA( element.m_cColor ), FONT_LEFT, element.m_szName );

		if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !Menu::m_pFocusItem.m_bFadeOut )
			value = i;
	}

	Interfaces::Surface->SetClipRect( 0, 0, ctx.m_ve2ScreenSize.x, ctx.m_ve2ScreenSize.y );

	if ( Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus ) {
		Menu::m_pFocusItem.m_bFrameAfterFocus = true;
		Menu::m_pFocusItem.m_bFadeOut = true;
	}
}

#undef MAX_ALPHA

void CMenuItem::Combo( ) {
	if ( !m_fnShow( ) )
		return;

	const auto& value{ *reinterpret_cast< int* >( m_pValue ) };
	const auto& args{ *reinterpret_cast< ComboArgs_t* >( m_pArgs ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 4, ITEM_HEIGHT * 1.5f ) };

	Render::Text( Fonts::Menu, Menu::m_vecDrawPos, Color( 255, 255, 255 ), FONT_LEFT, m_szName );
	Menu::m_vecDrawPos.y += 16;
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	if ( Menu::m_pFocusItem.m_pItem != this ) {
		Render::RoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
		Render::RoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
		Render::FilledRoundedBox( Menu::m_vecDrawPos + Vector2D( 1, 1 ), size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

		const auto& element{ args.m_pArguments[ value ] };
		Render::Text( Fonts::Menu, Menu::m_vecDrawPos + Vector2D{ PADDING, 2 }, Color( 255, 255, 255 ), FONT_LEFT, element.m_szName );
	}

	if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_pItem )
		Menu::m_pFocusItem.Update( this );

	Menu::m_vecDrawPos.y += ITEM_HEIGHT * 1.5f + PADDING;
}