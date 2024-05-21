#include "visuals.h"
#include "../animations/animation.h"

void CPlayerESP::Main( CBasePlayer* ent ) {
	int type{ ENEMY };
	if ( ent == ctx.m_pLocal )
		type = LOCAL;
	else if ( ent->IsTeammate( ) ) 
		type = TEAM;

	if ( ent->IsDead( ) || ent->m_iHealth( ) <= 0 )
		return;

	CheckPlayerBoolFig( type, VisEnable )

	auto& entry = Entries.at( ent->Index( ) - 1 );
	entry.type = type;
	entry.ent = ent;

	// reset stored health
	if ( entry.health < std::min( ent->m_iHealth( ), 100 ) )
		entry.health = std::min( ent->m_iHealth( ), 100 );

	// animation
	if ( entry.health > ent->m_iHealth( ) )
		entry.health -= 6.f * Interfaces::Globals->flFrameTime * ( entry.health - ent->m_iHealth( ) );

	if ( ent->Dormant( ) ) {
		entry.Alpha -= Interfaces::Globals->flFrameTime / 5.f;
		entry.DormancyFade += Interfaces::Globals->flFrameTime;
	}
	else {
		entry.Alpha += Interfaces::Globals->flFrameTime * 2.f;
		entry.DormancyFade -= Interfaces::Globals->flFrameTime * 2.f;
	}

	entry.Alpha = std::clamp<float>( entry.Alpha, 0.f, 1.f );
	entry.DormancyFade = std::clamp<float>( entry.DormancyFade, 0.f, 1.f );

	DrawOOF( entry );

	if ( !GetBBox( ent, entry.BBox ) )
		return;

	DrawBox( entry );
	DrawHealth( entry );
	DrawName( entry );
	DrawSkeleton( entry );
	DrawFlags( entry );
	DrawWeapon( entry, DrawAmmo( entry ) );

	if ( entry.m_iNadeDamage > 0 )
		Render::Text( Fonts::HealthESP, entry.BBox.x + entry.BBox.w / 2, entry.BBox.y - 23, Color( 255, 255, 255, static_cast<int>( 255 * entry.Alpha ) ), FONT_CENTER, std::to_string( entry.m_iNadeDamage ).c_str( ) );

	entry.m_iNadeDamage = 0;
}

void CPlayerESP::DrawBox( VisualPlayerEntry& entry ) {
	CheckPlayerBoolFig( entry.type, VisBox );

	Color last{};
	GetPlayerColorFig( entry.type, VisBoxCol, last );

	last = last.Set<COLOR_A>( last.Get<COLOR_A>( ) * entry.Alpha );
	last = last.Lerp( DormantCol.Set<COLOR_A>( last.Get<COLOR_A>( ) * 0.4f ), entry.DormancyFade );

	const auto outline{ Color( 0, 0, 0, static_cast< int >( last.Get<COLOR_A>( ) ) ) };

	Render::Rectangle( entry.BBox.x - 1, entry.BBox.y - 1, entry.BBox.w + 2, entry.BBox.h + 2, outline );
	Render::Rectangle( entry.BBox.x + 1, entry.BBox.y + 1,  entry.BBox.w - 2, entry.BBox.h - 2, outline );
	Render::Rectangle( entry.BBox.x, entry.BBox.y,  entry.BBox.w, entry.BBox.h, last );
}

void CPlayerESP::DrawHealth( VisualPlayerEntry& entry ) {
	CheckPlayerBoolFig( entry.type, VisHealth );

	Color last;
	GetPlayerColorFig( entry.type, VisHealthCol, last );

	switch ( entry.type ) {
		case 0:
			if ( Config::Get<bool>( Vars.VisHealthOverrideLocal ) )
				break;
		case 1:
			if ( Config::Get<bool>( Vars.VisHealthOverrideTeam ) )
				break;
		case 2:
			if ( Config::Get<bool>( Vars.VisHealthOverrideEnemy ) )
				break;

		const int green{ static_cast<int>( static_cast< float >( std::min( entry.health, 100 ) ) * 2.55f ) };
		const int red{ 255 - green };
		last = Color( red, green, 0 );
	}

	last = last.Set<COLOR_A>( last.Get<COLOR_A>( ) * entry.Alpha );
	last = last.Lerp( DormantCol.Set<COLOR_A>( last.Get<COLOR_A>( ) * 0.4f ), entry.DormancyFade );

	const auto outline{ Color( 0, 0, 0, static_cast< int >( last.Get<COLOR_A>( ) ) ) };

	const auto bar_height{ static_cast<int>( static_cast< float >( entry.health ) * static_cast< float >( entry.BBox.h ) / 100.0f ) };
	const auto offset{ entry.BBox.h - bar_height };

	Render::FilledRectangle( entry.BBox.x - 6, entry.BBox.y - 1, 4, entry.BBox.h + 2, outline.Set<COLOR_A>( outline.Get<COLOR_A>( ) * .5f ) );
	Render::FilledRectangle( entry.BBox.x - 5, entry.BBox.y + offset, 2, bar_height, last );

	Render::Rectangle( entry.BBox.x - 6, entry.BBox.y - 1, 4, entry.BBox.h + 2, outline );
	//Render::Text( Fonts::HealthESP, entry.BBox.x + entry.BBox.w + 15, entry.BBox.y, Color( 255, 255, 255 ).Set<COLOR_A>( last.Get<COLOR_A>( ) ), FONT_RIGHT, std::to_string( Features::AnimSys.m_arrEntries.at( entry.ent->Index( ) - 1 ).m_iLastChoked ).c_str( ) );

	if ( entry.health <= 92 )
		Render::Text( Fonts::HealthESP, entry.BBox.x - 5, entry.BBox.y + offset - 3, Color( 255, 255, 255 ).Set<COLOR_A>( last.Get<COLOR_A>( ) ), FONT_CENTER, std::to_string( entry.health ).c_str( ) );
}

bool CPlayerESP::DrawAmmo( VisualPlayerEntry& entry ) {
	switch ( entry.type ) {
		case 0: if ( !Config::Get<bool>( Vars.VisAmmoLocal ) ) return false; break;
		case 1: if ( !Config::Get<bool>( Vars.VisAmmoTeam ) ) return false; break;
		case 2: if ( !Config::Get<bool>( Vars.VisAmmoEnemy ) ) return false; break;
	}

	const auto weapon = entry.ent->GetWeapon( );
	if ( !weapon )
		return false;

	if ( weapon->m_iItemDefinitionIndex( ) == WEAPON_C4 || weapon->m_iItemDefinitionIndex( ) == WEAPON_HEALTHSHOT || weapon->IsGrenade( ) || weapon->IsKnife( ) )
		return false;

	const auto weapon_info = weapon->GetCSWeaponData( );
	if ( !weapon_info )
		return false;

	const auto ammo = weapon->m_iClip1( );
	const auto max_clip = weapon_info->iMaxClip1;

	Color last;
	GetPlayerColorFig( entry.type, VisAmmoCol, last )

	last = last.Set<COLOR_A>( last.Get<COLOR_A>( ) * entry.Alpha );
	last = last.Lerp( DormantCol.Set<COLOR_A>( last.Get<COLOR_A>( ) * 0.4f ), entry.DormancyFade );

	const auto outline = Color( 0, 0, 0, static_cast< int >( last.Get<COLOR_A>( ) ) );

	// outline
	Render::FilledRectangle( entry.BBox.x - 1, entry.BBox.y + entry.BBox.h + 2,  entry.BBox.w + 2, 4, outline.Set<COLOR_A>( outline.Get<COLOR_A>( ) * .5f ) );
	// color
	if ( ammo )
		Render::FilledRectangle( entry.BBox.x, entry.BBox.y + entry.BBox.h + 3,  std::min( entry.BBox.w, ammo * entry.BBox.w / max_clip ), 2, last );

	Render::Rectangle( entry.BBox.x - 1, entry.BBox.y + entry.BBox.h + 2, entry.BBox.w + 2, 4, outline );

	return true;
}

void CPlayerESP::DrawName( VisualPlayerEntry& entry ) {
	CheckPlayerBoolFig( entry.type, VisName )

	Color last;
	GetPlayerColorFig( entry.type, VisNameCol, last )

	last = last.Set<COLOR_A>( last.Get<COLOR_A>( ) * entry.Alpha );
	last = last.Lerp( DormantCol.Set<COLOR_A>( last.Get<COLOR_A>( ) * 0.4f ), entry.DormancyFade );

	static auto sanitize = [ ]( char* name ) -> std::string {
		name[ 127 ] = '\0';

		std::string tmp( name );

		if ( tmp.length( ) > 20 ) {
			tmp.erase( 20, tmp.length( ) - 20 );
			tmp.append( "..." );
		}

		return tmp;
	};

	auto player_info = Interfaces::Engine->GetPlayerInfo( entry.ent->Index( ) );
	if ( !player_info.has_value() )
		return;

	const auto name = sanitize( player_info.value( ).szName );

	Render::Text( Fonts::NameESP, entry.BBox.x + entry.BBox.w / 2, entry.BBox.y - 13, last, FONT_CENTER, name.c_str( ) );
}

void CPlayerESP::DrawWeapon( VisualPlayerEntry& entry, bool AmmoBar ) {
	switch ( entry.type ) {
		case 0: if ( !Config::Get<bool>( Vars.VisWeapIconLocal ) && !Config::Get<bool>( Vars.VisWeapTextLocal ) ) return; break;
		case 1: if ( !Config::Get<bool>( Vars.VisWeapIconTeam ) && !Config::Get<bool>( Vars.VisWeapTextTeam ) ) return; break;
		case 2: if ( !Config::Get<bool>( Vars.VisWeapIconEnemy ) && !Config::Get<bool>( Vars.VisWeapTextEnemy ) ) return; break;
	}

	const auto weapon = entry.ent->GetWeapon( );
	if ( !weapon )
		return;

	Color last;
	GetPlayerColorFig( entry.type, VisWeapCol, last )

	last = last.Set<COLOR_A>( last.Get<COLOR_A>( ) * entry.Alpha );
	last = last.Lerp( DormantCol.Set<COLOR_A>( last.Get<COLOR_A>( ) * 0.4f ), entry.DormancyFade );

	int append{ 2 };
	if ( AmmoBar )
		append += 6;
	CheckIfPlayer( VisWeapText, entry.type ) {
		Render::Text( Fonts::WeaponIcon, entry.BBox.x + entry.BBox.w / 2, entry.BBox.y + entry.BBox.h + append, last.Set<COLOR_A>( last.Get<COLOR_A>( ) * .9f ), FONT_CENTER, weapon->GetIcon( ).c_str( ) );

		append += 12;
	}

	CheckIfPlayer( VisWeapIcon, entry.type ) {
		Render::Text( Fonts::HealthESP, entry.BBox.x + entry.BBox.w / 2, entry.BBox.y + entry.BBox.h + append, last, FONT_CENTER, weapon->GetGunName( ).c_str( ) );
	}
}

void CPlayerESP::DrawFlags( VisualPlayerEntry& entry ) {
	if ( entry.ent->Dormant( ) )
		return;

	std::vector<std::pair< std::string, Color>> flags{ };
	CheckIfPlayer( VisFlagDefusing, entry.type ) {
		if ( entry.ent->m_bIsDefusing( ) )
			flags.push_back( std::make_pair( _( "DEFUSING" ), Color( 255, 90, 71 ) ) );
	}

	CheckIfPlayer( VisFlagBLC, entry.type ) {
		if ( Features::AnimSys.m_arrEntries.at( entry.ent->Index( ) - 1 ).m_bBrokeLC )
			flags.push_back( std::make_pair( _( "LC" ), Color( 255, 132, 117 ) ) );
	}

	CheckIfPlayer( VisFlagBLC, entry.type ) {
		if ( !Features::AnimSys.m_arrEntries.at( entry.ent->Index( ) - 1 ).m_bRecordAdded )
			flags.push_back( std::make_pair( _( "EXPLOIT" ), Color( 255, 158, 97 ) ) );
	}

	CheckIfPlayer( VisFlagC4, entry.type ) {
		if ( entry.ent->Index( ) == ctx.m_iBombCarrier )
			flags.push_back( std::make_pair( _( "C4" ), Color( 255, 255, 255 ) ) );
	}

	CheckIfPlayer( VisFlagArmor, entry.type ) {
		if ( entry.ent->m_ArmorValue( ) > 0 )
			flags.push_back( std::make_pair( entry.ent->m_bHasHelmet( ) ? _( "HK" ) : _( "K" ), Color( 255, 255, 255 ) ) );
	}	
	
	CheckIfPlayer( VisFlagFlash, entry.type ) {
		if ( entry.ent->m_flFlashDuration( ) > 1.f )
			flags.push_back( std::make_pair( _( "FLASH" ), Color( 255, 255, 255 ) ) );
	}	
	
	CheckIfPlayer( VisFlagReload, entry.type ) {
		const auto& layer{ entry.ent->m_AnimationLayers( )[ 1 ] };

		if ( entry.ent->GetSequenceActivity( layer.nSequence ) == 967u && layer.flWeight != 0.f )
			flags.push_back( std::make_pair( _( "RELOAD" ), Color( 255, 255, 255 ) ) );
	}	
	
	CheckIfPlayer( VisFlagScoped, entry.type ) {
		if ( entry.ent->m_bIsScoped( ) )
			flags.push_back( std::make_pair( _( "SCOPED" ), Color( 255, 255, 255 ) ) );
	}

	int i{ };
	for ( const auto& flag : flags ) {
		Render::Text( Fonts::HealthESP, entry.BBox.x + entry.BBox.w + 2, entry.BBox.y + 8 * i, flag.second.Set<COLOR_A>( static_cast< int >( 200 * entry.Alpha ) ), 0, flag.first.c_str( ) );

		++i;
	}
}

void CPlayerESP::DrawSkeleton( VisualPlayerEntry& entry ) {
	CheckPlayerBoolFig( entry.type, VisSkeleton )

	if ( entry.ent->IsDormant( ) )
		return;

	const auto hdr{ entry.ent->m_pStudioHdr( )->pStudioHdr };
	if ( !hdr )
		return;

	Color last;
	GetPlayerColorFig( entry.type, VisSkeletonCol, last )

	last = last.Set<COLOR_A>( last.Get<COLOR_A>( ) * entry.Alpha );
	last = last.Lerp( DormantCol.Set<COLOR_A>( last.Get<COLOR_A>( ) * 0.4f ), entry.DormancyFade );

	Vector2D bone1, bone2;
	for ( size_t n{ }; n < hdr->nBones; ++n ) {
		auto* bone = hdr->GetBone( n );
		if ( !bone || !( bone->iFlags & 256 ) || bone->iParent == -1 ) {
			continue;
		}

		auto BonePos = [ & ]( int n ) -> Vector {
			return {
				entry.ent->m_CachedBoneData( ).Base( )[ n ][ 0 ][ 3 ],
				entry.ent->m_CachedBoneData( ).Base( )[ n ][ 1 ][ 3 ],
				entry.ent->m_CachedBoneData( ).Base( )[ n ][ 2 ][ 3 ]
			};
		};

		if ( !Math::WorldToScreen( BonePos( n ), bone1 ) || !Math::WorldToScreen( BonePos( bone->iParent ), bone2 ) )
			continue;

		Render::Line( bone1, bone2, last );
	}
}

void RotateTriangle( std::array<Vector2D, 3>& points, float rotation ) {
	const auto pointsCenter = ( points.at( 0 ) + points.at( 1 ) + points.at( 2 ) ) / 3;
	for ( auto& point : points ) {
		point -= pointsCenter;

		const auto tempX = point.x;
		const auto tempY = point.y;

		const auto theta = DEG2RAD( rotation );
		const auto c = cos( theta );
		const auto s = sin( theta );

		point.x = tempX * c - tempY * s;
		point.y = tempX * s + tempY * c;

		point += pointsCenter;
	}
}

void CPlayerESP::DrawOOF( VisualPlayerEntry& entry ) {
	CheckPlayerBoolFig( entry.type, VisOOF );

	Color last;
	GetPlayerColorFig( entry.type, VisOOFCol, last );

	last = last.Set<COLOR_A>( last.Get<COLOR_A>( ) * entry.Alpha );
	last = last.Lerp( DormantCol.Set<COLOR_A>( last.Get<COLOR_A>( ) * 0.4f ), entry.DormancyFade );

	if ( ctx.m_pLocal->IsDead( ) )
		return;

	Vector screenPos;
	if ( Math::WorldToScreen( entry.ent->GetAbsOrigin( ), screenPos ) )
		return;

	QAngle viewAngles{ ctx.m_angOriginalViewangles };

	static int width{ static_cast<int>( ctx.m_ve2ScreenSize.x ) }, height{ static_cast< int >( ctx.m_ve2ScreenSize.y ) };

	const auto screenCenter{ Vector2D( width * 0.5f, height * 0.5f ) };
	const auto angleYawRad{ DEG2RAD( viewAngles.y - Math::CalcAngle( ctx.m_vecEyePos, entry.ent->GetAbsOrigin( ) ).y - 90.0f ) };

	constexpr auto radius{ 20 };
	constexpr auto size{ 15 };

	const auto newPointX{ screenCenter.x + ( ( ( ( width - ( size * 3 ) ) * 0.5f ) * ( radius / 100.0f ) ) * cos( angleYawRad ) ) + ( int )( 6.0f * ( ( ( float )size - 4.0f ) / 16.0f ) ) };
	const auto newPointY{ screenCenter.y + ( ( ( ( height - ( size * 3 ) ) * 0.5f ) * ( radius / 100.0f ) ) * sin( angleYawRad ) ) };

	std::array <Vector2D, 3> points
	{
		Vector2D( newPointX - size, newPointY - size ),
		Vector2D( newPointX + size, newPointY ),
		Vector2D( newPointX - size, newPointY + size )
	};

	RotateTriangle( points, viewAngles.y - Math::CalcAngle( ctx.m_vecEyePos, entry.ent->GetAbsOrigin( ) ).y - 90.0f );


	Render::Triangle( points.at( 0 ), points.at( 1 ), points.at( 2 ), last.Set<COLOR_A>( last.Get<COLOR_A>( ) / 4 ) );
	Render::Line( points.at( 0 ), points.at( 1 ), last );
	Render::Line( points.at( 1 ), points.at( 2 ), last );
	Render::Line( points.at( 2 ), points.at( 0 ), last );
}

bool CPlayerESP::GetBBox( CBasePlayer* ent, rect& box ) {
	// pasted i dont care about making this func myself
	const auto min = ent->m_vecMins( );
	const auto max = ent->m_vecMaxs( );

	QAngle dir = ctx.m_angOriginalViewangles;
	Vector vF, vR, vU;
	dir.x = 0;
	dir.z = 0;

	Math::AngleVectors( dir, &vF, &vR, &vU );

	const auto zh = vU * max.z + vF * max.y + vR * min.x; // = Front left front
	const auto e = vU * max.z + vF * max.y + vR * max.x; //  = Front right front
	const auto d = vU * max.z + vF * min.y + vR * min.x; //  = Front left back
	const auto c = vU * max.z + vF * min.y + vR * max.x; //  = Front right back

	const auto g = vU * min.z + vF * max.y + vR * min.x; //  = Bottom left front
	const auto f = vU * min.z + vF * max.y + vR * max.x; //  = Bottom right front
	const auto a = vU * min.z + vF * min.y + vR * min.x; //  = Bottom left back
	const auto b = vU * min.z + vF * min.y + vR * max.x; //  = Bottom right back*-

	Vector pointList[ ] = {
		a,
		b,
		c,
		d,
		e,
		f,
		g,
		zh,
	};

	Vector2D transformed[ ARRAYSIZE( pointList ) ];

	for ( int i = 0; i < ARRAYSIZE( pointList ); i++ )
	{
		auto origin = ent->GetAbsOrigin( );

		pointList[ i ] += origin;

		if ( !Math::WorldToScreen( pointList[ i ], transformed[ i ] ) )
			return false;
	}

	float left = FLT_MAX;
	float top = -FLT_MAX;
	float right = -FLT_MAX;
	float bottom = FLT_MAX;

	for ( int i = 0; i < ARRAYSIZE( pointList ); i++ ) {
		if ( left > transformed[ i ].x )
			left = transformed[ i ].x;
		if ( top < transformed[ i ].y )
			top = transformed[ i ].y;
		if ( right < transformed[ i ].x )
			right = transformed[ i ].x;
		if ( bottom > transformed[ i ].y )
			bottom = transformed[ i ].y;
	}

	box.x = left;
	box.y = bottom;
	box.w = right - left;
	box.h = top - bottom + 5;

	return true;
}