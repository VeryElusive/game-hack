#pragma once
#include "../core/interfaces.h"
#include "../havoc.h"
#include "math.h"
#include <any>
#include <shared_mutex>

typedef unsigned long HFont;

namespace Fonts
{
	inline HFont WeaponIcon;
	inline HFont NameESP;
	inline HFont DamageMarker;
	inline HFont HealthESP;
	inline HFont Menu;
	inline HFont MenuTabs;
	inline HFont Logs;
	inline HFont GrenadeWarning;
}

struct vertice {
	float x, y, z, rhw;
	uint32_t  c;
};

namespace Render {
	inline float GlobalAlpha = 1;

	// used these on menu when i was still using my dx render
	void FilledRectangle( Vector2D pos, Vector2D size, Color col );
	void FilledRoundedBox( Vector2D pos, Vector2D size, int points, int radius, Color color );
	void RoundedBox( Vector2D pos, Vector2D size, int points, int radius, Color color );
	void Rectangle( Vector2D a, Vector2D b, Color col );
	void FilledCircle( Vector2D pos, float radius, Color col, int percent );
	void Line( Vector2D a, Vector2D b, Color col );
	void Text( HFont font, Vector2D pos, Color color, DWORD alignment, const char* msg );

	// reg
	void Text( HFont font, int x, int y, Color color, DWORD alignment, const char* msg );
	Vector2D GetTextSize( std::string string, HFont font );

	void Gradient( int x, int y, int width, int height, Color color1, Color color2, bool horizontal );
	void Rectangle( int x, int y, int w, int h, Color color );
	void Triangle( Vector2D point_one, Vector2D point_two, Vector2D point_three, Color color );
	void WorldCircle( Vector origin, float radius, Color color, Color colorFill );
	void FilledRectangle( int x, int y, int w, int h, Color col );
	void Circle( int x, int y, int r, int s, Color col );

	void CreateFonts( );
};