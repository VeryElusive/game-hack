#include "../core/hooks.h"
#include "../context.h"
#include "../features/visuals/visuals.h"

void FASTCALL Hooks::hkDrawModel( IStudioRender* thisptr, int edx, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags )
{
	static auto oDrawModel = DTR::DrawModel.GetOriginal<decltype( &hkDrawModel )>( );

	if ( !Interfaces::Engine->IsInGame( ) || Features::Visuals.Chams.m_bInSceneEnd )
		return oDrawModel( thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );

	Features::Visuals.Chams.Main( pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );
}


bool FASTCALL Hooks::hkOverrideConfig( IMaterialSystem* ecx, void* edx, MaterialSystemConfig_t& config, bool bForceUpdate ) {
	static auto oOverrideConfig = DTR::OverrideConfig.GetOriginal<decltype( &hkOverrideConfig )>( );

	if ( Config::Get<bool>(Vars.WorldFullbright ) )
		config.uFullbright = true;

	return oOverrideConfig( ecx, edx, config, bForceUpdate );
}


void FASTCALL Hooks::hkSceneEnd( void* ecx, int edx ) {
	static auto oSceneEnd = DTR::SceneEnd.GetOriginal<decltype( &hkSceneEnd )>( );

	oSceneEnd( ecx, edx );

	ctx.GetLocal( );
	
	if ( ctx.m_pLocal ) {
		Features::Visuals.Chams.m_bInSceneEnd = true;
		Features::Visuals.Chams.OnSceneEnd( );
		Features::Visuals.Chams.m_bInSceneEnd = false;
	}
}