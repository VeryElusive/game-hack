#include "../menu.h"
#include "../../../sdk/datatypes/color.h"
#include "../../../features/misc/logger.h"

struct hsv_t {
	hsv_t( float hue, float sat, float val ) {
		this->hue = hue;
		this->sat = sat;
		this->val = val;
	}

	hsv_t( ) {
		hue = 1.f;
		sat = 1.f;
		val = 1.f;
	}
	float hue;
	float sat;
	float val;
};

Color hsv2rgb( float hue, float sat, float val ) {
	float red{ }, grn{ }, blu{ };
	float i, f, p, q, t;
	Color result( 0, 0, 0 );

	if ( val == 0 ) {
		red = 0;
		grn = 0;
		blu = 0;
	}
	else {
		hue /= 60.f;
		i = floor( hue );
		f = hue - i;
		p = val * ( 1 - sat );
		q = val * ( 1 - ( sat * f ) );
		t = val * ( 1 - ( sat * ( 1 - f ) ) );
		if ( i == 0 ) { red = val; grn = t; blu = p; }
		else if ( i == 1 ) { red = q; grn = val; blu = p; }
		else if ( i == 2 ) { red = p; grn = val; blu = t; }
		else if ( i == 3 ) { red = p; grn = q; blu = val; }
		else if ( i == 4 ) { red = t; grn = p; blu = val; }
		else if ( i == 5 ) { red = val; grn = p; blu = q; }
	}

	result[ 0 ] = red * 255.f;
	result[ 1 ] = grn * 255.f;
	result[ 2 ] = blu * 255.f;
	result[ 3 ] = 255.f;

	return result;
}

hsv_t rgb2hsv( Color a ) {
	float red, grn, blu;
	red = ( float ) a.Get<COLOR_R>( ) / 255.f;
	grn = ( float ) a.Get<COLOR_G>( ) / 255.f;
	blu = ( float ) a.Get<COLOR_B>( ) / 255.f;
	float hue, sat, val;
	float x, f, i;
	hsv_t result{ };

	x = std::min( std::min( red, grn ), blu );
	val = std::max( std::max( red, grn ), blu );
	if ( x == val ) {
		hue = 0;
		sat = 0;
	}
	else {
		f = ( red == x ) ? grn - blu : ( ( grn == x ) ? blu - red : red - grn );
		i = ( red == x ) ? 3 : ( ( grn == x ) ? 5 : 1 );
		hue = fmod( ( i - f / ( val - x ) ) * 60, 360 );
		sat = ( ( val - x ) / val );
	}
	result.hue = hue;
	result.sat = sat;
	result.val = val;
	return result;
}


#define MAX_ALPHA( col ) col.Set<COLOR_A>( Menu::m_pFocusItem.m_flFocusAnim * col.Get<COLOR_A>( ) )
void CMenuItem::ColorPickerFocus( ) {
	auto& value{ *reinterpret_cast< Color* >( m_pValue ) };
	auto& args{ *reinterpret_cast< ColorPickerArgs_t* >( m_pArgs ) };

	constexpr auto size{ Vector2D( 200, 200 ) };

	Render::RoundedBox( Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 1, 1 ), size + Vector2D( 2, 2 ), 4, 4, MAX_ALPHA( OUTLINE_DARK ) );
	Render::RoundedBox( Menu::m_pFocusItem.m_vecDrawPos, size, 4, 4, MAX_ALPHA( OUTLINE_LIGHT ) );
	Render::FilledRoundedBox( Menu::m_pFocusItem.m_vecDrawPos + Vector2D( 1, 1 ), size - Vector2D( 2, 2 ), 4, 4, MAX_ALPHA( OUTLINE_DARK ) );

	const auto hue_draw_pos = Menu::m_pFocusItem.m_vecDrawPos + Vector2D( 170, 10 );
	for ( int i = 0; i < 150; i++ ) {
		float hue = ( ( float ) i / 150 ) * 360.f;
		Color hue_color = hsv2rgb( hue, 1.f, 1.f );

		Render::Line( hue_draw_pos + Vector2D( 0, i ), hue_draw_pos + Vector2D( 20, i ), MAX_ALPHA( hue_color ) );
	}

	Render::FilledRectangle( Vector2D{ hue_draw_pos.x - 1, hue_draw_pos.y + ( args.m_flHue / 359.f ) * 150.f }, Vector2D{ 22, 3 }, MAX_ALPHA( OUTLINE_DARK ) );
	Render::Line( Vector2D{ hue_draw_pos.x, hue_draw_pos.y + ( args.m_flHue / 359.f ) * 150.f + 1 }, Vector2D{ hue_draw_pos.x + 20, hue_draw_pos.y + ( args.m_flHue / 359.f ) * 150.f  + 1 }, MAX_ALPHA( SELECTED_ELEMENT ) );

	const auto alpha_draw_pos{ Menu::m_pFocusItem.m_vecDrawPos + Vector2D( 10, 170 ) };

	if ( args.m_bAlpha ) {
		// checker pattern part two
		for ( int i = 0; i < 2; i++ ) {
			Vector2D thing_pos = alpha_draw_pos + Vector2D( 0, 10 * i );
			Vector2D thing_size = Vector2D( 10, 10 );

			for ( int n = 0; n < 150 / 10; ++n ) {
				Color thing_color = Color( 225, 225, 225 );
				if ( ( i + n ) % 2 == 0 ) {
					thing_color = Color( 255, 255, 255 );
				}

				Render::FilledRectangle( thing_pos + Vector2D( 10 * n, 0 ), thing_size, MAX_ALPHA( thing_color ) );
			}
		}

		Render::Gradient( alpha_draw_pos.x, alpha_draw_pos.y, 150, 20, Color( 0, 0, 0, 0 ), value.Set<COLOR_A>( 255 * Menu::m_pFocusItem.m_flFocusAnim ), true );

		Render::FilledRectangle( Vector2D{ alpha_draw_pos.x + ( value.Get<COLOR_A>( ) / 255.f ) * 150.f, alpha_draw_pos.y - 1 }, Vector2D{ 3, 22 }, MAX_ALPHA( OUTLINE_DARK ) );
		Render::Line( Vector2D{ alpha_draw_pos.x + ( value.Get<COLOR_A>( ) / 255.f ) * 150.f + 1, alpha_draw_pos.y }, Vector2D{ alpha_draw_pos.x + ( value.Get<COLOR_A>( ) / 255.f ) * 150.f + 1, alpha_draw_pos.y + 20 }, MAX_ALPHA( SELECTED_ELEMENT ) );

		if ( Inputsys::hovered( alpha_draw_pos, Vector2D( 150.f, 20.f ) ) && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !( args.m_bSelectingHue || args.m_bSelectingSat ) )
			args.m_bSelectingAlpha = true;
	}

	const auto actualDrawPos{ Menu::m_pFocusItem.m_vecDrawPos + Vector2D{ 10, 10 } };

	Render::FilledRectangle( Vector2D{ hue_draw_pos.x, alpha_draw_pos.y }, Vector2D{ 20, 20 }, MAX_ALPHA( ( *reinterpret_cast< Color* >( m_pValue ) ) ) );

	// actual picker
	Render::Gradient( actualDrawPos.x, actualDrawPos.y, 150, 150, Color( 1.f, 1.f, 1.f, Menu::m_pFocusItem.m_flFocusAnim ), MAX_ALPHA( hsv2rgb( args.m_flHue, 1.f, 1.f ) ), true );

	// fucken cringe fix to cringe surface gradients
	Interfaces::Surface->DrawSetColor( Color( 0.f, 0.f, 0.f, Menu::m_pFocusItem.m_flFocusAnim ) );
	Interfaces::Surface->DrawFilledRectFade( Menu::m_pFocusItem.m_vecDrawPos.x + 10, Menu::m_pFocusItem.m_vecDrawPos.y + 10, Menu::m_pFocusItem.m_vecDrawPos.x + 160, Menu::m_pFocusItem.m_vecDrawPos.y + 160, 0, Menu::m_pFocusItem.m_flFocusAnim * 255, false );

	Render::Gradient( Menu::m_pFocusItem.m_vecDrawPos.x + 10, Menu::m_pFocusItem.m_vecDrawPos.y + 10, 150, 150, Color( 0, 0, 0, 0 ), Color( 0.f, 0.f, 0.f, Menu::m_pFocusItem.m_flFocusAnim ), false );

	// controls
	if ( Inputsys::hovered( hue_draw_pos, Vector2D( 20.f, 150.f ) ) && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !( args.m_bSelectingAlpha || args.m_bSelectingSat ) )
		args.m_bSelectingHue = true;

	if ( Inputsys::hovered( actualDrawPos, Vector2D( 150.f, 150.f ) ) && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus && !( args.m_bSelectingAlpha || args.m_bSelectingHue ) )
		args.m_bSelectingSat = true;

	if ( args.m_bSelectingHue ) {
		const auto cursorDelta{ Inputsys::MousePos - Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 11, 11 ) };
		args.m_flHue = ( std::clamp<float>( Inputsys::MousePos.y - hue_draw_pos.y, 0.f, 150.f ) / 150.f ) * 359.f;
	}

	if ( args.m_bSelectingAlpha ) {
		const auto cursorDelta{ Inputsys::MousePos - Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 11, 11 ) };
		value = value.Set<COLOR_A>( 255 * std::clamp<float>( cursorDelta.x / 150.f, 0.f, 1.f ) );
	}

	if ( args.m_bSelectingSat ) {
		const auto cursorDelta{ Inputsys::MousePos - Menu::m_pFocusItem.m_vecDrawPos - Vector2D( 11, 11 ) };

		args.m_flSat = std::clamp<float>( cursorDelta.x / 150.f, 0.f, 1.f );
		args.m_flVal = std::clamp<float>( 1.f - cursorDelta.y / 150.f, 0.f, 1.f );
	}

	if ( Inputsys::released( VK_LBUTTON ) ) {
		args.m_bSelectingSat = false;
		args.m_bSelectingHue = false;
		args.m_bSelectingAlpha = false;
	}

	const auto alpha = value.Get<COLOR_A>( );
	value = hsv2rgb( args.m_flHue, args.m_flSat, args.m_flVal ).Set<COLOR_A>( alpha );

	const auto hovered{ Inputsys::hovered( Menu::m_pFocusItem.m_vecDrawPos, size ) };
	if ( !hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_bFrameAfterFocus ) {
		Menu::m_pFocusItem.m_bFrameAfterFocus = true;
		Menu::m_pFocusItem.m_bFadeOut = true;
	}
}

#undef MAX_ALPHA

void CMenuItem::ColorPicker( ) {
	if ( !m_fnShow( ) )
		return;

	auto& value{ *reinterpret_cast< Color* >( m_pValue ) };
	auto& args{ *reinterpret_cast< ColorPickerArgs_t* >( m_pArgs ) };

	const auto pos{ Menu::m_vecDrawPos + Vector2D{ ( Menu::m_vecSize.x - BAR_SIZE - MARGIN * 3 ) / 2 - PADDING * 4 - ITEM_HEIGHT * 1.5f, -ITEM_HEIGHT - PADDING } };
	const auto size{ Vector2D{ ITEM_HEIGHT * 1.5f, ITEM_HEIGHT } };

	const auto hovered{ Inputsys::hovered( pos, size ) };
	if ( hovered && Inputsys::pressed( VK_LBUTTON ) && !Menu::m_pFocusItem.m_pItem && !Menu::m_pFocusItem.m_bFrameAfterFocus ) {
		const auto backup{ Menu::m_vecDrawPos };
		Menu::m_vecDrawPos = pos + Vector2D{ size.x + PADDING, 0 };
		Menu::m_pFocusItem.Update( this );
		Menu::m_vecDrawPos = backup;

		const auto tmp{ rgb2hsv( value ) };
		args.m_flHue = tmp.hue;
		args.m_flSat = tmp.sat;
		args.m_flVal = tmp.val;
	}

	Render::FilledRoundedBox( pos - Vector2D{ 1.f, 1.f }, size + Vector2D{ 2.f, 2.f }, 4, 4, OUTLINE_LIGHT );
	Render::FilledRoundedBox( pos, size, 4, 4, value );
}