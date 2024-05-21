#include "visuals.h"
#include "../../core/hooks.h"
#include "../animations/animation.h"
#include "../../context.h"

void CChams::InitMaterials( ) {
	RegularMat = CreateMaterial(
		_( "HAVOC_regular.vmt" ),
		_( "VertexLitGeneric" ),
		_(
			R"#("VertexLitGeneric" {
					"$basetexture" "vgui/white_additive"
					"$ignorez"      "0"
					"$model"		"1"
					"$flat"			"0"
					"$nocull"		"1"
					"$halflambert"	"1"
					"$nofog"		"1"
					"$wireframe"	"0"
				})#"
		)
	);

	FlatMat = CreateMaterial(
		_( "vuln_solid.vmt" ),
		_( "UnlitGeneric" ),
		_(
			R"#("UnlitGeneric" {
					"$basetexture" "vgui/white_additive"
					"$ignorez"      "0"
					"$model"		"1"
					"$flat"			"1"
					"$nocull"		"1"
					"$selfillum"	"1"
					"$halflambert"	"1"
					"$nofog"		"1"
					"$wireframe"	"0"
				})#"
		)
	);

	GlowMat = Interfaces::MaterialSystem->FindMaterial( _( "dev/glow_armsrace" ), nullptr );

	MetallicMat = CreateMaterial(
		_( "HAVOC_metallic.vmt" ),
		_( "VertexLitGeneric" ),
		_(
			R"#("VertexLitGeneric"
            {
                    "$basetexture"                "vgui/white_additive"
                    "$ignorez"                    "0"
                    "$phong"                    "1"
                    "$BasemapAlphaPhongMask"    "1"
                    "$phongexponent"            "15"
                    "$normalmapalphaenvmask"    "1"
                    "$envmap"                    "env_cubemap"
                    "$envmaptint"                "[0.0 0.0 0.0]"
                    "$phongboost"                "[0.6 0.6 0.6]"
                    "phongfresnelranges"        "[0.5 0.5 1.0]"
                    "$nofog"                    "1"
                    "$model"                    "1"
                    "$nocull"                    "0"
                    "$selfillum"                "1"
                    "$halflambert"                "1"
                    "$znearer"                    "0"
                    "$flat"                        "0"    
                    "$rimlight"                    "1"
                    "$rimlightexponent"            "2"
                    "$rimlightboost"            "0"
            }
            )#"
		)
	);

	GalaxyMat = CreateMaterial(
		_( "HAVOC_galaxy.vmt" ),
		_( "UnlitGeneric" ),
		_( R"#("UnlitGeneric" {
			"$basetexture" "dev\snowfield"
			"$additive" "1"
			"$model"		"1"
			"$flat"			"1"
			"$nocull"		"1"
			"$selfillum"	"1"
			"$halflambert"	"1"
			"$nofog"		"1"

			"Proxies"
			{
				"TextureScroll"
				{
					"textureScrollVar" "$baseTextureTransform"
					"textureScrollRate" "0.05"
					"textureScrollAngle" "0.0"
				}
			}
		}
		)#" )
	);

	if ( !RegularMat || RegularMat->IsErrorMaterial( ) )
		return;

	if ( !FlatMat || FlatMat->IsErrorMaterial( ) )
		return;

	if ( !GlowMat || GlowMat->IsErrorMaterial( ) )
		return;

	if ( !MetallicMat || MetallicMat->IsErrorMaterial( ) )
		return;

	if ( !GalaxyMat || GalaxyMat->IsErrorMaterial( ) )
		return;

	GalaxyMat->IncrementReferenceCount( );
	MetallicMat->IncrementReferenceCount( );
	GlowMat->IncrementReferenceCount( );
	FlatMat->IncrementReferenceCount( );
	RegularMat->IncrementReferenceCount( );

	init = true;
}

IMaterial* CChams::CreateMaterial( 
	const std::string_view name, const std::string_view shader, const std::string_view material 
) const {
	CKeyValues* pKeyValues = new CKeyValues( shader.data( ) );
	pKeyValues->LoadFromBuffer( name.data( ), material.data( ) );

	return Interfaces::MaterialSystem->CreateMaterial( name.data( ), pKeyValues );
}

void CChams::OverrideMaterial(
	const int type, const bool ignore_z, Color col, const int glow, const bool wireframe
) const {
	IMaterial* material{ };

	switch ( type ) {
	case 0: material = RegularMat; break;
	case 1: material = FlatMat; break;
	case 2: material = GlowMat; break;
	case 3: material = MetallicMat; break;
	case 4: material = GalaxyMat; break;
	default: break;
	}

	float colorArray[ 3 ] = { col[ 0 ] / 255.f, col[ 1 ] / 255.f, col[ 2 ] / 255.f };

	if ( m_bInSceneEnd ) {
		Interfaces::RenderView->SetColorModulation( colorArray );
		Interfaces::RenderView->SetBlend( col[ 3 ] / 255.f );
	}

	material->AlphaModulate( col[ 3 ] / 255.f );
	material->ColorModulate( colorArray[ 0 ], colorArray[ 1 ], colorArray[ 2 ] );
	material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, ignore_z );
	material->SetMaterialVarFlag( MATERIAL_VAR_WIREFRAME, wireframe );

	switch ( type ) {
	case 0: material = RegularMat; break;
	case 1: material = FlatMat; break;
	case 2: material = GlowMat; break;
	case 3: material = MetallicMat; break;
	case 4: material = GalaxyMat; break;
	default: break;
	}

	material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, ignore_z );
	material->SetMaterialVarFlag( MATERIAL_VAR_WIREFRAME, wireframe );

	if ( type > 1 ) {
		if ( const auto $envmaptint = material->FindVar( _( "$envmaptint" ), nullptr, false ) )
			$envmaptint->SetVector( colorArray[ 0 ], colorArray[ 1 ], colorArray[ 2 ] );
	}

	if ( type == 2 ) {
		if ( const auto envmap = material->FindVar( _( "$envmapfresnelminmaxexp" ), nullptr ) )
			envmap->SetVector( 0.f, 1.f, std::clamp<float>( ( 100.0f - glow ) * 0.2f, 1.f, 20.f ) );
	}

	Interfaces::StudioRender->ForcedMaterialOverride( material );
}

void CChams::Main( DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags ) {
	static auto oDrawModel = DTR::DrawModel.GetOriginal<decltype( &Hooks::hkDrawModel )>( );

	if ( !init )
		InitMaterials( );

	IClientRenderable* pRenderable{ info.pClientEntity };
	if ( !pRenderable )
		return oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );

	CBasePlayer* pPlayer{ static_cast< CBasePlayer* >( pRenderable->GetIClientUnknown( )->GetBaseEntity( ) ) };
	if ( !pPlayer )
		return oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );

	const auto isPlayerModel{ *reinterpret_cast< const std::uint32_t* >( info.pStudioHdr->szName + 14u ) == 'yalp' };

	// rendering deez in scene end
	/*if ( pPlayer->GetClientClass( )->nClassID == EClassIndex::CBaseWeaponWorldModel ) {
		// skip glow models
		if ( nFlags & ( STUDIO_RENDER | STUDIO_SKIP_FLEXES | STUDIO_DONOTMODIFYSTENCILSTATE | STUDIO_NOLIGHTING_OR_CUBEMAP | STUDIO_SKIP_DECALS ) )
			return oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );

		const auto owner{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( pPlayer->m_hCombatWeaponParent( ) ) ) };

		int type{ ENEMY };
		if ( owner == ctx.m_pLocal )
			type = LOCAL;
		else if ( owner->IsTeammate( ) )
			type = TEAM;

		CheckIfPlayer( ChamVis, type )
			return;
	}
	// rendering deez in scene end
	else*/ if ( isPlayerModel ) {
		// skip glow models
		if ( nFlags & ( STUDIO_RENDER | STUDIO_SKIP_FLEXES | STUDIO_DONOTMODIFYSTENCILSTATE | STUDIO_NOLIGHTING_OR_CUBEMAP | STUDIO_SKIP_DECALS ) )
			return oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );

		int type{ ENEMY };
		if ( pPlayer == ctx.m_pLocal )
			type = LOCAL;
		else if ( pPlayer->IsTeammate( ) )
			type = TEAM;

		CheckIfPlayer( ChamVis, type ) 
			return;
	}
	else if ( *reinterpret_cast< const std::uint32_t* >( info.pStudioHdr->szName + 17u ) == 'smra' ) {
		if ( Config::Get<bool>( Vars.ChamHand ) ) {
			int Mat{ Config::Get<int>( Vars.ChamHandMat ) };

			OverrideMaterial(
				Mat, false,
				Config::Get<Color>( Vars.ChamHandCol ),
				Config::Get<int>( Vars.ChamHandGlow ),
				false
			);

			oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );
			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
		}
		else
			oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );

		if ( Config::Get<bool>( Vars.ChamHandOverlay ) ) {
			int Mat{ Config::Get<int>( Vars.ChamHandOverlayMat ) };

			OverrideMaterial(
				Mat, false,
				Config::Get<Color>( Vars.ChamHandOverlayCol ),
				Config::Get<int>( Vars.ChamHandGlow ),
				false
			);

			oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );
			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
		}
		return;
	}
	else if ( *reinterpret_cast< const std::uint32_t* >( info.pStudioHdr->szName ) == 'paew' && info.pStudioHdr->szName[ 8 ] == 'v' ) {
		if ( Config::Get<bool>( Vars.ChamWeapon ) ) {
			int Mat{ Config::Get<int>( Vars.ChamWeaponMat ) };

			OverrideMaterial(
				Mat, false,
				Config::Get<Color>( Vars.ChamWeaponCol ),
				Config::Get<int>( Vars.ChamWeaponGlow ),
				false
			);

			oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );
			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
		}
		else
			oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );

		if ( Config::Get<bool>( Vars.ChamWeaponOverlay ) ) {
			int Mat{ Config::Get<int>( Vars.ChamWeaponOverlayMat ) };

			OverrideMaterial(
				Mat, false,
				Config::Get<Color>( Vars.ChamWeaponOverlayCol ),
				Config::Get<int>( Vars.ChamWeaponGlow ),
				false
			);

			oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );
			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
		}

		return;
	}

	oDrawModel( Interfaces::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );
}

void CChams::RenderShotChams( ) {
	if ( !ctx.m_pLocal )
		return m_Hitmatrix.clear( );

	if ( m_Hitmatrix.empty( ) )
		return;

	const auto ctx{ Interfaces::MaterialSystem->GetRenderContext( ) };
	if ( !ctx )
		return;

	auto DrawModelRebuild = [ & ]( C_HitMatrixEntry it ) -> void {
		DrawModelResults_t results;
		DrawModelInfo_t info;
		ColorMeshInfo_t* pColorMeshes = NULL;
		info.bStaticLighting = false;
		info.pStudioHdr = it.state.pStudioHdr;
		info.pHardwareData = it.state.pStudioHWData;
		info.iSkin = it.info.iSkin;
		info.iBody = it.info.iBody;
		info.iHitboxSet = it.info.iHitboxSet;
		info.pClientEntity = ( IClientRenderable* )it.state.pRenderable;
		info.iLOD = it.state.iLOD;
		info.pColorMeshes = pColorMeshes;

		bool bShadowDepth = ( it.info.iFlags & STUDIO_SHADOWDEPTHTEXTURE ) != 0;

		// Don't do decals if shadow depth mapping...
		info.hDecals = bShadowDepth ? 0 : it.state.pDecals;

		// Sets up flexes
		float* pFlexWeights = NULL;
		float* pFlexDelayedWeights = NULL;

		int overlayVal = Displacement::Cvars.r_drawmodelstatsoverlay->GetInt( );
		int drawFlags = it.state.iDrawFlags;

		if ( bShadowDepth ) {
			drawFlags |= STUDIORENDER_DRAW_OPAQUE_ONLY;
			drawFlags |= STUDIORENDER_SHADOWDEPTHTEXTURE;
		}

		if ( overlayVal && !bShadowDepth ) {
			drawFlags |= STUDIORENDER_DRAW_GET_PERF_STATS;
		}


		Interfaces::StudioRender->DrawModel( &results, &info, it.pBoneToWorld, pFlexWeights, pFlexDelayedWeights, it.info.vecOrigin, drawFlags );
		Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
	};

	for ( auto i = m_Hitmatrix.begin( ); i != m_Hitmatrix.end( ); ) {
		if ( !i->m_pPlayer ) {
			i = m_Hitmatrix.erase( i );
			continue;
		}

		const auto playerInfo{ Interfaces::Engine->GetPlayerInfo( i->m_pPlayer->Index( ) ) };
		if ( !playerInfo.has_value( ) ) {
			i = m_Hitmatrix.erase( i );
			continue;
		}

		const auto delta{ i->time - Interfaces::Globals->flRealTime };
		if ( delta <= 0 ) {
			i = m_Hitmatrix.erase( i );
			continue;
		}

		auto alpha{ 1.f };

		if ( delta < 0.5f )
			alpha = delta * 2;

		auto color{ Config::Get<Color>( Vars.MiscHitMatrixCol ) };
		color = color.Set<COLOR_A>( color.Get<COLOR_A>( ) * alpha );

		int Mat{ Config::Get<int>( Vars.MiscHitMatrixMat ) };

		OverrideMaterial(
			Mat, Config::Get<bool>( Vars.MiscHitMatrixXQZ ),
			color,
			Config::Get<int>( Vars.ChamGlowStrengthEnemy ),
			false
		);

		Interfaces::StudioRender->SetAlphaModulation( alpha );

		DrawModelRebuild( *i );

		//Interfaces::ModelRender->DrawModelExecute( ctx, i->state, i->info, i->pBoneToWorld );
		//Interfaces::StudioRender->ForcedMaterialOverride( nullptr );

		i = std::next( i );
	}

	Interfaces::StudioRender->SetAlphaModulation( 1.f );
}

void CChams::AddHitmatrix( CBasePlayer* player, matrix3x4_t* bones ) {
	const auto model{ player->GetModel( ) };
	if ( !model )
		return;

	const auto mdlData = player->m_pStudioHdr( );
	if ( !mdlData
		|| !mdlData->pStudioHdr )
		return;

	auto& hit = m_Hitmatrix.emplace_back( );

	hit.m_pPlayer = player;
	hit.time = Interfaces::Globals->flRealTime + Config::Get<float>( Vars.MiscHitMatrixTime );
	hit.state.pStudioHdr = mdlData->pStudioHdr;
	hit.state.pStudioHWData = Interfaces::MDLCache->GetHardwareData( model->hStudio );
	hit.state.pRenderable = player->GetClientRenderable( );

	hit.info.pRenderable = player->GetClientRenderable( );
	hit.info.pModel = model;
	hit.info.iHitboxSet = player->m_nHitboxSet( );
	hit.info.iSkin = player->GetSkin( );
	hit.info.iBody = player->GetBody( );
	hit.info.nEntityIndex = player->Index( );
	hit.info.vecOrigin = player->GetAbsOrigin( );
	hit.info.angAngles.y = player->GetAbsAngles( ).y;

	hit.info.hInstance = player->GetModelInstance( );
	hit.info.iFlags = 1;

	std::memcpy( hit.pBoneToWorld, bones, player->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

	Math::AngleMatrix( hit.info.angAngles, hit.model_to_world );

	hit.model_to_world[ 0 ][ 3 ] = hit.info.vecOrigin.x;
	hit.model_to_world[ 1 ][ 3 ] = hit.info.vecOrigin.y;
	hit.model_to_world[ 2 ][ 3 ] = hit.info.vecOrigin.z;

	hit.info.pModelToWorld = hit.state.pModelToWorld = &hit.model_to_world;
}

void CChams::OnSceneEnd( ) {
	if ( !init )
		InitMaterials( );

	RenderShotChams( );

	std::vector<CBasePlayer*> queue;
	for ( int i{ 1 }; i < 64; ++i ) {
		const auto player{ static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntity( i ) ) };
		if ( !player || !player->IsPlayer( ) )
			continue;

		auto& entry = Features::AnimSys.m_arrEntries.at( i - 1 );

		if ( entry.m_vecUpdatedOrigin != Vector( 0, 0, 0 )
			|| player == ctx.m_pLocal )
			queue.push_back( player );
	}

	auto origin{ ctx.m_pLocal->GetAbsOrigin( ) + Interfaces::Input->vecCameraOffset };
	std::sort( queue.begin( ), queue.end( ), [ origin ]( CBasePlayer* pl, CBasePlayer* pl0 ) { return origin.DistToSqr( pl->GetAbsOrigin( ) ) > origin.DistToSqr( pl0->GetAbsOrigin( ) ); } );

	for ( auto& player : queue ) {
		int type{ ENEMY };
		if ( player == ctx.m_pLocal )
			type = LOCAL;
		else if ( player->IsTeammate( ) )
			type = TEAM;

		int Mat{ };
		GetPlayerIntFig( type, ChamMatInvisible, Mat );

		int GlowStrength{ };
		GetPlayerIntFig( type, ChamGlowStrength, GlowStrength );

		if ( type == ENEMY && Config::Get<bool>( Vars.ChamBacktrack ) ) {
			Color Col{ Config::Get<Color>( Vars.ChamBacktrackCol ) };

			const auto& records = Features::AnimSys.m_arrEntries.at( player->Index( ) - 1 ).m_pRecords;
			auto matrix{ new matrix3x4_t[ 256 ] };
			bool valid{ };
			if ( !records.empty( ) ) {
				for ( auto it{ records.rbegin( ) }; it != records.rend( ); it = std::next( it ) ) {
					const auto& record{ *it };

					if ( record->m_bFirst )
						continue;

					const auto validity{ record->Validity( ) };
					if ( !validity )// **( int** ) Displacement::Sigs.numticks * 2 )
						continue;

					if ( validity < 3
						&& ctx.m_iTicksAllowed )
						continue;

					if ( ( record->m_cAnimData.m_vecOrigin - player->GetAbsOrigin( ) ).Length( ) < 2.f )
						continue;

					std::memcpy(
						matrix, record->m_cAnimData.m_arrSides.at( 0 ).m_pMatrix,
						player->m_iBoneCount( ) * sizeof( matrix3x4_t )
					);

					valid = true;
				}

				if ( valid ) {
					// lazy, AND WHAT HAHA
					const auto delta{ ( player->m_CachedBoneData( )[ 4 ].GetOrigin( ) - matrix[ 4 ].GetOrigin( ) ).Length( ) };
					if ( delta < 40.f )
						Col = Col.Set<COLOR_A>( Col.Get<COLOR_A>( ) * ( delta / 40.f ) );

					OverrideMaterial(
						Config::Get<int>( Vars.ChamBacktrackMat ), true,
						Col,
						GlowStrength,
						false
					);

					auto backupBones{ new matrix3x4_t[ 256 ] };
					std::memcpy(
						backupBones, player->m_CachedBoneData( ).Base( ),
						player->m_iBoneCount( ) * sizeof( matrix3x4_t )
					);
					// should do cbb... idc tho.

					std::memcpy(
						player->m_CachedBoneData( ).Base( ), matrix,
						player->m_iBoneCount( ) * sizeof( matrix3x4_t )
					);

					const auto backup{ ctx.m_bInCreatemove };
					ctx.m_bInCreatemove = true;
					player->DrawModel( STUDIO_RENDER, { } );
					ctx.m_bInCreatemove = false;

					Interfaces::StudioRender->ForcedMaterialOverride( nullptr );

					std::memcpy(
						player->m_CachedBoneData( ).Base( ), backupBones,
						player->m_iBoneCount( ) * sizeof( matrix3x4_t )
					);

					delete[ ] backupBones;
				}
			}

			delete[ ] matrix;
		}

		if ( type == LOCAL && Config::Get<bool>( Vars.ChamDesync ) ) {
			Color Col{ Config::Get<Color>( Vars.ChamDesyncCol ) };

			int Mat{ Config::Get<int>( Vars.ChamDesyncMat ) };

			auto matrix{ new matrix3x4_t[ 256 ] };
			std::memcpy( matrix, ctx.m_cFakeData.m_matMatrix, 256 * sizeof( matrix3x4_t ) );

			const auto delta{ ctx.m_pLocal->GetAbsOrigin( ) - ctx.m_vecSetupBonesOrigin };

			for ( std::size_t i{ }; i < 256; ++i ) {
				auto& mat{ ctx.m_cFakeData.m_matMatrix[ i ] };
				auto& bone{ matrix[ i ] };
				bone[ 0 ][ 3 ] = mat[ 0 ][ 3 ] + delta.x;
				bone[ 1 ][ 3 ] = mat[ 1 ][ 3 ] + delta.y;
				bone[ 2 ][ 3 ] = mat[ 2 ][ 3 ] + delta.z;
			}

			OverrideMaterial(
				Mat, false,
				Col,
				GlowStrength,
				false
			);

			auto backupBones{ new matrix3x4_t[ 256 ] };
			std::memcpy( backupBones, player->m_CachedBoneData( ).Base( ), player->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

			std::memcpy( player->m_CachedBoneData( ).Base( ), matrix, player->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

			const auto backup{ ctx.m_bInCreatemove };
			ctx.m_bInCreatemove = true;
			player->DrawModel( STUDIO_RENDER, { } );
			ctx.m_bInCreatemove = false;
			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );

			std::memcpy( player->m_CachedBoneData( ).Base( ), backupBones, player->m_iBoneCount( ) * sizeof( matrix3x4_t ) );

			delete[ ] backupBones;
			delete[ ] matrix;

			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
		}

		CheckIfPlayer( ChamHid, type ) {
			Color Col{ };
			GetPlayerColorFig( type, ChamHidCol, Col );

			OverrideMaterial(
				Mat, true,
				Col,
				GlowStrength,
				false
			);

			player->DrawModel( STUDIO_RENDER, { } );

			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
			//ret = false;
		}

		CheckIfPlayer( ChamVis, type ) {
			Color Col{ };
			GetPlayerColorFig( type, ChamVisCol, Col );

			GetPlayerIntFig( type, ChamMatVisible, Mat );

			OverrideMaterial(
				Mat, false,
				Col,
				GlowStrength,
				false
			);

			player->DrawModel( STUDIO_RENDER, { } );

			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
		}
		// else we render regular model
		//else
		//	player->DrawModel( STUDIO_RENDER, { } );

		CheckIfPlayer( ChamDouble, type ) {
			Color Col{ };
			GetPlayerColorFig( type, ChamDoubleCol, Col );

			int Mat{ };
			GetPlayerIntFig( type, ChamDoubleMat, Mat );

			bool z{ };
			GetPlayerBoolFig( type, ChamDoubleZ, z )

				OverrideMaterial(
					Mat, z,
					Col,
					GlowStrength,
					false
				);

			player->DrawModel( STUDIO_RENDER, { } );

			Interfaces::StudioRender->ForcedMaterialOverride( nullptr );
		}
	}
}