#include "logger.h"

void CLogger::PrintToScreen( ) {
	int iterations = 1;
	m_cOnscreenText.erase(
		std::remove_if(
			m_cOnscreenText.begin( ), m_cOnscreenText.end( ),
			[ & ]( const OnsCreen_t& onscreen ) -> bool {
				return onscreen.m_flTimeOnScreen <= 0.f;
			}
		),
		m_cOnscreenText.end( )
				);

	for ( auto& screenText : m_cOnscreenText ) {
		screenText.m_flTimeOnScreen -= Interfaces::Globals->flFrameTime;

		if ( screenText.m_flTimeOnScreen < 0.5f )
			screenText.m_fAlpha = std::max( screenText.m_fAlpha - ( Interfaces::Globals->flFrameTime * 2.f ), 0.f );
		else if ( screenText.m_flTimeOnScreen > 4.5f )
			screenText.m_fAlpha = std::min( screenText.m_fAlpha + ( Interfaces::Globals->flFrameTime * 2.f ), 1.f );

		const auto textSize = Render::GetTextSize( screenText.m_strText.c_str( ), Fonts::Logs );

		Render::FilledRoundedBox( { 4, 22 * iterations - 4 }, { textSize.x + 11, 21 }, 5, 5, Color( 10, 10, 10, static_cast< int >( 255 * screenText.m_fAlpha ) ) );
		Render::FilledRoundedBox( { 5, 22 * iterations - 3 }, { textSize.x + 9, 19 }, 5, 5, OUTLINE_LIGHT.Set<COLOR_A>( 255 * screenText.m_fAlpha ) );
		Render::FilledRoundedBox( { 6, 22 * iterations - 2 }, { textSize.x + 7, 17 }, 5, 5, Color( 10, 10, 10, static_cast< int >( 255 * screenText.m_fAlpha ) ) );
		Render::FilledRoundedBox( { 7, 22 * iterations - 1 }, { textSize.x + 5, 15 }, 5, 5, BACKGROUND.Set<COLOR_A>( 255 * screenText.m_fAlpha ) );
		Render::Line( { 5, 22 * iterations }, { 5, 22 * iterations - 3 + 17 }, ( iterations & 1 ? ACCENT : ACCENT2 ).Set<COLOR_A>( 255 * screenText.m_fAlpha ) );
		//Render::Line( { 4, 22 * iterations - 3 }, { 4, 22 * iterations - 3 + 18 }, ( screenText.m_bAccent ? ACCENT : ACCENT2 ).Set<COLOR_A>( 255 * screenText.m_fAlpha ) );
		Render::Text( Fonts::Logs, Vector2D( 10, 22 * iterations + 1 ), Color( 255, 255, 255, static_cast< int >( 255 * screenText.m_fAlpha ) ), 0, screenText.m_strText.c_str( ) );

		/*Render::FilledRectangle( 5, 22 * iterations - 3, textSize.x + 10, 18, BACKGROUND.Set<COLOR_A>( BACKGROUND.Get<COLOR_A>( ) / 2 * screenText.m_fAlpha ) );
		Render::Rectangle( 5, 22 * iterations - 4, textSize.x + 11, 20, ACCENT.Set<COLOR_A>( ACCENT.Get<COLOR_A>( ) / 2 * screenText.m_fAlpha ) );
		Render::Text( Fonts::Logs, Vector2D( 10, 22 * iterations ), Color( 255, 255, 255, static_cast<int>( 255 * screenText.m_fAlpha ) ), 0, screenText.m_strText.c_str( ) );*/

		iterations++;
	}
}