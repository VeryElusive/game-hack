#include "../menu.h"
#include "../../../sdk/datatypes/color.h"

const char* keys[ ] = { "[-]", "[M1]", "[M2]", "[BRK]", "[M3]", "[M4]", "[M5]",
"[_]", "[BSPC]", "[TAB]", "[_]", "[_]", "[_]", "[ENTER]", "[_]", "[_]", "[SHFT]",
"[CTRL]", "[ALT]", "[PAUSE]", "[CAPS]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[ESC]", "[_]", "[_]", "[_]", "[_]", "[SPACE]", "[PGUP]", "[PGDOWN]", "[END]", "[HOME]", "[LEFT]",
"[UP]", "[RIGHT]", "[DOWN]", "[_]", "[PRNT]", "[_]", "[PRTSCR]", "[INS]", "[DEL]", "[_]", "[0]", "[1]",
"[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[A]", "[B]", "[C]", "[D]", "[E]", "[F]", "[G]", "[H]", "[I]", "[J]", "[K]", "[L]", "[M]", "[N]", "[O]", "[P]", "[Q]", "[R]", "[S]", "[T]", "[U]",
"[V]", "[W]", "[X]", "[Y]", "[Z]", "[LFTWIN]", "[RGHTWIN]", "[_]", "[_]", "[_]", "[NUM0]", "[NUM1]",
"[NUM2]", "[NUM3]", "[NUM4]", "[NUM5]", "[NUM6]", "[NUM7]", "[NUM8]", "[NUM9]", "[*]", "[+]", "[_]", "[-]", "[.]", "[/]", "[F1]", "[F2]", "[F3]",
"[F4]", "[F5]", "[F6]", "[F7]", "[F8]", "[F9]", "[F10]", "[F11]", "[F12]", "[F13]", "[F14]", "[F15]", "[F16]", "[F17]", "[F18]", "[F19]", "[F20]", "[F21]",
"[F22]", "[F23]", "[F24]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[NUMLCK]", "[SCRLLCK]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[LSHFT]", "[RSHFT]", "[LCTRL]",
"[RCTRL]", "[LALT]", "[RALT]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[NTRK]", "[PTRK]", "[STOP]", "[PLAY]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[_]", "[;]", "[+]", "[,]", "[-]", "[.]", "[?]", "[~]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]",
"[_]", "[_]" };

#define MAX_ALPHA( col ) col.Set<COLOR_A>( Menu::m_pFocusItem.m_flFocusAnim * 255 )

ComboElement_t items[ 4 ]{ _( "Always on" ),_( "Toggle" ), _( "Hold" ), _( "Off hotkey" ) };

void CMenuItem::KeybindFocus( ) {
	auto& value{ *reinterpret_cast< keybind_t* >( m_pValue ) };

	const auto size{ Vector2D( Render::GetTextSize( items[3].m_szName, Fonts::Menu ).x + PADDING * 2, 15 * 4 + PADDING * 2 ) };

	Render::RoundedBox( Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 1, 1 ), size + Vector2D( 2, 2 ), 4, 4, MAX_ALPHA( OUTLINE_DARK ) );
	Render::RoundedBox( Menu::m_pFocusItem.m_vecDrawPos, size, 4, 4, MAX_ALPHA( OUTLINE_LIGHT ) );
	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos + Vector2D( 1, 1 ), size - Vector2D( 2, 2 ), 4, 4, MAX_ALPHA( OUTLINE_DARK ) );

	for ( int i{ }; i < 4; ++i ) {
		auto& element{ items[ i ] };

		const auto hovered{ Inputsys::hovered( Menu::m_pFocusItem.m_vecDrawPos + Vector2D( PADDING, PADDING + 15 * i ), Vector2D{ size.x, 15 } ) };

		Menu::LerpToCol( element.m_cColor, value.mode == i ? ( i & 1 ? ACCENT2 : ACCENT ) : hovered ? HOVERED_ELEMENT : DIM_ELEMENT );

		Render::Text( Fonts::Menu, Menu::m_pFocusItem.m_vecDrawPos + Vector2D( PADDING, PADDING + 15 * i ),
			MAX_ALPHA( element.m_cColor ), FONT_LEFT, element.m_szName );

		if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !Menu::m_pFocusItem.m_bFadeOut )
			value.mode = i;
	}

	const auto hovered{ Inputsys::hovered( Menu::m_pFocusItem.m_vecDrawPos, size ) };

	if ( Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus ) {
		Menu::m_pFocusItem.m_bFrameAfterFocus = true;
		Menu::m_pFocusItem.m_bFadeOut = true;
	}
}

#undef MAX_ALPHA

void CMenuItem::Keybind( ) {
	if ( !m_fnShow( ) )
		return;

	auto& value{ *reinterpret_cast< keybind_t* >( m_pValue ) };
	const char* renderName{ keys[ value.key ] };

	const auto textSize{ Render::GetTextSize( renderName, Fonts::Menu ) };

	const auto pos{ Menu::m_vecDrawPos + Vector2D{ ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 4, -ITEM_HEIGHT - PADDING } };
	const auto hovered{ Inputsys::hovered( pos - Vector2D{textSize.x, 0 }, textSize ) };
	if ( hovered && Inputsys::pressed( VK_RBUTTON ) && !Menu::m_pFocusItem.m_pItem && !Menu::m_pFocusItem.m_bFrameAfterFocus ) {
		const auto backup{ Menu::m_vecDrawPos };
		Menu::m_vecDrawPos = pos;
		Menu::m_pFocusItem.Update( this );
		Menu::m_vecDrawPos = backup;
	}

	if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_pItem && !Menu::m_pFocusItem.m_bFrameAfterFocus )
		m_pArgs = this;
	else if ( !hovered && Inputsys::pressed( VK_LBUTTON ) )
		m_pArgs = nullptr;

	if ( m_pArgs ) {
		for ( int i{ 2 }; i < 255; i++ ) {
			if ( Inputsys::pressed( i ) ) {
				if ( i == VK_ESCAPE ) {
					value.key = 0;
					m_pArgs = nullptr;
					break;
				}

				value.key = i;
				m_pArgs = nullptr;
				break;
			}
		}
	}

	Render::Text( Fonts::Menu, pos, m_pArgs ? HOVERED_ELEMENT : SELECTED_ELEMENT, FONT_RIGHT, renderName );
}