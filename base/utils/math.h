#pragma once
// used: directx math definitions
#include <directxmath.h>

// used: winapi, directx includes
#include "../havoc.h"
// used: vector, vector2d
#include "../sdk/datatypes/vector.h"
// used: angle
#include "../sdk/datatypes/qangle.h"
// used: matrix
#include "../sdk/datatypes/matrix.h"

#include "../context.h"

#pragma region math_definitions
#define M_HPI			DirectX::XM_PIDIV2	// half pi
#define M_QPI			DirectX::XM_PIDIV4	// quarter pi
#define M_PI			DirectX::XM_PI		// pi const
#define M_2PI			DirectX::XM_2PI		// pi double
#define M_GPI			1.6180339887498f	// golden ratio
#define M_RADPI			57.295779513082f	// pi in radians

#define M_METRE2INCH( x )	( ( x ) / 0.0254f )
#define M_INCH2METRE( x )	( ( x ) * 0.0254f )
#define M_METRE2FOOT( x )	( ( x ) * 3.28f )
#define M_FOOT2METRE( x )	( ( x ) / 3.28f )
#define RAD2DEG( x )		DirectX::XMConvertToDegrees( x )
#define DEG2RAD( x )		DirectX::XMConvertToRadians( x )
#pragma endregion

extern "C" using RandomSeedFn = int(__cdecl*)(int iSeed);
extern "C" using RandomFloatFn = float(__cdecl*)(float flMinVal, float flMaxVal);
extern "C" using RandomFloatExpFn = float(__cdecl*)(float flMinVal, float flMaxVal, float flExponent);
extern "C" using RandomIntFn = int(__cdecl*)(int iMinVal, int iMaxVal);
extern "C" using RandomGaussianFloatFn = float(__cdecl*)(float flMean, float flStdDev);

/*
 * MATHEMATICS
 */
namespace Math
{
	// Get
	/* export specific functions addresses from libraries handles to get able call them */
	bool	Setup();
	bool	Stub();
	/* convert vector to angles */
	FORCEINLINE void	VectorAngles(const Vector& vecForward, QAngle& angView);
	/* convert angles to x, y, z vectors */
	FORCEINLINE void	AngleVectors(const QAngle& angView, Vector* pForward, Vector* pRight = nullptr, Vector* pUp = nullptr);
	/* convert angles to matrix */
	FORCEINLINE void	AngleMatrix(const QAngle& angView, matrix3x4_t& matOutput, const Vector& vecOrigin = Vector(0.0f, 0.0f, 0.0f));
	/* convert angle to screen pixels by sensivity, pitch and yaw */
	// @note: could be useful at mouse event aimbot
	FORCEINLINE Vector2D AnglePixels(const float flSensitivity, const float flPitch, const float flYaw, const QAngle& angBegin, const QAngle& angEnd);
	/* convert screen pixels to angles by sensivity, pitch and yaw */
	FORCEINLINE QAngle	PixelsAngle(const float flSensitivity, const float flPitch, const float flYaw, const Vector2D& vecPixels);
	/* calculate angles by source and destination vectors */
	FORCEINLINE QAngle	CalcAngle(const Vector& vecStart, const Vector& vecEnd);
	/* transform vector by matrix and return it */
	FORCEINLINE Vector	VectorTransform(const Vector& vecTransform, const matrix3x4_t& matrix);
	FORCEINLINE void VectorITransform( const Vector& in1, const matrix3x4_t& in2, Vector& out );
	/* calculate next tick position */
	FORCEINLINE Vector	ExtrapolateTick(const Vector& p0, const Vector& v0);
	/* calculate FOV from one angle to another */
	FORCEINLINE float GetFov( const QAngle& viewAngle, const QAngle& aimAngle );
	/* calculate relative 2d screen position of smth in world */
	FORCEINLINE bool WorldToScreen( const Vector& origin, Vector& screen );
	FORCEINLINE bool WorldToScreen( const Vector& origin, Vector2D& screen );
	bool ScreenTransform( const Vector& point, Vector& screen );
	bool ScreenTransform( const Vector& point, Vector2D& screen );

	FORCEINLINE bool IntersectionSegmentToSegment( const Vector s1, const Vector s2, const Vector k1, const Vector k2, const float radius );
	FORCEINLINE bool LineThroughBB( const Vector& src, const Vector& dst, const Vector& min, const Vector& max );
	FORCEINLINE matrix3x4_t VectorMatrix( const Vector& forward );
	FORCEINLINE void VectorVectors( const Vector& forward, Vector& right, Vector& up );
	FORCEINLINE void VectorIRotate( const Vector& in1, const matrix3x4_t& in2, Vector& out );

	FORCEINLINE float ApproachAngle( float target, float value, float speed );

	FORCEINLINE float anglemod( float a ) { return ( 360.f / 65536 ) * ( ( int )( a * ( 65536.f / 360.0f ) ) & 65535 ); }



	FORCEINLINE void SinCos( float radians, float& sine, float& cosine ) { sine = sin( radians ); cosine = cos( radians ); }
	FORCEINLINE float Interpolate( const float from, const float to, const float percent ) { if ( from == to ) return from; return to * percent + from * ( 1.f - percent ); }

	// clamps yaw to 0-360 rather than -180-180
	FORCEINLINE float NormalizeEyeAngles( float yaw ) {
		yaw = std::remainderf( yaw, 360.f );
		if ( yaw < 0 )
			yaw += 360.f;

		return yaw;
	}

	/* Normalize each angle and get difference of each angle (normalized) */
	FORCEINLINE float AngleDiff( float dest_angle, float src_angle ) {
		float delta; // xmm1_4

		for ( ; dest_angle > 180.f; dest_angle = dest_angle - 360.f )
			;
		for ( ; dest_angle < -180.f; dest_angle = dest_angle + 360.f )
			;
		for ( ; src_angle > 180.f; src_angle = src_angle - 360.f )
			;
		for ( ; src_angle < -180.f; src_angle = src_angle + 360.f )
			;
		for ( delta = src_angle - dest_angle; delta > 180.f; delta = delta - 360.f )
			;
		for ( ; delta < -180.f; delta = delta + 360.f )
			;
		return delta;
	}

	/* round the number */
	FORCEINLINE int round( int n, int to = 10 ) {
		// Smaller multiple
		int a = ( n / to ) * to;

		// Larger multiple
		int b = a + to;

		// Return of closest of two
		return ( n - a > b - n ) ? b : a;
	}

	/* get the mean angle */
	//https://rosettacode.org/wiki/Averages/Mean_angle if you wanna do morethan 2 angles!
	FORCEINLINE float meanAngle( float angle1, float angle2 ) {
		float y_part{ }, x_part{ };

		x_part += std::cos( angle1 * M_PI / 180 );
		y_part += std::sin( angle1 * M_PI / 180 );

		x_part += std::cos( angle2 * M_PI / 180 );
		y_part += std::sin( angle2 * M_PI / 180 );

		return std::atan2( y_part / 2.f, x_part / 2.f ) * 180.f / M_PI;
	}

	inline float Approach( float target, float value, float speed ) {
		float delta = target - value;

		if ( delta > speed )
			value += speed;
		else if ( delta < -speed )
			value -= speed;
		else
			value = target;

		return value;
	}


	template <class T>
	__forceinline T Lerp( float flPercent, T const& A, T const& B ) {
		return A + ( B - A ) * flPercent;
	}

	// Exports
	inline RandomSeedFn				RandomSeed;
	inline RandomFloatFn			RandomFloat;
	inline RandomFloatExpFn			RandomFloatExp;
	inline RandomIntFn				RandomInt;
	inline RandomGaussianFloatFn	RandomGaussianFloat;
}

#include "math.inl"