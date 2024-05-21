#include "../menu.h"
#include "../../../sdk/datatypes/color.h"

void CMenuItem::TextInputFocus( ) {
	auto& maxGroups{ *reinterpret_cast< int* > ( m_pArgs ) };
	auto& value{ *reinterpret_cast< std::string* > ( m_pValue ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( maxGroups + 1 ) ) / maxGroups - PADDING * 4, ITEM_HEIGHT * 2.f ) };
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

	Render::Text( Fonts::Menu, Menu::m_pFocusItem.m_vecDrawPos + Vector2D{ PADDING, 4 }, Color( 1.f, 1.f, 1.f ), FONT_LEFT, value.c_str( ) );

	const auto textSize{ Render::GetTextSize( value, Fonts::Menu ) };

	const auto second{ static_cast< int >( Interfaces::Globals->flRealTime ) };
	this->m_cColor = Color( 1.f, 1.f, 1.f, second & 1 ? Interfaces::Globals->flRealTime - second :  1.f - ( Interfaces::Globals->flRealTime - second ) );
	Render::Line( Menu::m_pFocusItem.m_vecDrawPos + Vector2D{ PADDING + textSize.x, 4 }, Menu::m_pFocusItem.m_vecDrawPos + Vector2D{ PADDING + textSize.x, size.y - 4 }, this->m_cColor );


	if ( Inputsys::pressed( VK_ESCAPE ) || Inputsys::pressed( VK_RETURN ) ) {
		Menu::m_pFocusItem.m_pItem = nullptr;
		Menu::m_pFocusItem.m_bFrameAfterFocus = true;
	}
	else if ( Inputsys::pressed( VK_BACK ) )
		value = value.substr( 0, value.size( ) - 1 );
	else if ( value.size( ) < 110 ) {
		for ( auto i{ 0 }; i < 255; i++ ) {
			if ( isalpha( i ) ) {
				if ( Inputsys::pressed( i ) ) {
					auto final{ i };
					if ( !GetKeyState( VK_CAPITAL ) && !Inputsys::down( VK_SHIFT ) )
						final += 32;

					value += final;
				}
			}
		}

		if ( Inputsys::pressed( VK_SPACE ) )
			value += ' ';
	}

	if ( Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !hovered ) {
		Menu::m_pFocusItem.m_pItem = nullptr;
		Menu::m_pFocusItem.m_bFrameAfterFocus = true;
	}

	Menu::m_vecDrawPos.y += size.y + PADDING;
}

void CMenuItem::TextInput( ) {
	if ( !m_fnShow( ) )
		return;

	auto& maxGroups{ *reinterpret_cast< int* > ( m_pArgs ) };
	auto& value{ *reinterpret_cast< std::string* > ( m_pValue ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( maxGroups + 1 ) ) / maxGroups - PADDING * 4, ITEM_HEIGHT * 2.f ) };
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !Menu::m_pFocusItem.m_pItem )
		Menu::m_pFocusItem.Update( this );

	Render::FilledRoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
	Render::FilledRoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_vecDrawPos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

	Render::Text( Fonts::Menu, Menu::m_vecDrawPos + Vector2D{ PADDING, 4 }, Color( 1.f, 1.f, 1.f ), FONT_LEFT, value.c_str( ) );

	Menu::m_vecDrawPos.y += ITEM_HEIGHT * 2.f + PADDING;
}