#include "../core/hooks.h"
#include "../core/menu rework/menu.h"
#include "../utils/render.h"
#include "../features/visuals/visuals.h"
#include "../features/misc/logger.h"
#include "../context.h"
#include "../core/framework/framework.h"

void FASTCALL Hooks::hkLockCursor( ISurface* thisptr, int edx )
{
	static auto oLockCursor = DTR::LockCursor.GetOriginal<decltype( &hkLockCursor )>( );

	if ( Menu::m_bOpened ) {
		Interfaces::Surface->UnLockCursor( );
		return;
	}

	oLockCursor( thisptr, edx );
}

void FASTCALL Hooks::hkPaintTraverse( ISurface* thisptr, int edx, unsigned int uPanel, bool bForceRepaint, bool bForce )
{
	static auto oPaintTraverse = DTR::PaintTraverse.GetOriginal<decltype( &hkPaintTraverse )>( );
	const FNV1A_t uPanelHash = FNV1A::Hash( Interfaces::Panel->GetName( uPanel ) );

	

	if ( Config::Get<bool>( Vars.RemovalScope ) && uPanelHash == FNV1A::HashConst( ( "HudZoom" ) ) )
		return;

	oPaintTraverse( thisptr, edx, uPanel, bForceRepaint, bForce );
}

void FASTCALL Hooks::HkPaint( const std::uintptr_t ecx, const std::uintptr_t edx, const int mode ) {
	static auto oPaint = DTR::Paint.GetOriginal<decltype( &HkPaint )>( );

	ctx.GetLocal( );

	oPaint( ecx, edx, mode );

	typedef void( __thiscall* start_drawing_t )( void* );
	typedef void( __thiscall* finish_drawing_t )( void* );

	static auto StartDraw = ( start_drawing_t )Displacement::Sigs.StartDrawing;
	static auto Finishdrawing = ( start_drawing_t )Displacement::Sigs.FinishDrawing;

	// ik nem did 2 aswell as 1 weird huh
	if ( mode & 1 /* || mode & 2*/ ) {
		StartDraw( Interfaces::Surface );
		
		Features::Visuals.Main( );
		Features::Visuals.Watermark( );
		Features::Visuals.KeybindsList( );
		Features::Logger.PrintToScreen( );

		/* MENU */
		Inputsys::updateNeededKeys( );
		Inputsys::update( );

		/*if ( Menu::MenuAlpha >= 0 && !Menu::Opened )
			Menu::MenuAlpha -= 5.f * Interfaces::Globals->flFrameTime;
		else if ( Menu::MenuAlpha <= 1 && Menu::Opened )
			Menu::MenuAlpha += 5.f * Interfaces::Globals->flFrameTime;

		Menu::MenuAlpha = std::clamp( Menu::MenuAlpha, 0.f, 1.f );*/
		Render::GlobalAlpha = Menu::m_flAlpha;

		Menu::Render( );

		//Menu::render( );
		//Menu::GetElements( );

		//MenuFramework::Main( );

		Inputsys::scroll = 0;

		Render::GlobalAlpha = 1.f;
		/* FINISH MENU */

		Finishdrawing( Interfaces::Surface );

		static int beep_boop = 500;
		if ( beep_boop >= 500 ) {
			int x, y;
			Interfaces::Engine->GetScreenSize( x, y );

			if ( x != ctx.m_ve2ScreenSize.x || y != ctx.m_ve2ScreenSize.y )
				Render::CreateFonts( );
			
			ctx.m_ve2ScreenSize.x = x;
			ctx.m_ve2ScreenSize.y = y;
			beep_boop = 0;
		}
		beep_boop++;
	}
}