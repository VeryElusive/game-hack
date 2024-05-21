#include "../menu.h"
#include "../../../sdk/datatypes/color.h"

void CMenuItem::SliderFloatFocus( ) {
	auto& value{ *reinterpret_cast< float* >( m_pValue ) };
	auto& args{ *reinterpret_cast< SliderArgs_t* >( m_pArgs ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( args.m_iMaxGroups + 1 ) ) / args.m_iMaxGroups - PADDING * 4, ITEM_HEIGHT ) };
	const auto pos{ Menu::m_pFocusItem.m_vecDrawPos - Vector2D{ 0, 7 * Menu::m_pFocusItem.m_flFocusAnim } };

	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );

	const auto sliderRatio{ Menu::mapNumber( args.m_flValue, args.m_flMin, args.m_flMax, 0.f, 1.f ) };

	Render::Text( Fonts::Menu, pos - Vector2D{ 0, 16 }, Color( 255, 255, 255 ), FONT_LEFT, m_szName );

	Render::FilledRoundedBox( pos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
	Render::FilledRoundedBox( pos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( pos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

	Render::FilledRoundedBox( pos + 2, Vector2D( std::max( std::ceil( ( size.x - 4 ) * sliderRatio ), 8.f ), size.y - 4 ), 4, 4, this->m_cColor );

	Render::Text( Fonts::Menu, pos + Vector2D( size.x / 2, -1 ), Color( 255, 255, 255 ), FONT_CENTER, std::to_string( static_cast< int >( value ) ).c_str( ) );
}

void CMenuItem::SliderFloat( ) {
	auto& args{ *reinterpret_cast< SliderArgs_t* >( m_pArgs ) };

	if ( !m_fnShow( ) ) {
		args.m_flValue = 0.f;
		return;
	}

	auto& value{ *reinterpret_cast< float* >( m_pValue ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( args.m_iMaxGroups + 1 ) ) / args.m_iMaxGroups - PADDING * 4, ITEM_HEIGHT ) };

	value = std::clamp( value, args.m_flMin, args.m_flMax );

	if ( Menu::m_pFocusItem.m_pItem != this )
		Render::Text( Fonts::Menu, Menu::m_vecDrawPos, Color( 255, 255, 255 ), FONT_LEFT, m_szName );

	Menu::m_vecDrawPos.y += 16;
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	args.m_flValue = Math::Interpolate( args.m_flValue, value, ANIMATION_SPEED );

	Menu::LerpToCol( this->m_cColor, Menu::m_pFocusItem.m_pItem == this && !Menu::m_pFocusItem.m_bFadeOut ? ACCENT : hovered ? HOVERED_ELEMENT : DIM_ELEMENT );

	if ( Menu::m_pFocusItem.m_pItem == this ) {
		if ( !Menu::m_pFocusItem.m_bFadeOut ) {
			const auto offset{ std::clamp( Vector2D( Inputsys::MousePos - Menu::m_vecDrawPos ).x, 0.f, size.x ) };
			value = Menu::mapNumber( offset, 0, size.x, args.m_flMin, args.m_flMax );
		}

		if ( Inputsys::released( VK_LBUTTON ) ) {
			Menu::m_pFocusItem.m_bFrameAfterFocus = true;
			Menu::m_pFocusItem.m_bFadeOut = true;
		}
	}
	else {
		if ( !Menu::m_pFocusItem.m_pItem && hovered && !Menu::m_pFocusItem.m_bFrameAfterFocus && !Menu::m_pFocusItem.m_pItem ) {
			if ( Inputsys::pressed( VK_LBUTTON ) )
				Menu::m_pFocusItem.Update( this );
		}

		const auto sliderRatio{ Menu::mapNumber( args.m_flValue, args.m_flMin, args.m_flMax, 0.f, 1.f ) };

		Render::FilledRoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
		Render::FilledRoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
		Render::FilledRoundedBox( Menu::m_vecDrawPos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

		Render::FilledRoundedBox( Menu::m_vecDrawPos + 2, Vector2D( std::max( std::ceil( ( size.x - 4 ) * sliderRatio ), 8.f ), size.y - 4 ), 4, 4, this->m_cColor );

		Render::Text( Fonts::Menu, Menu::m_vecDrawPos + Vector2D( size.x / 2, -1 ), Color( 255, 255, 255 ), FONT_CENTER, std::to_string( static_cast< int >( value ) ).c_str( ) );
	}

	Menu::m_vecDrawPos.y += ITEM_HEIGHT + PADDING;
}

void CMenuItem::SliderIntFocus( ) {
	auto& value{ *reinterpret_cast< int* >( m_pValue ) };
	auto& args{ *reinterpret_cast< SliderArgs_t* >( m_pArgs ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( args.m_iMaxGroups + 1 ) ) / args.m_iMaxGroups - PADDING * 4, ITEM_HEIGHT ) };
	const auto pos{ Menu::m_pFocusItem.m_vecDrawPos - Vector2D{ 0, 7 * Menu::m_pFocusItem.m_flFocusAnim } };

	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );

	const auto sliderRatio{ Menu::mapNumber( args.m_flValue, args.m_flMin, args.m_flMax, 0.f, 1.f ) };

	Render::Text( Fonts::Menu, pos - Vector2D{ 0, 16 }, Color( 255, 255, 255 ), FONT_LEFT, m_szName );

	Render::FilledRoundedBox( pos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
	Render::FilledRoundedBox( pos, size, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( pos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

	Render::FilledRoundedBox( pos + 2, Vector2D( std::max( std::ceil( ( size.x - 4 ) * sliderRatio ), 8.f ), size.y - 4 ), 4, 4, this->m_cColor );

	Render::Text( Fonts::Menu, pos + Vector2D( size.x / 2, -1 ), Color( 255, 255, 255 ), FONT_CENTER, std::to_string( static_cast< int >( value ) ).c_str( ) );
}

void CMenuItem::SliderInt( ) {
	auto& args{ *reinterpret_cast< SliderArgs_t* >( m_pArgs ) };

	if ( !m_fnShow( ) ) {
		args.m_flValue = 0.f;
		return;
	}

	auto& value{ *reinterpret_cast< int* >( m_pValue ) };

	const auto size{ Vector2D( ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * ( args.m_iMaxGroups + 1 ) ) / args.m_iMaxGroups - PADDING * 4, ITEM_HEIGHT ) };

	value = std::clamp< int >( value, args.m_flMin, args.m_flMax );

	if ( Menu::m_pFocusItem.m_pItem != this )
		Render::Text( Fonts::Menu, Menu::m_vecDrawPos, Color( 255, 255, 255 ), FONT_LEFT, m_szName );

	Menu::m_vecDrawPos.y += 16;
	const auto hovered{ Inputsys::hovered( Menu::m_vecDrawPos, size ) };

	args.m_flValue = Math::Interpolate( args.m_flValue, value, ANIMATION_SPEED );

	Menu::LerpToCol( this->m_cColor, Menu::m_pFocusItem.m_pItem == this && !Menu::m_pFocusItem.m_bFadeOut ? ACCENT : hovered ? HOVERED_ELEMENT : DIM_ELEMENT );

	if ( Menu::m_pFocusItem.m_pItem == this ) {
		if ( !Menu::m_pFocusItem.m_bFadeOut ) {
			const auto offset{ std::clamp( Vector2D( Inputsys::MousePos - Menu::m_vecDrawPos ).x, 0.f, size.x ) };
			value = Menu::mapNumber( offset, 0, size.x, args.m_flMin, args.m_flMax );
		}

		if ( Inputsys::released( VK_LBUTTON ) ) {
			Menu::m_pFocusItem.m_bFrameAfterFocus = true;
			Menu::m_pFocusItem.m_bFadeOut = true;
		}

	}
	else {
		if ( !Menu::m_pFocusItem.m_pItem && hovered && !Menu::m_pFocusItem.m_bFrameAfterFocus && !Menu::m_pFocusItem.m_pItem ) {
			if ( Inputsys::pressed( VK_LBUTTON ) )
				Menu::m_pFocusItem.Update( this );
		}

		const auto sliderRatio{ Menu::mapNumber( args.m_flValue, args.m_flMin, args.m_flMax, 0.f, 1.f ) };

		Render::FilledRoundedBox( Menu::m_vecDrawPos - Vector2D( 1, 1 ), size + 2, 4, 4, OUTLINE_DARK );
		Render::FilledRoundedBox( Menu::m_vecDrawPos, size, 4, 4, OUTLINE_LIGHT );
		Render::FilledRoundedBox( Menu::m_vecDrawPos + 1, size - Vector2D( 2, 2 ), 4, 4, OUTLINE_DARK );

		Render::FilledRoundedBox( Menu::m_vecDrawPos + 2, Vector2D( std::max( std::ceil( ( size.x - 4 ) * sliderRatio ), 8.f ), size.y - 4 ), 4, 4, this->m_cColor );

		Render::Text( Fonts::Menu, Menu::m_vecDrawPos + Vector2D( size.x / 2, -1 ), Color( 255, 255, 255 ), FONT_CENTER, std::to_string( static_cast< int >( value ) ).c_str( ) );
	}

	Menu::m_vecDrawPos.y += ITEM_HEIGHT + PADDING;
}