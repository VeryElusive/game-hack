#include "visuals.h"

void CBulletTracers::Draw( ) {
	if ( m_vecTracers.empty( ) )
		return;

	m_vecTracers.erase(
		std::remove_if(
			m_vecTracers.begin( ), m_vecTracers.end( ),
			[ & ]( const TracerData_t& tracer ) -> bool {
				return Interfaces::Globals->flRealTime >= tracer.m_flStartTime + 5.f;
			}
		),
		m_vecTracers.end( )
				);

	if ( m_vecTracers.empty( ) )
		return;

	switch ( Config::Get<int>( Vars.VisBulletTracersType ) ) {
	case 1:
		if ( !PrecacheModel( _( "materials/sprites/laserbeam.vmt" ) ) )
			return;
		break;
	}

	for ( const auto& tracer : m_vecTracers ) {
		const auto alpha{ std::min( tracer.m_flStartTime + 5.f - Interfaces::Globals->flRealTime, 1.f ) };

		const auto& col{ tracer.m_pShooter == ctx.m_pLocal ? Config::Get<Color>( Vars.VisLocalBulletTracersCol ) : Config::Get<Color>( Vars.VisOtherBulletTracersCol ) };

		switch ( Config::Get<int>( Vars.VisBulletTracersType ) ) {
		case 0: {
			Vector2D start{ };
			if ( !Math::WorldToScreen( tracer.m_vecStart, start ) )
				continue;

			Vector2D end{ };
			if ( !Math::WorldToScreen( tracer.m_vecEnd, end ) )
				continue;

			Render::Line( start, end, col.Set<COLOR_A>( col.Get<COLOR_A>( ) * alpha ) );

			break;
		}
		case 1: {
			BeamInfo_t info{ };

			info.nType = 0;
			info.pszModelName = _( "materials/sprites/laserbeam.vmt" );
			info.nModelIndex = Interfaces::ModelInfo->GetModelIndex( _( "materials/sprites/laserbeam.vmt" ) );
			info.flHaloScale = 0.0f;
			info.flLife = 0.09f; //0.09
			info.flWidth = .6f;
			info.flEndWidth = .75f;
			info.flFadeLength = 3.0f;
			info.flAmplitude = 0.f;
			info.flBrightness = col.Get<COLOR_A>( ) * alpha;
			info.flSpeed = 1.f;
			info.iStartFrame = 1;
			info.flFrameRate = 60;
			info.flRed = col.Get<COLOR_R>( );
			info.flGreen = col.Get<COLOR_G>( );
			info.flBlue = col.Get<COLOR_B>( );
			info.nSegments = 4;
			info.bRenderable = true;
			info.nFlags = 0;

			info.vecStart = tracer.m_vecEnd;
			info.vecEnd = tracer.m_vecStart;

			const auto beam{ Interfaces::ViewRenderBeams->CreateBeamPoints( info ) };
			if ( beam )
				Interfaces::ViewRenderBeams->DrawBeam( beam );

			break;
		}
		}
	}
}