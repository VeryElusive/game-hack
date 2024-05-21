#include "Render.h"

void Render::CreateFonts( ) {
	Interfaces::Surface->SetFontGlyphSet( Fonts::WeaponIcon = Interfaces::Surface->FontCreate( ), _( "undefeated" ), 12, FW_MEDIUM, NULL, NULL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	Interfaces::Surface->SetFontGlyphSet( Fonts::GrenadeWarning = Interfaces::Surface->FontCreate( ), _( "undefeated" ), 20, FW_MEDIUM, NULL, NULL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	Interfaces::Surface->SetFontGlyphSet( Fonts::NameESP = Interfaces::Surface->FontCreate( ), _( "Verdana" ), 12, FW_MEDIUM, NULL, NULL, FONTFLAG_DROPSHADOW );
	Interfaces::Surface->SetFontGlyphSet( Fonts::HealthESP = Interfaces::Surface->FontCreate( ), _( "Small Fonts" ), 8, FW_MEDIUM, NULL, NULL, FONTFLAG_OUTLINE );
	Interfaces::Surface->SetFontGlyphSet( Fonts::Menu = Interfaces::Surface->FontCreate( ), _( "Tahoma" ), 13, FW_DONTCARE, NULL, NULL, FONTFLAG_DROPSHADOW );
	Interfaces::Surface->SetFontGlyphSet( Fonts::Logs = Interfaces::Surface->FontCreate( ), _( "Tahoma" ), 11, FW_DONTCARE, NULL, NULL, NULL );
	Interfaces::Surface->SetFontGlyphSet( Fonts::MenuTabs = Interfaces::Surface->FontCreate( ), _( "test2" ), 55, FW_NORMAL, NULL, NULL, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS );
	Interfaces::Surface->SetFontGlyphSet( Fonts::DamageMarker = Interfaces::Surface->FontCreate( ), _( "Verdana" ), 14, FW_NORMAL, NULL, NULL, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS );
}

void Render::FilledRectangle( Vector2D pos, Vector2D size, Color col ) {
	if ( GlobalAlpha < 1.f )
		col = col.Set<COLOR_A>( col.Get<COLOR_A>( ) * GlobalAlpha );

	Interfaces::Surface->DrawSetColor( col );
	Interfaces::Surface->DrawFilledRect( pos.x, pos.y, pos.x + size.x, pos.y + size.y );
}

void Render::FilledRoundedBox( Vector2D pos, Vector2D size, int points, int radius, Color col ) {
	if ( GlobalAlpha < 1.f )
		col = col.Set<COLOR_A>( col.Get<COLOR_A>( ) * GlobalAlpha );

	pos.x = std::floor( pos.x );
	pos.y = std::floor( pos.y );

	Vertex_t* round = new Vertex_t[ 4 * points ];

	for ( int i = 0; i < 4; i++ ) {
		int _x = pos.x + ( ( i < 2 ) ? ( size.x - radius ) : radius );
		int _y = pos.y + ( ( i % 3 ) ? ( size.y - radius ) : radius );

		float a = 90.f * i;

		for ( int j = 0; j < points; j++ ) {
			float _a = DEG2RAD( a + ( j / ( float )( points - 1 ) ) * 90.f );

			round[ ( i * points ) + j ] = Vertex_t( Vector2D( _x + radius * sin( _a ), _y - radius * cos( _a ) ) );
		}
	}

	static int Texture = Interfaces::Surface->CreateNewTextureID( true );
	unsigned char buffer[ 4 ] = { 255, 255, 255, 255 };

	Interfaces::Surface->DrawSetTextureRGBA( Texture, buffer, 1, 1 );
	Interfaces::Surface->DrawSetColor( col );
	Interfaces::Surface->DrawSetTexture( Texture );

	Interfaces::Surface->DrawTexturedPolygon( 4 * points, round, true );

	delete[ ] round;
}

void Render::RoundedBox( Vector2D pos, Vector2D size, int points, int radius, Color col ) {
	size -= Vector2D{ 1,1 };
	if ( GlobalAlpha < 1.f )
		col = col.Set<COLOR_A>( col.Get<COLOR_A>( ) * GlobalAlpha );

	pos.x = std::floor( pos.x );
	pos.y = std::floor( pos.y );

	Vertex_t* round = new Vertex_t[ 4 * points ];

	for ( int i = 0; i < 4; i++ ) {
		int _x = pos.x + ( ( i < 2 ) ? ( size.x - radius ) : radius );
		int _y = pos.y + ( ( i % 3 ) ? ( size.y - radius ) : radius );

		float a = 90.f * i;

		for ( int j = 0; j < points; j++ ) {
			float _a = DEG2RAD( a + ( j / ( float ) ( points - 1 ) ) * 90.f );

			round[ ( i * points ) + j ] = Vertex_t( Vector2D( _x + radius * sin( _a ), _y - radius * cos( _a ) ) );
		}
	}

	static int Texture = Interfaces::Surface->CreateNewTextureID( true );
	unsigned char buffer[ 4 ] = { 255, 255, 255, 255 };

	Interfaces::Surface->DrawSetTextureRGBA( Texture, buffer, 1, 1 );
	Interfaces::Surface->DrawSetColor( col );
	Interfaces::Surface->DrawSetTexture( Texture );

	Interfaces::Surface->DrawTexturedPolyLine( round, 4 * points );

	delete[ ] round;
}

void Render::FilledRectangle( int x, int y, int w, int h, Color col ) {
	Interfaces::Surface->DrawSetColor( col );
	Interfaces::Surface->DrawFilledRect( x, y, x + w, y + h );
}

void Render::Triangle( Vector2D point_one, Vector2D point_two, Vector2D point_three, Color color )
{
	Vertex_t verts[ 3 ] = {
		Vertex_t( point_one ),
		Vertex_t( point_two ),
		Vertex_t( point_three )
	};

	static int texture = Interfaces::Surface->CreateNewTextureID( true );
	unsigned char buffer[ 4 ] = { 255, 255, 255, 255 };

	Interfaces::Surface->DrawSetTextureRGBA( texture, buffer, 1, 1 );
	Interfaces::Surface->DrawSetColor( color );
	Interfaces::Surface->DrawSetTexture( texture );

	Interfaces::Surface->DrawTexturedPolygon( 3, verts );
}

void Render::WorldCircle( Vector origin, float radius, Color color, Color colorFill ) {
	auto prevScreenPos = Vector2D( 0, 0 );
	auto step = M_PI * 2.0f / 72.0f;

	auto screenPos = Vector2D( 0, 0 );
	auto screen = Vector2D( 0, 0 );

	if ( !Math::WorldToScreen( origin, screen ) )
		return;

	for ( auto rotation = 0.0f; rotation <= M_PI * 2.0f; rotation += step ) {
		Vector pos( radius * cos( rotation ) + origin.x, radius * sin( rotation ) + origin.y, origin.z );

		if ( Math::WorldToScreen( pos, screenPos ) ) {
			if ( !prevScreenPos.IsZero( ) && prevScreenPos.IsValid( ) && screenPos.IsValid( ) && prevScreenPos != screenPos ) {
				Line( prevScreenPos, screenPos, color );
				Triangle( screen, screenPos, prevScreenPos, colorFill );
			}

			prevScreenPos = screenPos;
		}
	}
}

void Render::Gradient( int x, int y, int w, int h, Color first, Color second, bool horizontal ) {
	if ( GlobalAlpha < 1.f ) {
		first = first.Set<COLOR_A>( first.Get<COLOR_A>( ) * GlobalAlpha );
		second = second.Set<COLOR_A>( second.Get<COLOR_A>( ) * GlobalAlpha );

	}
	// GOD I WISH I HAD LEGENDWARE
	auto filled_rect_fade = [ & ]( bool reversed, float alpha ) {
		Interfaces::Surface->DrawFilledRectFade( x, y, x + w, y + h,
			reversed ? alpha : 0,
			reversed ? 0 : alpha,
			horizontal );
	};

	static auto blend = [ ]( const Color& first, const Color& second, float t ) -> Color {
		return Color(
			first.Get<COLOR_R>( ) + t * ( second.Get<COLOR_R>( ) - first.Get<COLOR_R>( ) ),
			first.Get<COLOR_G>( ) + t * ( second.Get<COLOR_G>( ) - first.Get<COLOR_G>( ) ),
			first.Get<COLOR_B>( ) + t * ( second.Get<COLOR_B>( ) - first.Get<COLOR_B>( ) ),
			first.Get<COLOR_A>( ) + t * ( second.Get<COLOR_A>( ) - first.Get<COLOR_A>( ) ) );
	};

	if ( first.Get<COLOR_A>( ) == 255 || second.Get<COLOR_A>( ) == 255 ) {
		Interfaces::Surface->DrawSetColor( blend( first, second, 0.5f ) );
		Interfaces::Surface->DrawFilledRect( x, y, x + w, y + h );
	}

	Interfaces::Surface->DrawSetColor( first );
	filled_rect_fade( true, first.Get<COLOR_A>( ) );

	Interfaces::Surface->DrawSetColor( second );
	filled_rect_fade( false, second.Get<COLOR_A>( ) );
}

void Render::Rectangle( Vector2D pos, Vector2D size, Color color ) {
	if ( GlobalAlpha < 1.f )
		color = color.Set<COLOR_A>( color.Get<COLOR_A>( ) * GlobalAlpha );

	Rectangle( pos.x, pos.y, size.x, size.y, color );
}

void Render::Rectangle( int x, int y, int w, int h, Color color ) {
	Interfaces::Surface->DrawSetColor( color );
	Interfaces::Surface->DrawOutlinedRect( x, y, x + w, y + h );
}

void Render::Line( Vector2D a, Vector2D b, Color col ) {
	if ( GlobalAlpha < 1.f )
		col = col.Set<COLOR_A>( col.Get<COLOR_A>( ) * GlobalAlpha );

	Interfaces::Surface->DrawSetColor( col );
	Interfaces::Surface->DrawLine( a.x, a.y, b.x, b.y );
}

void Render::Circle( int x, int y, int r, int s, Color col )
{
	Interfaces::Surface->DrawSetColor( col );
	Interfaces::Surface->DrawOutlinedCircle( x, y, r, s );
}

void Render::FilledCircle( Vector2D pos, float radius, Color col, int points )
{
	static bool once = true;

	static std::vector<float> temppointsx;
	static std::vector<float> temppointsy;

	if ( once )
	{
		float step = ( float ) DirectX::XM_PI * 2.0f / points;
		for ( float a = 0; a < ( DirectX::XM_PI * 2.0f ); a += step ) //-V1034
		{
			temppointsx.push_back( cosf( a ) );
			temppointsy.push_back( sinf( a ) );
		}
		once = false;
	}

	std::vector<int> pointsx;
	std::vector<int> pointsy;
	std::vector<Vertex_t> vertices;

	for ( int i = 0; i < temppointsx.size( ); i++ )
	{
		float eeks = radius * temppointsx[ i ] + pos.x;
		float why = radius * temppointsy[ i ] + pos.y;
		pointsx.push_back( eeks );
		pointsy.push_back( why );

		vertices.emplace_back( Vertex_t( Vector2D( eeks, why ) ) );
	}

	Interfaces::Surface->DrawSetColor( col );
	Interfaces::Surface->DrawTexturedPolygon( points, vertices.data( ) );
}

void Render::Text( HFont font, int x, int y, Color color, DWORD alignment, const char* msg ) {
	if ( GlobalAlpha < 1.f )
		color = color.Set<COLOR_A>( color.Get<COLOR_A>( ) * GlobalAlpha );

	va_list va_alist;
	char buf[ 1024 ];
	va_start( va_alist, msg );
	_vsnprintf_s( buf, sizeof( buf ), msg, va_alist );
	va_end( va_alist );
	wchar_t wbuf[ 1024 ];
	MultiByteToWideChar( CP_UTF8, 0, buf, 256, wbuf, 256 );

	int w, h;

	Interfaces::Surface->GetTextSize( font, wbuf, w, h );
	Interfaces::Surface->DrawSetTextFont( font );
	Interfaces::Surface->DrawSetTextColor( color );

	if ( alignment & FONT_RIGHT )
		x -= w;

	if ( alignment & FONT_CENTER )
		x -= w / 2;

	Interfaces::Surface->DrawSetTextPos( x, y );
	Interfaces::Surface->DrawPrintText( wbuf, wcslen( wbuf ) );
}

void Render::Text( HFont font, Vector2D pos, Color color, DWORD alignment, const char* msg ) {
	Text( font, pos.x, pos.y, color, alignment, msg );
}

Vector2D Render::GetTextSize( std::string string, HFont font )
{
	Vector2D rect;

	wchar_t wbuf[ 1024 ];
	if ( MultiByteToWideChar( CP_UTF8, 0, string.c_str( ), -1, wbuf, 256 ) > 0 ) {
		int x, y;
		Interfaces::Surface->GetTextSize( font, wbuf, x, y );
		rect.x = x;
		rect.y = x;
	}
	return rect;
}