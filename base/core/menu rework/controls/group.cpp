#include "../menu.h"

#define GET_HEIGHT( percent ) std::ceil( static_cast<float>( Menu::m_vecSize.y - MARGIN * 2 - ( rowBelow ? MARGIN : 0 ) ) * percent )

void CMenuGroup::Render( int groupNum, std::vector < CMenuGroup >& groups ) {
	const auto& groupCount{ groups.size( ) };

	// TODO: properly handle this in the case that we use 3 groups
	const auto rowBelow{ groupNum == 1 ? groupCount > m_iParentMaxGroups && this->m_flPercent < 1.f 
		: groupNum == 2 ? ( groupCount > m_iParentMaxGroups && groups.front( ).m_flPercent >= 1.f ) || groupCount >= 4 : groupNum > m_iParentMaxGroups };

	auto size{ Vector2D{ ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( m_iParentMaxGroups + 1 ) ) / m_iParentMaxGroups, GET_HEIGHT( this->m_flPercent ) } };
	bool hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	// new line
	if ( groupNum > 2 ) {
		if ( groups.front( ).m_flPercent + groups.front( ).m_flPercent > 0.99f ) {
			Menu::m_vecDrawPos.x += size.x + MARGIN;
			Menu::m_vecDrawPos.y += GET_HEIGHT( groups.at( 1 ).m_flPercent ) + MARGIN;
		}
		else {
			Menu::m_vecDrawPos.y += GET_HEIGHT( groups.front( ).m_flPercent ) + MARGIN;
		}
	}

	size = Vector2D{ ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( m_iParentMaxGroups + 1 ) ) / m_iParentMaxGroups, GET_HEIGHT( this->m_flPercent ) };
	hovered = Inputsys::hovered( Menu::m_vecDrawPos, size );


	Render::RoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );
	Render::RoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( Menu::m_vecDrawPos + Vector2D( 1, 1 ), size - Vector2D( 2, 2 ), 4, 4, GROUP );

	Interfaces::Surface->SetClipRect( Menu::m_vecDrawPos.x + 2, Menu::m_vecDrawPos.y + 2, size.x - 4, size.y - 4 );

	const auto backupDrawPos{ Menu::m_vecDrawPos };
	Menu::m_vecDrawPos.x += PADDING * 2;
	Menu::m_vecDrawPos.y += PADDING * 2;
	Menu::m_vecDrawPos.y -= m_flScroll;


	if ( this->m_vecItems.size( ) ) {
		for ( auto& element : this->m_vecItems )
			element.Render( );
	}

	Interfaces::Surface->SetClipRect( 0, 0, ctx.m_ve2ScreenSize.x, ctx.m_ve2ScreenSize.y );

	const float outside{ std::max( 0.f, Menu::m_vecDrawPos.y - backupDrawPos.y - size.y + m_flScroll ) };
	if ( hovered )
		m_flScroll -= Inputsys::scroll * 12;
	
	m_flScroll = std::min( outside, m_flScroll );
	m_flScroll = std::max( 0.f, m_flScroll );

	Menu::m_vecDrawPos = backupDrawPos;

	if ( outside ) {
		const float percent{ m_flScroll / outside };
		const float barLength{ ( size.y - PADDING * 2 ) * ( 1.f - ( outside / size.y ) ) };
		Render::FilledRectangle( Menu::m_vecDrawPos + Vector2D( size.x - 3, PADDING * 2 + percent * ( size.y - PADDING * 2 - barLength ) ),
			{ 2, barLength }, ACCENT );
	}

	Render::Gradient( Menu::m_vecDrawPos.x + 2, Menu::m_vecDrawPos.y + 2, size.x - 4, PADDING * 2, GROUP, GROUP.Set<COLOR_A>( 0.f ), false );
	Render::Gradient( Menu::m_vecDrawPos.x + 2, Menu::m_vecDrawPos.y + size.y - PADDING * 2 - 1, size.x - 4 - 1, PADDING * 2, GROUP.Set<COLOR_A>( 0.f ), GROUP, false );

	const auto textX{ Render::GetTextSize( m_szName, Fonts::Menu ).x + ( PADDING * 2 ) };
	Render::FilledRectangle( Menu::m_vecDrawPos + Vector2D{ PADDING, -1.f }, { textX, 4.f, }, GROUP );
	Render::Text( Fonts::Menu, Menu::m_vecDrawPos + Vector2D{ ( PADDING * 2 ), -4.f }, Color( 255, 255, 255 ), FONT_LEFT, m_szName );

	// new line
	if ( groupNum == 2 && groupCount > 2 )
		Menu::m_vecDrawPos.x = Menu::m_vecPos.x + BAR_SIZE + MARGIN;
	else
		Menu::m_vecDrawPos.x += size.x + MARGIN;
}

void CMenuGroup::Reset( ) {
	if ( this->m_vecItems.size( ) ) {
		for ( auto& element : this->m_vecItems ) {
			if ( element.m_eItemType == SLIDER_FLOAT || element.m_eItemType == SLIDER_INT )
				reinterpret_cast< SliderArgs_t* >( element.m_pArgs )->m_flValue = 0;

			//element.m_cColor = BACKGROUND;
		}
	}
}

void CMenuItem::RenderFocus( ) {
	switch ( this->m_eItemType ) {
	case SLIDER_FLOAT:
		SliderFloatFocus( );
		break;
	case SLIDER_INT:
		SliderIntFocus( );
		break;
	case COMBO:
		ComboFocus( );
		break;
	case MULTI_COMBO:
		MultiComboFocus( );
		break;
	case KEY_BIND:
		KeybindFocus( );
		break;
	case COLOR_PICKER:
		ColorPickerFocus( );
		break;
	case TEXT_INPUT:
		TextInputFocus( );
		break;
	default:
		break;
	}
}

void CMenuItem::Render( ) {
	switch ( this->m_eItemType ) {
	case CHECKBOX:
		Checkbox( );
		break;
	case SLIDER_FLOAT:
		SliderFloat( );
		break;
	case SLIDER_INT:
		SliderInt( );
		break;
	case COMBO:
		Combo( );
		break;
	case MULTI_COMBO:
		MultiCombo( );
		break;
	case KEY_BIND:
		Keybind( );
		break;
	case COLOR_PICKER:
		ColorPicker( );
		break;	
	case LABEL:
		Label( );
		break;
	case BUTTON:
		Button( );
		break;
	case LIST_BOX:
		Listbox( );
		break;
	case TEXT_INPUT:
		TextInput( );
		break;
	default:
		break;
	}
}