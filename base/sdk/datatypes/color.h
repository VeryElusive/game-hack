#pragma once
#include <array>

enum
{
	COLOR_R = 0,
	COLOR_G = 1,
	COLOR_B = 2,
	COLOR_A = 3
};

struct ColorRGBExp32
{
	std::uint8_t r, g, b;
	char exponent;
};

class Color
{
public:
	Color() = default;

	constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) :
		arrColor({ r, g, b, a }) { }

	constexpr Color(int r, int g, int b, int a = 255) :
		arrColor({ static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b), static_cast<std::uint8_t>(a) }) { }

	constexpr Color(float r, float g, float b, float a = 1.0f) :
		arrColor({ static_cast<std::uint8_t>(r * 255.f), static_cast<std::uint8_t>(g * 255.f), static_cast<std::uint8_t>(b * 255.f), static_cast<std::uint8_t>(a * 255.f) }) { }

	void Get(std::uint8_t& r, std::uint8_t& g, std::uint8_t& b, std::uint8_t& a) const
	{
		r = arrColor[COLOR_R];
		g = arrColor[COLOR_G];
		b = arrColor[COLOR_B];
		a = arrColor[COLOR_A];
	}

	D3DCOLOR GetD3D() const
	{
		return D3DCOLOR_ARGB(arrColor[COLOR_A], arrColor[COLOR_R], arrColor[COLOR_G], arrColor[COLOR_B]);
	}


	std::uint8_t& operator[](const std::size_t i)
	{
		return this->arrColor[i];
	}

	const std::uint8_t& operator[](const std::size_t i) const
	{
		return this->arrColor[i];
	}

	bool operator==(const Color& colSecond) const
	{
		return this->arrColor == colSecond.arrColor;
	}

	bool operator!=(const Color& colSecond) const
	{
		return !(operator==(colSecond));
	}

	Color& operator=(const Color& colFrom)
	{
		arrColor[COLOR_R] = colFrom.arrColor[COLOR_R];
		arrColor[COLOR_G] = colFrom.arrColor[COLOR_G];
		arrColor[COLOR_B] = colFrom.arrColor[COLOR_B];
		arrColor[COLOR_A] = colFrom.arrColor[COLOR_A];
		return *this;
	}

	template <std::size_t N>
	std::uint8_t Get() const
	{
		static_assert(N >= COLOR_R && N <= COLOR_A, "given index is out of range");
		return arrColor[N];
	}

	template <std::size_t N>
	Color Set(const std::uint8_t nValue) const
	{
		static_assert(N >= COLOR_R && N <= COLOR_A, "given index is out of range");

		Color colCopy = *this;
		colCopy.arrColor[N] = nValue;
		return colCopy;
	}

	__forceinline Color Lerp( Color to, float strength ) {
		return Color(
			static_cast< int >( ( to.arrColor[ 0 ] - arrColor[ 0 ] ) * strength ) + arrColor[ 0 ],
			static_cast< int >( ( to.arrColor[ 1 ] - arrColor[ 1 ] ) * strength ) + arrColor[ 1 ],
			static_cast< int >( ( to.arrColor[ 2 ] - arrColor[ 2 ] ) * strength ) + arrColor[ 2 ],
			static_cast< int >( ( to.arrColor[ 3 ] - arrColor[ 3 ] ) * strength ) + arrColor[ 3 ]
		);
	}

	template <std::size_t N>
	Color Multiplier(const float flValue) const
	{
		static_assert(N >= COLOR_R && N <= COLOR_A, "given index is out of range");

		Color colCopy = *this;
		colCopy.arrColor[N] = static_cast<std::uint8_t>(static_cast<float>(colCopy.arrColor[N]) * flValue);
		return colCopy;
	}

	template <std::size_t N>
	Color Divider(const int iValue) const
	{
		static_assert(N >= COLOR_R && N <= COLOR_A, "given index is out of range");

		Color colCopy = *this;
		colCopy.arrColor[N] /= iValue;
		return colCopy;
	}

	/* returns certain R/G/B/A float value (in: 0 - 255, out: 0.0 - 1.0) */
	template <std::size_t N>
	float Base() const
	{
		static_assert(N >= COLOR_R && N <= COLOR_A, "given index is out of range");
		return arrColor[N] / 255.f;
	}

	/* convert color to float array (in: 0 - 255, out: 0.0 - 1.0) */
	std::array<float, 3U> Base() const
	{
		std::array<float, 3U> arrBaseColor = { };
		arrBaseColor[COLOR_R] = this->Base<COLOR_R>();
		arrBaseColor[COLOR_G] = this->Base<COLOR_G>();
		arrBaseColor[COLOR_B] = this->Base<COLOR_B>();
		return arrBaseColor;
	}

	static Color FromBase3(float arrBase[3])
	{
		return Color(arrBase[0], arrBase[1], arrBase[2]);
	}

	std::array<float, 4U> BaseAlpha() const
	{
		std::array<float, 4U> arrBaseColor = { };
		arrBaseColor[COLOR_R] = this->Base<COLOR_R>();
		arrBaseColor[COLOR_G] = this->Base<COLOR_G>();
		arrBaseColor[COLOR_B] = this->Base<COLOR_B>();
		arrBaseColor[COLOR_A] = this->Base<COLOR_A>();
		return arrBaseColor;
	}

	static Color FromBase4(float arrBase[4])
	{
		return Color(arrBase[0], arrBase[1], arrBase[2], arrBase[3]);
	}

	float Hue() const
	{
		if (arrColor[COLOR_R] == arrColor[COLOR_G] && arrColor[COLOR_G] == arrColor[COLOR_B])
			return 0.f;

		const float r = this->Base<COLOR_R>();
		const float g = this->Base<COLOR_G>();
		const float b = this->Base<COLOR_B>();

		const float flMax = std::max(r, std::max(g, b)), flMin = std::min(r, std::min(g, b));

		if (flMax == flMin)
			return 0.f;

		const float flDelta = flMax - flMin;
		float flHue = 0.f;

		if (flMax == r)
			flHue = (g - b) / flDelta;
		else if (flMax == g)
			flHue = 2.f + (b - r) / flDelta;
		else if (flMax == b)
			flHue = 4.f + (r - g) / flDelta;

		flHue *= 60.f;

		if (flHue < 0.f)
			flHue += 360.f;

		return flHue / 360.f;
	}

	float Saturation() const
	{
		const float r = this->Base<COLOR_R>();
		const float g = this->Base<COLOR_G>();
		const float b = this->Base<COLOR_B>();

		const float flMax = std::max(r, std::max(g, b)), flMin = std::min(r, std::min(g, b));
		const float flDelta = flMax - flMin;

		if (flMax == 0.f)
			return flDelta;

		return flDelta / flMax;
	}

	float Brightness() const
	{
		const float r = this->Base<COLOR_R>();
		const float g = this->Base<COLOR_G>();
		const float b = this->Base<COLOR_B>();

		return std::max(r, std::max(g, b));
	}

	/* return RGB color converted from HSB/HSV color */
	static Color FromHSB(float flHue, float flSaturation, float flBrightness, float flAlpha = 1.0f)
	{
		const float h = std::fmodf(flHue, 1.0f) / (60.0f / 360.0f);
		const int i = static_cast<int>(h);
		const float f = h - static_cast<float>(i);
		const float p = flBrightness * (1.0f - flSaturation);
		const float q = flBrightness * (1.0f - flSaturation * f);
		const float t = flBrightness * (1.0f - flSaturation * (1.0f - f));

		float r = 0.0f, g = 0.0f, b = 0.0f;

		switch (i)
		{
		case 0:
			r = flBrightness, g = t, b = p;
			break;
		case 1:
			r = q, g = flBrightness, b = p;
			break;
		case 2:
			r = p, g = flBrightness, b = t;
			break;
		case 3:
			r = p, g = q, b = flBrightness;
			break;
		case 4:
			r = t, g = p, b = flBrightness;
			break;
		case 5:
		default:
			r = flBrightness, g = p, b = q;
			break;
		}

		return Color(r, g, b, flAlpha);
	}

private:
	std::array<std::uint8_t, 4U> arrColor;
};

class FloatColor {
public:
	FloatColor( ) = default;
	FloatColor( float _r, float _g, float _b, float _a = 1.0f ) :
		r( _r ), g( _g ), b( _b ), a( _a ) {
	}

	FloatColor( float col[ 4 ] ) :
		r( col[ 0 ] ), g( col[ 1 ] ), b( col[ 2 ] ), a( col[ 3 ] ) {
	}

	FloatColor( int _r, int _g, int _b, int _a = 255 ) { SetColor( _r, _g, _b, _a ); }

	FloatColor Lerp( const FloatColor& c, float t ) const;

	Color ToRegularColor( ) {
		return Color( r * 255.f, g * 255.f, b * 255.f, a * 255.f );
	}

	void SetColor( float _r, float _g, float _b, float _a = 1.0f ) {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}

	void SetColor( Color clr );

	void SetColor( int _r, int _g, int _b, int _a = 255 ) {
		r = static_cast< float >( _r ) / 255.0f;
		g = static_cast< float >( _g ) / 255.0f;
		b = static_cast< float >( _b ) / 255.0f;
		a = static_cast< float >( _a ) / 255.0f;
	}

	FloatColor Alpha( float alpha, bool adaptive = false ) {
		float prev_alpha = a;
		float alpha_internal = alpha;
		alpha_internal = alpha;
		if ( adaptive )
			alpha_internal *= ( float )prev_alpha;


		return FloatColor( r, g, b, alpha_internal );
	}

	uint32_t Hex( ) const {
		union {
			uint32_t i;
			struct {
				uint8_t bytes[ 4 ];
			};
		} conv;

		conv.bytes[ 0 ] = static_cast< int >( r * 255.0f );
		conv.bytes[ 1 ] = static_cast< int >( g * 255.0f );
		conv.bytes[ 2 ] = static_cast< int >( b * 255.0f );
		conv.bytes[ 3 ] = static_cast< int >( a * 255.0f );

		return conv.i;
	};

	bool operator==( const FloatColor& clr ) const {
		return clr.r == r && clr.g == g && clr.b == b && clr.a == a;
	};

	FloatColor( const Color& f ) {
		r = f[0] / 255;  g = f[1] / 255; b = f[2] / 255; a = f[3] / 255;
	}
	operator Color( ) const { return Color( r, g, b, a ); }

	bool operator!=( const FloatColor& clr ) const {
		return clr.r != r || clr.g != g || clr.b != b || clr.a != a;
	};

	FloatColor operator*( float v ) const {
		return FloatColor( r * v, g * v, b * v, a );
	}

	operator uint32_t( ) const { return Hex( ); };

	operator float* ( ) { return &r; };

	float r, g, b, a;

	static FloatColor Black;
	static FloatColor White;
	static FloatColor Gray;
};