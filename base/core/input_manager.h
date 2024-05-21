#pragma once
#include "../sdk/datatypes/vector.h"
#include "../havoc.h"

enum EKeyMode {
	AlwaysOn,
	Toggle,
	Hold,
	Off
};

struct keybind_t{
	std::int32_t key{ 0 };
	bool enabled{ };
	bool prevState{ };
	int mode{ EKeyMode::Hold };

	void Resolve( ) {

		switch ( this->mode ) {
		case EKeyMode::AlwaysOn:
			this->enabled = true;
			break;
		case EKeyMode::Toggle: {
			bool cock = false;
			cock = GetAsyncKeyState( this->key );
			if ( cock && !this->prevState )
				this->enabled = !this->enabled;

			this->prevState = cock;

			this->enabled;
			break;
		}
		case EKeyMode::Hold:
			this->enabled = GetAsyncKeyState( this->key );
			break;
		case EKeyMode::Off: 
			this->enabled = !GetAsyncKeyState( this->key );
			break;
		}
	}
};

namespace Inputsys {
	inline bool state[ 256 ];
	inline bool prev_state[ 256 ];

	inline Vector2D MousePos;
	inline Vector2D MouseDelta;

	inline std::vector<int> KeysToCheck;

	inline int scroll;

	void update( );
	void updateNeededKeys( );
	bool pressed( int key );
	bool down( int key );
	bool released( int key );
	bool hovered( const Vector2D& pos, const Vector2D& size );
}