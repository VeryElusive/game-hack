#include "visuals.h"

void CVisuals::ManageHitmarkers( ) {
    if ( !ctx.m_pLocal || ctx.m_pLocal->IsDead( ) ) {
        hits.clear( );
        return;
    }

    hits.erase(
        std::remove_if(
            hits.begin( ), hits.end( ),
            [ & ]( const std::shared_ptr<hitmarker_t>& hit ) -> bool {
                return hit->time + 2.f < Interfaces::Globals->flRealTime || hit->alpha <= 0.f;
            }
        ),
        hits.end( )
                );

    for ( const auto& hit : hits ) {
        Math::WorldToScreen( hit->position, hit->screen );

        if ( hit->screen.IsZero( ) )
            continue;

        const auto step = Interfaces::Globals->flFrameTime * 2;

        if ( hit->time + 1.5f <= Interfaces::Globals->flRealTime )
            hit->alpha -= step;

        hit->step += step;

        DrawHitMarker( hit );
    }
}

void CVisuals::DrawHitMarker( const std::shared_ptr<hitmarker_t>& hit ) {
    if ( hit->alpha > 0 ) {
        const auto col{ Color( 200, 200, 200, static_cast< int >( 255 * hit->alpha ) ) };
        const auto outlineCol{ Color( 10, 10, 10, static_cast< int >( 255 * hit->alpha ) ) };

        // TODO: fix bottom pixel of each line....
        auto DrawOutlinedLine{ [ & ]( Vector2D pos1, Vector2D pos2, bool left ) {
            // left
            Render::Line( pos1 - Vector2D{ 1, 0 }, pos2 - Vector2D{ 1, 0 }, outlineCol );

            // right
            Render::Line( pos1 - Vector2D{ -1, 0 }, pos2 - Vector2D{ -1, 0 }, outlineCol );

            // middle longer
            Render::Line( pos1 - Vector2D{ 0, 1 }, pos2 + Vector2D{ 0, 1 }, outlineCol );

            Render::Line( pos1, pos2, col );
        } };


        // welcome to hell!
        if ( Config::Get<bool>( Vars.MiscWorldHitmarker ) ) {
            const auto& linesize{ Config::Get<int>( Vars.MiscWorldHitmarkerSize ) };
            auto lineOffset{ Config::Get<int>( Vars.MiscWorldHitmarkerGap ) };
            // top left
            DrawOutlinedLine( { hit->screen.x - linesize - lineOffset, hit->screen.y - linesize - lineOffset }, { hit->screen.x - lineOffset, hit->screen.y - lineOffset }, true );

            // top right
            DrawOutlinedLine( { hit->screen.x + linesize + lineOffset, hit->screen.y - linesize - lineOffset }, { hit->screen.x + lineOffset, hit->screen.y - lineOffset }, false );

            // bottom left
            DrawOutlinedLine( { hit->screen.x - linesize - lineOffset, hit->screen.y + linesize + lineOffset }, { hit->screen.x - lineOffset, hit->screen.y + lineOffset }, true );

            // bottom right
            DrawOutlinedLine( { hit->screen.x + linesize + lineOffset, hit->screen.y + linesize + lineOffset } , { hit->screen.x + lineOffset, hit->screen.y + lineOffset }, false );
        }

        if ( Config::Get<bool>( Vars.MiscScreenHitmarker ) ) {
            const auto& linesize{ Config::Get<int>( Vars.MiscScreenHitmarkerSize ) };
            const auto& lineOffset{ Config::Get<int>( Vars.MiscScreenHitmarkerGap ) };

            const auto center{ ctx.m_ve2ScreenSize / 2 };
            // top left
            DrawOutlinedLine( { center.x - linesize - lineOffset, center.y - linesize - lineOffset }, { center.x - lineOffset, center.y - lineOffset }, true );

            // top right
            DrawOutlinedLine( { center.x + linesize + lineOffset, center.y - linesize - lineOffset }, { center.x + lineOffset, center.y - lineOffset }, false );

            // bottom left
            DrawOutlinedLine( { center.x - linesize - lineOffset, center.y + linesize + lineOffset }, { center.x - lineOffset, center.y + lineOffset }, true );

            // bottom right
            DrawOutlinedLine( { center.x + linesize + lineOffset, center.y + linesize + lineOffset }, { center.x + lineOffset, center.y + lineOffset }, false );
        }

        if ( Config::Get<bool>( Vars.MiscDamageMarker ) ) {
            const auto col = Config::Get<Color>( Vars.MiscDamageMarkerCol ).Set<COLOR_A>( Config::Get<Color>( Vars.MiscDamageMarkerCol ).Get<COLOR_A>( ) * hit->alpha );

            const auto displayDmg{ static_cast< int >( std::ceil( hit->damage * std::min( Interfaces::Globals->flRealTime - hit->time, 1.f ) ) ) };
            Render::Text( Fonts::DamageMarker, Vector2D( hit->screen.x, hit->screen.y - 12 - hit->step * 5 ), col, FONT_CENTER, std::to_string( displayDmg ).c_str( ) );
        }
    }
}