#pragma once

enum EItemType {
	CHECKBOX,
	SLIDER_INT,
	SLIDER_FLOAT,
	COMBO,
	MULTI_COMBO,
	KEY_BIND,
	COLOR_PICKER,
	LABEL,
	BUTTON,
	LIST_BOX,
	TEXT_INPUT,
};

struct SliderArgs_t {
	SliderArgs_t( float min, float max, int maxGroups )
		: m_flMin( min ), m_flMax( max ), m_iMaxGroups( maxGroups ) { };

	float m_flMin{ };
	float m_flMax{ };
	float m_flValue{ };

	int m_iMaxGroups{ };
};

struct ComboElement_t {
	ComboElement_t( const char* name )
		: m_szName( name ) { };

	ComboElement_t( const char* name, Color col )
		: m_szName( name ), m_cColor( col ) { };

	ComboElement_t( ) {};

	Color m_cColor{ };
	const char* m_szName{ };
};

struct ComboArgs_t {
	ComboArgs_t( int size, ComboElement_t* args )
		: m_iSize( size ), m_pArguments( args ) { };

	int m_iSize{ };
	ComboElement_t* m_pArguments{ };
};

struct MultiElement_t {
	MultiElement_t( bool* value, const char* name ) 
		: m_szName( name ), m_pValue( value ) { };

	bool* m_pValue{ };
	Color m_cColor{ };
	const char* m_szName{ };
};

struct MultiComboArgs_t {
	MultiComboArgs_t( int size, MultiElement_t* args )
		: m_iSize( size ), m_pArguments( args ) { };

	int m_iSize{ };
	MultiElement_t* m_pArguments{ };
};

struct ColorPickerArgs_t {
	ColorPickerArgs_t( bool alpha ) : m_bAlpha( alpha ) {};

	float m_flHue{ };
	float m_flSat{ };
	float m_flVal{ };

	bool m_bSelectingHue{ };
	bool m_bSelectingAlpha{ };
	bool m_bSelectingSat{ };

	bool m_bAlpha{ };
};

struct ButtonArgs_t {
	ButtonArgs_t( const char* name, void* callBack, int maxGroups )
		: m_szName( name ), m_pCallback( callBack ), m_iMaxGroups( maxGroups ) { };

	float m_flTimer{ };
	void* m_pCallback{ };
	const char* m_szName{ };
	int m_iMaxGroups{ };
	bool m_bDown{ };
};