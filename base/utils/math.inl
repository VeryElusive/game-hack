#include "math.h"

FORCEINLINE void Math::VectorAngles( const Vector& vecForward, QAngle& angView )
{
	Vector  left;
	float   len, pitch, yaw, roll;

	// get 2d length.
	len = vecForward.Length2D( );

	if ( len > 0.f ) {
		// calculate pitch and yaw.
		pitch = RAD2DEG( std::atan2( -vecForward.z, len ) );
		yaw = RAD2DEG( std::atan2( vecForward.y, vecForward.x ) );
		roll = 0.f;
	}

	else {
		pitch = float( ( vecForward.z > 0 ) ? 270 : 90 );
		yaw = 0.f;
		roll = 0.f;
	}

	// set out angles.
	angView = { pitch, yaw, roll };
}

FORCEINLINE void Math::AngleVectors( const QAngle& angView, Vector* pForward, Vector* pRight, Vector* pUp )
{
	float sp, sy, sr, cp, cy, cr;

	DirectX::XMScalarSinCos( &sp, &cp, DEG2RAD( angView.x ) );
	DirectX::XMScalarSinCos( &sy, &cy, DEG2RAD( angView.y ) );
	DirectX::XMScalarSinCos( &sr, &cr, DEG2RAD( angView.z ) );

	if ( pForward != nullptr )
	{
		pForward->x = cp * cy;
		pForward->y = cp * sy;
		pForward->z = -sp;
	}

	if ( pRight != nullptr )
	{
		pRight->x = -1 * sr * sp * cy + -1 * cr * -sy;
		pRight->y = -1 * sr * sp * sy + -1 * cr * cy;
		pRight->z = -1 * sr * cp;
	}

	if ( pUp != nullptr )
	{
		pUp->x = cr * sp * cy + -sr * -sy;
		pUp->y = cr * sp * sy + -sr * cy;
		pUp->z = cr * cp;
	}
}

FORCEINLINE void Math::AngleMatrix( const QAngle& angView, matrix3x4_t& matOutput, const Vector& vecOrigin )
{
	float sp, sy, sr, cp, cy, cr;

	DirectX::XMScalarSinCos( &sp, &cp, DEG2RAD( angView.x ) );
	DirectX::XMScalarSinCos( &sy, &cy, DEG2RAD( angView.y ) );
	DirectX::XMScalarSinCos( &sr, &cr, DEG2RAD( angView.z ) );

	matOutput.SetForward( Vector( cp * cy, cp * sy, -sp ) );

	const float crcy = cr * cy;
	const float crsy = cr * sy;
	const float srcy = sr * cy;
	const float srsy = sr * sy;

	matOutput.SetLeft( Vector( sp * srcy - crsy, sp * srsy + crcy, sr * cp ) );
	matOutput.SetUp( Vector( sp * crcy + srsy, sp * crsy - srcy, cr * cp ) );
	matOutput.SetOrigin( vecOrigin );
}

FORCEINLINE Vector2D Math::AnglePixels( const float flSensitivity, const float flPitch, const float flYaw, const QAngle& angBegin, const QAngle& angEnd )
{
	QAngle angDelta = angBegin - angEnd;
	angDelta.Normalize( );

	const float flPixelMovePitch = ( -angDelta.x ) / ( flYaw * flSensitivity );
	const float flPixelMoveYaw = ( angDelta.y ) / ( flPitch * flSensitivity );

	return Vector2D( flPixelMoveYaw, flPixelMovePitch );
}

FORCEINLINE QAngle Math::PixelsAngle( const float flSensitivity, const float flPitch, const float flYaw, const Vector2D& vecPixels )
{
	const float flAngleMovePitch = ( -vecPixels.x ) * ( flYaw * flSensitivity );
	const float flAngleMoveYaw = ( vecPixels.y ) * ( flPitch * flSensitivity );

	return QAngle( flAngleMoveYaw, flAngleMovePitch, 0.f );
}

FORCEINLINE float Math::GetFov( const QAngle& viewAngle, const QAngle& aimAngle ) {
	QAngle delta = aimAngle - viewAngle;
	delta.Normalize( );

	return std::min( sqrtf( powf( delta.x, 2.0f ) + powf( delta.y, 2.0f ) ), 180.0f );
}

FORCEINLINE QAngle Math::CalcAngle( const Vector& vecStart, const Vector& vecEnd )
{
	QAngle angView;
	const Vector vecDelta = vecEnd - vecStart;
	VectorAngles( vecDelta, angView );
	angView.Normalize( );

	return angView;
}

FORCEINLINE Vector Math::VectorTransform( const Vector& vecTransform, const matrix3x4_t& matrix )
{
	return Vector( vecTransform.DotProduct( matrix[ 0 ] ) + matrix[ 0 ][ 3 ],
		vecTransform.DotProduct( matrix[ 1 ] ) + matrix[ 1 ][ 3 ],
		vecTransform.DotProduct( matrix[ 2 ] ) + matrix[ 2 ][ 3 ] );
}

FORCEINLINE  Vector CrossProduct( const Vector& a, const Vector& b )
{
	return Vector( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}


FORCEINLINE void Math::VectorVectors( const Vector& forward, Vector& right, Vector& up )
{
	Vector tmp;

	if ( fabs( forward[ 0 ] ) < 1e-6 && fabs( forward[ 1 ] ) < 1e-6 )
	{
		// pitch 90 degrees up/down from identity
		right[ 0 ] = 0;
		right[ 1 ] = -1;
		right[ 2 ] = 0;
		up[ 0 ] = -forward[ 2 ];
		up[ 1 ] = 0;
		up[ 2 ] = 0;
	}
	else
	{
		tmp[ 0 ] = 0; tmp[ 1 ] = 0; tmp[ 2 ] = 1.0;
		right = CrossProduct( forward, tmp );
		right = right.Normalized( );
		up = CrossProduct( right, forward );
		up = up.Normalized( );
	}
}

FORCEINLINE void MatrixSetColumn( const Vector& in, int column, matrix3x4_t& out )
{
	out[ 0 ][ column ] = in.x;
	out[ 1 ][ column ] = in.y;
	out[ 2 ][ column ] = in.z;
}

FORCEINLINE  matrix3x4_t Math::VectorMatrix( const Vector& forward )
{
	matrix3x4_t matrix;
	Vector right, up;
	VectorVectors( forward, right, up );

	MatrixSetColumn( forward, 0, matrix );
	MatrixSetColumn( right * -1.f, 1, matrix );
	MatrixSetColumn( up, 2, matrix );

	return matrix;
}

FORCEINLINE void Math::VectorIRotate( const Vector& in1, const matrix3x4_t& in2, Vector& out )
{
	out.x = in1.x * in2[ 0 ][ 0 ] + in1.y * in2[ 1 ][ 0 ] + in1.z * in2[ 2 ][ 0 ];
	out.y = in1.x * in2[ 0 ][ 1 ] + in1.y * in2[ 1 ][ 1 ] + in1.z * in2[ 2 ][ 1 ];
	out.z = in1.x * in2[ 0 ][ 2 ] + in1.y * in2[ 1 ][ 2 ] + in1.z * in2[ 2 ][ 2 ];
}

FORCEINLINE void Math::VectorITransform( const Vector& in1, const matrix3x4_t& in2, Vector& out )
{
	out.x = ( in1.x - in2[ 0 ][ 3 ] ) * in2[ 0 ][ 0 ] + ( in1.y - in2[ 1 ][ 3 ] ) * in2[ 1 ][ 0 ] + ( in1.z - in2[ 2 ][ 3 ] ) * in2[ 2 ][ 0 ];
	out.y = ( in1.x - in2[ 0 ][ 3 ] ) * in2[ 0 ][ 1 ] + ( in1.y - in2[ 1 ][ 3 ] ) * in2[ 1 ][ 1 ] + ( in1.z - in2[ 2 ][ 3 ] ) * in2[ 2 ][ 1 ];
	out.z = ( in1.x - in2[ 0 ][ 3 ] ) * in2[ 0 ][ 2 ] + ( in1.y - in2[ 1 ][ 3 ] ) * in2[ 1 ][ 2 ] + ( in1.z - in2[ 2 ][ 3 ] ) * in2[ 2 ][ 2 ];
}

FORCEINLINE Vector Math::ExtrapolateTick( const Vector& p0, const Vector& v0 )
{
	// position formula: p0 + v0t
	return p0 + ( v0 * Interfaces::Globals->flIntervalPerTick );
}

typedef __declspec( align( 16 ) ) union
{
	float f[ 4 ];
	__m128 v;
} m128;

FORCEINLINE  __m128 sqrt_ps( const __m128 squared )
{
	return _mm_sqrt_ps( squared );
}

FORCEINLINE bool Math::WorldToScreen( const Vector& origin, Vector& screen )
{
	if ( !ScreenTransform( origin, screen ) )
	{
		screen.x = 0.5 * screen.x * ctx.m_ve2ScreenSize.x;
		screen.y = -0.5 * screen.y * ctx.m_ve2ScreenSize.y;
		screen.x += 0.5 * ctx.m_ve2ScreenSize.x;
		screen.y += 0.5 * ctx.m_ve2ScreenSize.y;
		return true;
	}

	return false;
}

FORCEINLINE bool Math::WorldToScreen( const Vector& in, Vector2D& out ) {
	if ( ScreenTransform( in, out ) ) {
		out.x = 0.5 * out.x * ctx.m_ve2ScreenSize.x;
		out.y = -0.5 * out.y * ctx.m_ve2ScreenSize.y;
		out.x += 0.5 * ctx.m_ve2ScreenSize.x;
		out.y += 0.5 * ctx.m_ve2ScreenSize.y;

		return true;
	}
	return false;
}

#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
#define norm(v)    sqrt(dot(v,v))  // norm = length of  vector

FORCEINLINE bool Math::IntersectionSegmentToSegment( const Vector s1, const Vector s2, const Vector k1, const Vector k2, const float radius )
{
	Vector   u = s2 - s1;
	Vector   v = k2 - k1;
	Vector   w = s1 - k1;
	float    a = dot( u, u );
	float    b = dot( u, v );
	float    c = dot( v, v );
	float    d = dot( u, w );
	float    e = dot( v, w );
	float    D = a * c - b * b;
	float    sc, sN, sD = D;
	float    tc, tN, tD = D;

	if ( D < 0.00000001f ) {
		sN = 0.0;
		sD = 1.0;
		tN = e;
		tD = c;
	}
	else {
		sN = ( b * e - c * d );
		tN = ( a * e - b * d );
		if ( sN < 0.0 ) {
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if ( sN > sD ) {
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if ( tN < 0.0 ) {
		tN = 0.0;

		if ( -d < 0.0 )
			sN = 0.0;
		else if ( -d > a )
			sN = sD;
		else {
			sN = -d;
			sD = a;
		}
	}
	else if ( tN > tD ) {
		tN = tD;

		if ( ( -d + b ) < 0.0 )
			sN = 0;
		else if ( ( -d + b ) > a )
			sN = sD;
		else {
			sN = ( -d + b );
			sD = a;
		}
	}

	sc = ( abs( sN ) < 0.00000001f ? 0.0 : sN / sD );
	tc = ( abs( tN ) < 0.00000001f ? 0.0 : tN / tD );

	Vector  dP = w + ( u * sc ) - ( v * tc );

	auto shit = norm( dP );
	//printf( "shit %f | rad %f\n", shit, radius );
	return shit < radius;
}

FORCEINLINE bool Math::LineThroughBB( const Vector& src, const Vector& dst, const Vector& min, const Vector& max ) {
	float d1{ }, d2{ }, f{ };
	auto t1 = -1.f, t2 = 1.f;

	auto start_solid = true;

	for ( std::size_t i{ }; i < 6u; ++i ) {
		if ( i >= 3 ) {
			const auto j = i - 3u;

			d1 = src[ j ] - max[ j ];
			d2 = d1 + dst[ j ];
		}
		else {
			d1 = -src[ i ] + min[ i ];
			d2 = d1 - dst[ i ];
		}

		if ( d1 > 0.0f
			&& d2 > 0.0f )
			return false;

		if ( d1 <= 0.0f
			&& d2 <= 0.0f )
			continue;

		if ( d1 > 0.f )
			start_solid = false;

		if ( d1 > d2 ) {
			f = d1;

			if ( f < 0.f )
				f = 0.f;

			f /= d1 - d2;

			if ( f > t1 )
				t1 = f;
		}
		else {
			f = d1 / ( d1 - d2 );

			if ( f < t2 )
				t2 = f;
		}
	}

	return start_solid || ( t1 < t2&& t1 >= 0.f );
}

FORCEINLINE float Math::ApproachAngle( float target, float value, float speed )
{
	target = anglemod( target );
	value = anglemod( value );

	float delta = target - value;

	// Speed is assumed to be positive
	if ( speed < 0 )
		speed = -speed;

	if ( delta < -180 )
		delta += 360;
	else if ( delta > 180 )
		delta -= 360;

	if ( delta > speed )
		value += speed;
	else if ( delta < -speed )
		value -= speed;
	else
		value = target;

	return value;
}
