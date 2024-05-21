/*#pragma once
#include "../../../context.h"

class CSkinChanger {
public:
	void OnNetworkUpdate( bool start );
	void ForceItemUpdate( CBasePlayer* local );
	void UpdateHud( );
	void PostDataUpdateStart( CBasePlayer* local );

	CBaseHandle m_hGloveHandle{ };

	bool m_bUpdateSkins{ };
	bool m_bUpdateGloves{ };
	bool m_bActive{ };

	float m_flLastSkinsUpdate{ };
	float m_flLastGlovesUpdate{ };
};

namespace Features { inline CSkinChanger SkinChanger; };*/