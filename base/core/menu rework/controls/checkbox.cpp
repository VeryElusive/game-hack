#include "../menu.h"
#include "../../../sdk/datatypes/color.h"

void CMenuItem::Checkbox( ) {
	if ( !m_fnShow( ) )
		return;

	auto& value{ *reinterpret_cast< bool* >( m_pValue ) };
	const auto size{ Vector2D{ ITEM_HEIGHT, ITEM_HEIGHT } };
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size + Vector2D{ Render::GetTextSize( m_szName, Fonts::Menu ).x + ITEM_HEIGHT * 2, 0 } ) };

	if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_pItem && !Menu::m_pFocusItem.m_bFrameAfterFocus )
		value = !value;

	Menu::LerpToCol( this->m_cColor, value ? ACCENT : hovered ? HOVERED_ELEMENT : BACKGROUND );

	Render::FilledRoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
	Render::FilledRoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_vecDrawPos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
	Render::FilledRoundedBox( Menu::m_vecDrawPos + 2, size - Vector2D( 4, 4 ), 4, 4, this->m_cColor );

	Render::Text( Fonts::Menu, Menu::m_vecDrawPos + Vector2D( ITEM_HEIGHT * 2, -2 ), Color( 255, 255, 255 ), FONT_LEFT, m_szName );

	Menu::m_vecDrawPos.y += ITEM_HEIGHT + PADDING;
}