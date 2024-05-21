#include "../core/hooks.h"
#include "../utils/render.h"
#include "../features/visuals/visuals.h"
#include "../features/misc/logger.h"

HRESULT __stdcall Hooks::hkPresent( LPDIRECT3DDEVICE9 device, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion ) {
	static auto oPresent = DTR::Present.GetOriginal<decltype( &hkPresent )>( );

	// backup
	IDirect3DStateBlock9* state_block = nullptr;
	IDirect3DVertexDeclaration9* vert_dec = nullptr;
	IDirect3DVertexShader9* vert_shader = nullptr;

	if ( device->CreateStateBlock( D3DSBT_ALL, &state_block ) || state_block->Capture( ) || device->GetVertexDeclaration( &vert_dec ) || device->GetVertexShader( &vert_shader ) )
		return oPresent( device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion );

	Render::init( device );

	Features::Visuals.Main( );
	Features::Visuals.Watermark( );
	Features::Visuals.KeybindsList( );
	Features::Logger.PrintToScreen( );

	/* MENU */

	if ( Menu::MenuAlpha >= 0 && !Menu::Opened )
		Menu::MenuAlpha -= 5.f * Interfaces::Globals->flFrameTime;
	else if ( Menu::MenuAlpha <= 1 && Menu::Opened )
		Menu::MenuAlpha += 5.f * Interfaces::Globals->flFrameTime;

	Menu::MenuAlpha = std::clamp( Menu::MenuAlpha, 0.f, 1.f );
	Render::GlobalAlpha = Menu::MenuAlpha;

	Menu::render( );
	Menu::GetElements( );

	Render::draw( );

	Inputsys::scroll = 0;
	Render::GlobalAlpha = 1.f;

	// restore
	state_block->Apply( ), state_block->Release( ), device->SetVertexDeclaration( vert_dec ), device->SetVertexShader( vert_shader );

	return oPresent( device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion );
}

long __stdcall Hooks::hkReset( IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp ) {
	static auto oReset = DTR::Reset.GetOriginal<decltype( &hkReset )>( );

	Render::invalidate( );

	int ret = oReset( device, pp );
	Render::restore( device );
	return ret;
}