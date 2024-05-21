#pragma once
#include "../../context.h"

struct CompressionVars_t {
	QAngle m_aimPunchAngle{ };
	Vector m_aimPunchAngleVel{ };
	float m_vecViewOffsetZ{ };

	int m_iCommandNumber{ };
};

class CEnginePrediction {
public:
	void RunCommand( CUserCmd& cmd );
	void Finish( );

	void RestoreNetvars( int slot );
	void StoreNetvars( int slot );

	bool ModifyDatamap( );

	float Spread{ };
	float Inaccuracy{ };
	std::array< CompressionVars_t, 150> m_cCompressionVars{ };
private:
	float m_flCurtime{ };
	float m_flFrametime{ };
	//void* m_pOldWeapon{ };

	//float VelocityModifier{ };

	//bool FirstTimePrediction{ };
	//bool InPrediction{ };


	CMoveData MoveData{ };
};

namespace Features { inline CEnginePrediction EnginePrediction; };