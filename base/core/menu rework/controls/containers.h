#pragma once
#include <vector>
#include "argument_types.h"

class CMenuGroup;

class CMenuItem {
public:
	CMenuItem( const char* name, float* value, float min, float max, int width, std::function<bool( )> show )
		: m_szName( name ), m_pValue( value ), m_eItemType( SLIDER_FLOAT ), m_fnShow( show ) {
		m_pArgs = new SliderArgs_t( min, max, width );
	};

	CMenuItem( const char* name, int* value, float min, float max, int width, std::function<bool( )> show )
		: m_szName( name ), m_pValue( value ), m_eItemType( SLIDER_INT ), m_fnShow( show ) {
		m_pArgs = new SliderArgs_t( min, max, width );
	};	
	
	CMenuItem( const char* name, int* value, int size, ComboElement_t* args, std::function<bool( )> show )
		: m_szName( name ), m_pValue( value ), m_eItemType( COMBO ), m_fnShow( show ) {
		m_pArgs = new ComboArgs_t( size, args );
	};

	CMenuItem( const char* name, int size, MultiElement_t* args, std::function<bool( )> show )
		: m_szName( name ), m_eItemType( MULTI_COMBO ), m_fnShow( show ) {
		m_pArgs = new MultiComboArgs_t( size, args );
	};

	CMenuItem( Color* value, bool alpha, std::function<bool( )> show )
		: m_pValue( value ), m_eItemType( COLOR_PICKER ), m_fnShow( show ) {
		m_pArgs = new ColorPickerArgs_t( alpha );
	};

	CMenuItem( const char* name, void* callback, int width, std::function<bool( )> show )
		: m_szName( name ), m_eItemType( BUTTON ), m_fnShow( show ) {
			m_pArgs = new ButtonArgs_t( name, callback, width );
	};

	CMenuItem( const char* name, bool fuck, std::function<bool( )> show )
		: m_szName( name ),  m_eItemType( LABEL ), m_fnShow( show ) {
	};

	CMenuItem( const char* name, bool* value, std::function<bool( )> show )
		: m_szName( name ), m_pValue( value ),  m_eItemType( CHECKBOX ), m_fnShow( show ) { };

	CMenuItem( keybind_t* value, std::function<bool( )> show )
		: m_pValue( value ), m_eItemType( KEY_BIND ), m_fnShow( show ) { };

	CMenuItem( std::string* value, int width, std::function<bool( )> show )
		: m_pValue( value ), m_eItemType( TEXT_INPUT ), m_fnShow( show ) {
		m_pArgs = new int;
		*reinterpret_cast< int* > ( m_pArgs ) = width;
	};

	CMenuItem( int* value )
		: m_pValue( value ), m_eItemType( LIST_BOX ) {
		m_pArgs = new float;
	};

	// TODO: stop it from deleting when it's a temp item (emplace_back)
	//~CMenuItem( ) {
	//	if ( m_pArgs )
	//		delete m_pArgs;
	//}

	void* m_pValue{ };
	EItemType m_eItemType{ };

	void* m_pArgs{ };

	//float m_flAnimation{ };
	Color m_cColor{ };
	const char* m_szName{ };

	std::function<bool( )> m_fnShow;

	void Render( );
	void RenderFocus( );
private:
	void Checkbox( );
	void Label( );
	void Listbox( );
	void Button( );
	void TextInput( );
	void TextInputFocus( );
	void Keybind( );
	void KeybindFocus( );
	void ColorPicker( );
	void ColorPickerFocus( );
	void Combo( );
	void ComboFocus( );
	void MultiCombo( );
	void MultiComboFocus( );
	void SliderFloat( );
	void SliderFloatFocus( );
	void SliderInt( );
	void SliderIntFocus( );
};

class CMenuGroup {
public:
	CMenuGroup( const char* name, int maxGroupsPerLine, float percent )
		: m_flPercent( percent ), m_iParentMaxGroups( maxGroupsPerLine ), m_szName( name ) {};

	void Register( const char* name, float* value, float min, float max, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( name, value, min, max, m_iParentMaxGroups, show );
	};

	void Register( const char* name, int* value, float min, float max, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( name, value, min, max, m_iParentMaxGroups, show );
	};

	void Register( const char* name, void* callback, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( name, callback, m_iParentMaxGroups, show );
	};

	void Register( std::string* value, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( value, m_iParentMaxGroups, show );
	}

	void Register( const char* name, int* value, int size, ComboElement_t* args, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( name, value, size, args, show );
	};	
	
	void Register( const char* name, int size, MultiElement_t* args, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( name, size, args, show );
	};

	void Register( const char* name, bool* value, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( name, value, show );
	};

	void Register( keybind_t* value, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( value, show );
	};

	void Register( const char* name, bool fuck = true, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( name, fuck, show );
	};

	void Register( Color* value, bool alpha = true, std::function<bool( )> show = [ ]( ) {return true; } ) {
		m_vecItems.emplace_back( value, alpha, show );
	};

	void Register( int* value ) {
		m_vecItems.emplace_back( value );
	}

	void Render( int groupNum, std::vector < CMenuGroup >& groups );
	void Reset( );

	/*bool NewLine( ) {
		return Menu::m_vecDrawPos.x - Menu::m_vecPos.x + MARGIN * 2 + m_vecSize.x > Menu::m_vecSize.x;
	}*/

private:
	std::vector< CMenuItem > m_vecItems{ };
	int m_iParentMaxGroups{ };
	const char* m_szName{ };
	float m_flPercent;
	float m_flScroll{ };
};


class CMenuSubtab {
public:
	const char* m_szName{ };
	int m_iMaxGroupsX{ };

	std::vector < CMenuGroup > m_vecGroups{ };
	Color m_cColor{ };

	CMenuGroup* AddGroup( const char* name, float percent ) {
		return &this->m_vecGroups.emplace_back( name, m_iMaxGroupsX, percent );
	}
};

class CMenuTab {
public:
	const char* m_szName{ };
	std::vector< CMenuSubtab > m_vecSubtabs{ };
	float m_flAnimation{ };
	Color m_cColor{ };

	CMenuSubtab* m_pSelectedSubtab{ };
};