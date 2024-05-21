#include "../menu.h"
#include "../../../sdk/datatypes/color.h"

// TODO: use lambdas for callbacks
void CMenuItem::Button( ) {
	if ( !m_fnShow( ) )
		return;

	auto& args{ *reinterpret_cast< ButtonArgs_t* >( m_pArgs ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( args.m_iMaxGroups + 1 ) ) / args.m_iMaxGroups - PADDING * 4, ITEM_HEIGHT * 2.f ) };
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	Menu::LerpToCol( this->m_cColor, hovered ? Inputsys::down( VK_LBUTTON ) ? OUTLINE_DARK : DIM_ELEMENT : BACKGROUND );

	if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !Menu::m_pFocusItem.m_pItem )
		args.m_bDown = true;

	if ( args.m_bDown && Inputsys::released( VK_LBUTTON ) ) {
		args.m_bDown = false;

		// let user drag off and cancel it
		if ( hovered ) {
			if ( args.m_pCallback && args.m_flTimer ) {
				args.m_flTimer = 0.f;
				reinterpret_cast< void( __cdecl* )( ) >( args.m_pCallback )( );
			}
			else if ( !args.m_flTimer )
				args.m_flTimer = 3.f;
		}
	}

	if ( args.m_flTimer )
		args.m_flTimer -= Interfaces::Globals->flFrameTime;

	args.m_flTimer = std::max( args.m_flTimer, 0.f );

	Render::FilledRoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
	Render::FilledRoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_vecDrawPos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
	Render::FilledRoundedBox( Menu::m_vecDrawPos + 2, size - Vector2D( 4, 4 ), 4, 4, this->m_cColor );

	if ( args.m_flTimer )
		Render::Line( Menu::m_vecDrawPos + Vector2D{ 3, size.y - 1 }, Menu::m_vecDrawPos + Vector2D{ ( size.x - 3 ) * ( args.m_flTimer / 3.f ), size.y - 1 }, ACCENT.Set<COLOR_A>( ( args.m_flTimer / 3.f ) * 255.f ) );

	Render::Text( Fonts::Menu, Menu::m_vecDrawPos + ( size / 2.f ) - Vector2D{ 0, 8 }, Color( 1.f, 1.f, 1.f ), FONT_CENTER, args.m_flTimer ? _( "Are you sure?" ) : m_szName );

	Menu::m_vecDrawPos.y += ITEM_HEIGHT * 2.f + PADDING;
}