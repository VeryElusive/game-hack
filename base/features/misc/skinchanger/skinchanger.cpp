/*#include "skinchanger.h"
#include "kitparser.h"
#include "../../../core/config.h"

void CSkinChanger::OnNetworkUpdate( bool start ) {
	if ( ctx.m_pLocal->IsDead( ) )
		return;

	if ( !start ) {
		if ( m_bUpdateSkins && !m_bUpdateGloves ) {
			// CLEANUP: wtf is this soufiw???

			float deltatime = Interfaces::Globals->flRealTime - m_flLastSkinsUpdate;
			if ( deltatime >= 0.65f )
				m_flLastSkinsUpdate = Interfaces::Globals->flRealTime - 0.125f;
			else if ( deltatime >= 0.2f ) {
				ForceItemUpdate( ctx.m_pLocal );
				m_bUpdateSkins = false;
				m_flLastSkinsUpdate = Interfaces::Globals->flRealTime;
			}
		}

		/*if ( ( !m_bActive /* || !global.m_glove_changer  ) || m_bUpdateGloves ) {
			auto glove = static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( m_hGloveHandle ) );
			if ( glove ) {
				glove->GetClientNetworkable( )->SetDestroyedOnRecreateEntities( );
				glove->GetClientNetworkable( )->Release( );
				m_hGloveHandle = INVALID_EHANDLE_INDEX;
			}

			const auto glove_config = GetDataFromIndex( global.m_gloves_idx );
			if ( ( m_bUpdateGloves && Interfaces::Globals->flRealTime - m_flLastGlovesUpdate >= 0.5f ) || ( glove_config && !glove_config->m_enabled && glove_config->m_executed ) ) {
				Interfaces::ClientState->iDeltaTick = -1;
				if ( m_bUpdateGloves )
					m_flLastGlovesUpdate = Interfaces::Globals->flRealTime;

				m_bUpdateGloves = false;

				if ( glove_config )
					glove_config->m_executed = false;
			}
		}
		else if ( ctx.m_pLocal&& /*global.m_glove_changer ) {
			GloveChanger( ctx.m_pLocal );
		}* /
		return;
	}

	if ( !m_bActive )
		return;

	PostDataUpdateStart( local );
}

void CSkinChanger::ForceItemUpdate( CBasePlayer* local ) {
	if ( !local || local->IsDead( ) )
		return;

	auto ForceUpdate = [ ]( CBaseCombatWeapon* item ) {
		CEconItemView* view = reinterpret_cast< CEconItemView* >( item );

		if ( !view )
			return;

		if ( !item->GetClientNetworkable( ) )
			return;

		auto clearCustomMaterials = [ ]( CUtlVector< IRefCounted* >& vec ) {
			for ( int i = 0; i < vec.Count( ); ++i ) {
				auto& element = vec.Element( i );
				// actually makes no sense
				*( int* ) ( ( ( uintptr_t ) element ) + 0x10 ) = 0;
				*( int* ) ( ( ( uintptr_t ) element ) + 0x18 ) = 0;
				*( int* ) ( ( ( uintptr_t ) element ) + 0x20 ) = 0;
				*( int* ) ( ( ( uintptr_t ) element ) + 0x24 ) = 0;

				vec.Remove( i );
			}
		};

		item->m_bCustomMaterialInitialized( ) = false;
		clearCustomMaterials( item->m_CustomMaterials( ) );
		item->m_VisualsDataProcessors( ).RemoveAll( );

		item->GetClientNetworkable( )->PostDataUpdate( DATA_UPDATE_CREATED );
		item->GetClientNetworkable( )->OnDataChanged( DATA_UPDATE_CREATED );
	};

	UpdateHud( );

	auto weapons = local->m_hMyWeapons( );
	for ( size_t i = 0; i < 48; ++i ) {
		auto weapon_handle = weapons[ i ];
		if ( !weapon_handle != INVALID_EHANDLE_INDEX )
			break;

		auto weapon = static_cast< CBaseCombatWeapon* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( weapon_handle ) );
		if ( !weapon )
			continue;

		ForceUpdate( weapon );
	}
}

struct hud_weapons_t {
	std::int32_t* get_weapon_count( ) {
		return reinterpret_cast< std::int32_t* >( std::uintptr_t( this ) + 0x80 );
	}
};

void CSkinChanger::UpdateHud( ) {
	if ( Displacement::Sigs.m_uClearHudWeaponIcon ) {
		static auto clear_hud_weapon_icon_fn =
			reinterpret_cast< std::int32_t( __thiscall* )( void*, std::int32_t ) >( ( Displacement::Sigs.m_uClearHudWeaponIcon ) );
		auto element = MEM::FindHudElement<std::uintptr_t*>( ( _( "CCSGO_HudWeaponSelection" ) ) );

		if ( element && clear_hud_weapon_icon_fn ) {
			auto hud_weapons = reinterpret_cast< hud_weapons_t* >( std::uintptr_t( element ) - 0xA0 );
			if ( hud_weapons == nullptr )
				return;

			if ( !*hud_weapons->get_weapon_count( ) )
				return;

			for ( std::int32_t i = 0; i < *hud_weapons->get_weapon_count( ) - 1; i++ )
				i = clear_hud_weapon_icon_fn( hud_weapons, i );
		}
	}
}

void CSkinChanger::PostDataUpdateStart( CBasePlayer* local ) {
	auto player_info = Interfaces::Engine->GetPlayerInfo( local->Index( ) );
	if ( !player_info.has_value( ) )
		return;

	const auto m_knife_idx{ k_knife_names.at( Config::Get<int>( Vars.SkinKnifeChanger ) ).definition_index };

	// we can do our model changer here too

	static bool test;

	// Handle weapon configs
	{
		auto weapons = local->m_hMyWeapons( );
		for ( int i = 0; i < 48; ++i ) {
			auto weapon_handle = weapons[ i ];
			if ( !weapon_handle != INVALID_EHANDLE_INDEX )
				break;

			auto weapon = static_cast< CWeaponCSBase* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( weapon_handle ) );
			if ( !weapon )
				continue;

			// MIGHT BE AN ISSUE
			auto& definition_index = weapon->m_iItemDefinitionIndex( );

			auto idx = weapon->IsKnife( ) ? m_knife_idx : definition_index;
			const auto active_conf = GetDataFromIndex( idx );
			if ( active_conf ) {
				if ( ( !active_conf->m_enabled || !m_bActive ) && active_conf->m_executed )
					m_bUpdateSkins = true;

				ApplyConfigOnAttributableItem( weapon, active_conf, player_info.xuid_low );
			}
			else
				EraseOverrideIfExistsByIndex( definition_index );
		}
	}
	const auto view_model = ( C_BaseViewModel* ) local->m_hViewModel( ).Get( );
	if ( !view_model )
		return;

	//using UpdateAllViewmodelAddons_t = int( __thiscall* )( C_BaseViewModel* );
	//static auto UpdateAllViewmodelAddonsAdr = Memory::Scan( _( "client.dll" ), _( "55 8B EC 83 E4 ? 83 EC ? 53 8B D9 56 57 8B 03 FF 90 ? ? ? ? 8B F8 89 7C 24 ? 85 FF 0F 84 ? ? ? ? 8B 17 8B CF" ) );
	//( ( UpdateAllViewmodelAddons_t ) UpdateAllViewmodelAddonsAdr ) ( view_model );

	const auto view_model_weapon = ( C_BaseAttributableItem* ) view_model->m_hWeapon( ).Get( );
	if ( !view_model_weapon )
		return;

	auto idx = view_model_weapon->m_Item( ).m_iItemDefinitionIndex( );
	if ( k_weapon_info.count( idx ) > 0 ) {
		const auto override_info = k_weapon_info.at( idx );
		const auto override_model_index = Source::m_pModelInfo->GetModelIndex( override_info.model );

		const auto weapon = reinterpret_cast< C_WeaponCSBaseGun* >( view_model_weapon );
		if ( weapon ) {
			view_model->SetModelIndex( override_model_index );

			auto weapondata = weapon->GetCSWeaponData( );
			if ( weapondata.IsValid( ) ) {
				const auto world_model = view_model_weapon->m_hWeaponWorldModel( ).Get( );
				if ( world_model )
					world_model->SetModelIndex( override_model_index + 1 );
			}
		}
	}
}*/