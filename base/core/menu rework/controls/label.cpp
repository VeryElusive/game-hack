#include "../menu.h"
#include "../../../sdk/datatypes/color.h"

void CMenuItem::Label( ) {
	if ( !m_fnShow( ) )
		return;

	const auto size{ Vector2D{ ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 2, 16 } };
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	Menu::LerpToCol( this->m_cColor, hovered ? Color( 255, 255, 255 ) : SELECTED_ELEMENT );

	Render::Text( Fonts::Menu, Menu::m_vecDrawPos + Vector2D( 0, -2 ), this->m_cColor, FONT_LEFT, m_szName );

	Menu::m_vecDrawPos.y += ITEM_HEIGHT + PADDING;
}