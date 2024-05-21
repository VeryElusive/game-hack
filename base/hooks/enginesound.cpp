#include "../core/hooks.h"
#include "../core/config.h"
#include "../context.h"

int FASTCALL Hooks::hkEmitSound( void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk ) {
	static auto oEmitSound = DTR::EmitSound.GetOriginal<decltype( &hkEmitSound )>( );
	if ( ctx.m_bProhibitSounds )
		return 0;

	if ( Config::Get<int>(Vars.MiscWeaponVolume ) < 100 ) {
		const float volume_scale = static_cast< float >( Config::Get<int>( Vars.MiscWeaponVolume ) ) / 100;

		if ( strstr( pSample, _( "weapon" ) ) )
			flVolume = std::clamp( flVolume * volume_scale, 0.0f, 1.0f );
	}

	if ( strstr( pSample, ( _( "weapon" ) ) ) && ( strstr( pSample, ( _( "draw" ) ) ) || strstr( pSample, ( _( "deploy" ) ) ) ) ) {
		static FNV1A_t prev_hash{ };
		const auto hash{ FNV1A::Hash( pSample ) };

		if ( prev_hash == hash )
			return 0;

		prev_hash = hash;
	}

	return oEmitSound( _this, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk );
}