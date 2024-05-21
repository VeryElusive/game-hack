#include "menu.h"

void Menu::Render( ) {
	if ( !m_bOpened )
		m_pFocusItem.m_pItem = nullptr;

	if ( m_flAlpha >= 0 && !m_bOpened )
		m_flAlpha -= 5.f * Interfaces::Globals->flFrameTime;
	else if ( m_flAlpha <= 1 && m_bOpened )
		m_flAlpha += 5.f * Interfaces::Globals->flFrameTime;

	m_flAlpha = std::clamp( m_flAlpha, 0.f, 1.f );

	if ( m_flAlpha <= 0 )
		return;

	HandleControls( );

	m_bRendering = true;

	// TODO: fix RoundedBox... its caused by anti-aliasing of DrawTexturedPolygon, while DrawTexturedPolyLine doesn't anti alias
	Render::RoundedBox( m_vecPos - Vector2D{ 1, 1 }, m_vecSize, 5, 5, ACCENT );
	Render::RoundedBox( m_vecPos + Vector2D{ 1, 1 }, m_vecSize, 5, 5, ACCENT2 );

	//Interfaces::Surface->SetClipRect( m_vecPos.x - 1, m_vecPos.y - 1, m_vecSize.x / 2, m_vecSize.y + 1 );
	Render::FilledRoundedBox( m_vecPos, m_vecSize, 5, 5, BACKGROUND );
	Render::RoundedBox( m_vecPos, m_vecSize, 5, 5, OUTLINE_DARK );
	Render::RoundedBox( m_vecPos + Vector2D{ 1,1 }, m_vecSize - Vector2D{ 2, 2 }, 5, 5, OUTLINE_LIGHT );

	const auto barSize{ Vector2D{ BAR_SIZE, m_vecSize.y - 4 } };

	Render::FilledRoundedBox( m_vecPos + Vector2D{ 2, 2 }, barSize, 5, 5, OUTLINE_DARK );
	Render::RoundedBox( m_vecPos + Vector2D{ 3, 3 }, barSize - Vector2D{ 2, 2 }, 5, 5, OUTLINE_LIGHT );

	Render::Text( Fonts::Menu, m_vecPos + Vector2D( BAR_SIZE / 2, 20 ), ACCENT, FONT_CENTER, _( "Havoc" ) );
	Render::Text( Fonts::Menu, m_vecPos + Vector2D( m_vecSize.x - 125, m_vecSize.y - MARGIN + 2 ), DIM_ELEMENT, 0, _( "Developed by" ) );
	Render::Text( Fonts::Menu, m_vecPos + Vector2D( m_vecSize.x - 56, m_vecSize.y - MARGIN + 2 ), ACCENT, 0, _( "Artie" ) );

	RenderElements( );

	//Interfaces::Surface->SetClipRect( 0, 0, ctx.m_ve2ScreenSize.x, ctx.m_ve2ScreenSize.y );

	m_bRendering = false;
	m_pFocusItem.m_bFrameAfterFocus = false;
}

// TODO: rename inputsys to input and fucken fix the variable names lmfao
void Menu::HandleControls( ) {
	const auto topBarHovered{ Inputsys::hovered( m_vecPos, { m_vecSize.x, MARGIN } ) };

	if ( !m_bDraggingMenu && Inputsys::pressed( VK_LBUTTON ) && topBarHovered )
		m_bDraggingMenu = true;

	else if ( m_bDraggingMenu && Inputsys::down( VK_LBUTTON ) )
		m_vecPos -= Inputsys::MouseDelta;
	else if ( m_bDraggingMenu && !Inputsys::down( VK_LBUTTON ) )
		m_bDraggingMenu = false;


	const auto bottomCornerHovered{ Inputsys::hovered( m_vecPos + m_vecSize - Vector2D( 20, 20 ), Vector2D( 20, 20 ) ) };

	if ( bottomCornerHovered || m_bDraggingSize ) {
		SetCursor( true );

		if ( Inputsys::down( VK_LBUTTON ) ) {
			m_bDraggingSize = true;
			m_vecSize += Vector2D( Inputsys::MousePos - ( m_vecPos + m_vecSize ) );
		}
		else
			m_bDraggingSize = false;

	}
	else
		SetCursor( false );

	m_vecSize.x = std::clamp( static_cast< int >( m_vecSize.x ), 570, 1000 );
	m_vecSize.y = std::clamp( static_cast< int >( m_vecSize.y ), 420, 737 );
}

void Menu::SetCursor( bool resize ) {
	/*if ( resize )
		SetCursor( CursorResize );
	else
		SetCursor( CursorArrow );*/
}

void FocusItem_t::Update( CMenuItem* item ) {
	this->m_pItem = item;
	this->m_vecDrawPos = Menu::m_vecDrawPos;
	this->m_bFadeOut = false;
	this->m_bFrameAfterFocus = true;
}