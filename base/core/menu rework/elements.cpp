#include "menu.h"
#include "../../features/misc/logger.h"
#include "../../features/misc/skinchanger/kitparser.h"
#include "abuse.h"

int selectedConfig{ };
std::string typedConfig{ };
int weapGroup{ };

void __cdecl LoadConfig( ) {
	const auto fig{ Config::vecFileNames.empty( ) ? "" : Config::vecFileNames.at( selectedConfig ) };
	Config::Load( fig );
	Features::Logger.Log( _( "Loaded config " ) + fig, true );
}

void __cdecl SaveConfig( ) {
	const auto fig{ Config::vecFileNames.empty( ) ? "" : Config::vecFileNames.at( selectedConfig ) };
	Config::Save( fig );
	Features::Logger.Log( _( "Saved config " ) + fig, true );
}

void __cdecl RemoveConfig( ) {
	const auto fig{ Config::vecFileNames.empty( ) ? "" : Config::vecFileNames.at( selectedConfig ) };
	Config::Remove( selectedConfig );
	Features::Logger.Log( _( "Removed config " ) + fig, true );
	Config::m_bDueRefresh = true;
}

void __cdecl CreateConfig( ) {
	Config::Save( typedConfig );
	Features::Logger.Log( _( "Created config " ) + typedConfig, true );
	Config::m_bDueRefresh = true;
}

void __cdecl RefreshConfig( ) {
	Config::m_bDueRefresh = true;
}

void __cdecl ResetMenuSize( ) {
	Menu::m_vecSize = { 800, 650 };
	Features::Logger.Log( _( "Reset menu size." ), true );
}

void __cdecl Unload( ) {
	ctx.m_bUnload = true;
}

void Menu::Register( ) {
	m_cTabs[ 0 ].m_szName = "A";
	m_cTabs[ 1 ].m_szName = "F";
	m_cTabs[ 2 ].m_szName = "C";
	m_cTabs[ 3 ].m_szName = "D";
	m_cTabs[ 4 ].m_szName = "E";

	auto aimbot{ &m_cTabs[ 0 ].m_vecSubtabs.emplace_back( "Aimbot", 2 ) };
	{
		auto generalGroup{ aimbot->AddGroup( "General", 1.f ) };
		{
			generalGroup->Register( _( "Enable" ), &Config::Get<bool>( Vars.RagebotEnable ) );
			generalGroup->Register( _( "Resolver" ), &Config::Get<bool>( Vars.RagebotResolver ) );
			generalGroup->Register( _( "Lag-compensation" ), &Config::Get<bool>( Vars.RagebotLagcompensation ) );
			generalGroup->Register( _( "Weapon group" ), &weapGroup, 9, weaponGroups );

			RAGEINTSLIDER( generalGroup, _( "FOV" ), RagebotFOV, weapGroup, 1, 180 );
			RAGECHECKBOX( generalGroup, _( "Auto fire" ), RagebotAutoFire, weapGroup );
			RAGECHECKBOX( generalGroup, _( "Auto scope" ), RagebotAutoScope, weapGroup );
			RAGECHECKBOX( generalGroup, _( "Silent aim" ), RagebotSilentAim, weapGroup );
			RAGEINTSLIDER( generalGroup, _( "Hitchance" ), RagebotHitchance, weapGroup, 0, 100 );
			RAGEINTSLIDER( generalGroup, _( "Noscope hitchance" ), RagebotNoscopeHitchance, weapGroup, 0, 100 );
			RAGECHECKBOX( generalGroup, _( "Ensure accuracy" ), RagebotHitchanceThorough, weapGroup );
			RAGEINTSLIDER( generalGroup, _( "Minimum damage" ), RagebotMinimumDamage, weapGroup, 0, 110 );
			RAGECHECKBOX( generalGroup, _( "Autowall" ), RagebotAutowall, weapGroup );

			RAGEINTSLIDER_SHOWCONDITION( generalGroup, _( "Penetration damage" ), RagebotPenetrationDamage, weapGroup, 0, 110, RagebotAutowall );

			RAGECHECKBOX( generalGroup, _( "Scale damage" ), RagebotScaleDamage, weapGroup );
			generalGroup->Register( _( "Damage override" ), &Config::Get<bool>( Vars.RagebotDamageOverride ) );
			generalGroup->Register( &Config::Get<keybind_t>( Vars.RagebotDamageOverrideKey ) );

			RAGEINTSLIDER( generalGroup, _( "Override damage" ), RagebotOverrideDamage, weapGroup, 0, 110 );
			RAGECHECKBOX( generalGroup, _( "Autostop" ), RagebotAutoStop, weapGroup );
			RAGECHECKBOX( generalGroup, _( "Move between shots" ), RagebotBetweenShots, weapGroup );
			//RAGECHECKBOX( generalGroup, _( "Stop in air" ), RagebotAutostopInAir, weapGroup );
			generalGroup->Register( _( "Zeusbot" ), &Config::Get<bool>( Vars.RagebotZeusbot ) );
			generalGroup->Register( _( "Knifebot" ), &Config::Get<bool>( Vars.RagebotKnifebot ) );
		}

		auto selectionGroup{ aimbot->AddGroup( "Selection", 0.6f ) };
		{
			selectionGroup->Register( _( "Target selection" ), &Config::Get<int>( Vars.RagebotTargetSelection ), 4, targetSelection );

			RAGEMULTICOMBO( selectionGroup, _( "Hitboxes" ), 9, hitboxes, weapGroup );
			RAGEMULTICOMBO( selectionGroup, _( "Multipoints" ), 9, multipoints, weapGroup );

			RAGECHECKBOX( selectionGroup, _( "Static pointscale" ), RagebotStaticPointscale, weapGroup );

			RAGEINTSLIDER_SHOWCONDITION( selectionGroup, _( "Head scale" ), RagebotHeadScale, weapGroup, 0, 100, RagebotStaticPointscale );
			RAGEINTSLIDER_SHOWCONDITION( selectionGroup, _( "Body scale" ), RagebotBodyScale, weapGroup, 0, 100, RagebotStaticPointscale );

			RAGECHECKBOX( selectionGroup, _( "Ignore limbs when moving" ), RagebotIgnoreLimbs, weapGroup );

			RAGECHECKBOX( selectionGroup, _( "Force baim after missed shots" ), RagebotForceBaimAfterX, weapGroup );
			RAGEINTSLIDER_SHOWCONDITION( selectionGroup, _( "Shot count" ), RagebotForceBaimAfterXINT, weapGroup, 1, 20, RagebotForceBaimAfterX );

			RAGEMULTICOMBO( selectionGroup, "Prefer baim", 3, preferBaim, weapGroup );

			selectionGroup->Register( _( "Force baim" ) );
			selectionGroup->Register( &Config::Get<keybind_t>( Vars.RagebotForceBaimKey ) );

			selectionGroup->Register( _( "Force safe point" ) );
			selectionGroup->Register( &Config::Get<keybind_t>( Vars.RagebotForceSafePointKey ) );

			selectionGroup->Register( _( "Force yaw safety" ) );
			selectionGroup->Register( &Config::Get<keybind_t>( Vars.RagebotForceYawSafetyKey ) );
		}

		auto exploitsGroup{ aimbot->AddGroup( "Exploits", 0.4f ) };
		{
			exploitsGroup->Register( _( "Doubletap" ), &Config::Get<bool>( Vars.ExploitsDoubletap ) );
			exploitsGroup->Register( &Config::Get<keybind_t>( Vars.ExploitsDoubletapKey ) );

			exploitsGroup->Register( _( "Lag peek" ), &Config::Get<bool>( Vars.ExploitsDoubletapDefensive ), SHOW( Config::Get<bool>( Vars.ExploitsDoubletap ) ) );
			exploitsGroup->Register( _( "Delay teleport" ), &Config::Get<bool>( Vars.ExploitsDoubletapExtended ), SHOW( Config::Get<bool>( Vars.ExploitsDoubletap ) && Config::Get<bool>( Vars.ExploitsDoubletapDefensive ) ) );
			exploitsGroup->Register( _( "Break animations" ), &Config::Get<bool>( Vars.ExploitsDefensiveBreakAnimations ), SHOW( Config::Get<bool>( Vars.ExploitsDoubletap ) && Config::Get<bool>( Vars.ExploitsDoubletapDefensive ) ) );
			exploitsGroup->Register( _( "Continuous in air" ), &Config::Get<bool>( Vars.ExploitsDefensiveInAir ), SHOW( Config::Get<bool>( Vars.ExploitsDoubletap ) && Config::Get<bool>( Vars.ExploitsDoubletapDefensive ) ) );

			//exploitsGroup->Register( _( "No fakelag on safety" ), &Config::Get<bool>( Vars.AntiaimSafeNoFakelag ), SHOW( Config::Get<bool>( Vars.ExploitsDoubletap ) && Config::Get<bool>( Vars.ExploitsDoubletapDefensive ) ) );
			exploitsGroup->Register( _( "Safe pitch" ), &Config::Get<int>( Vars.AntiaimSafePitch ), 5, safePitch, SHOW( Config::Get<bool>( Vars.ExploitsDoubletap ) && Config::Get<bool>( Vars.ExploitsDoubletapDefensive ) ) );
			exploitsGroup->Register( _( "Safe yaw randomisation" ), &Config::Get<int>( Vars.AntiaimSafeYawRandomisation ), 0, 100, SHOW( Config::Get<bool>( Vars.ExploitsDoubletap ) && Config::Get<bool>( Vars.ExploitsDoubletapDefensive ) ) );

			exploitsGroup->Register( _( "Hideshots" ), &Config::Get<bool>( Vars.ExploitsHideshots ) );
			//exploitsGroup->Register( _( "Break CBB" ), &Config::Get<bool>( Vars.ExploitsBreakCBB ) );
			exploitsGroup->Register( &Config::Get<keybind_t>( Vars.ExploitsHideshotsKey ) );
		}
	}

	auto antiaim{ &m_cTabs[ 0 ].m_vecSubtabs.emplace_back( "Anti-aim", 2 ) };
	{
		const auto generalGroup{ antiaim->AddGroup( _( "General" ), 1.f ) };
		{
			generalGroup->Register( _( "Enable" ), &Config::Get<bool>( Vars.AntiaimEnable ) );
			generalGroup->Register( _( "Pitch" ), &Config::Get<int>( Vars.AntiaimPitch ), 5, pitch );
			generalGroup->Register( _( "Yaw" ), &Config::Get<int>( Vars.AntiaimYaw ), 4, yaw );

			generalGroup->Register( _( "Yaw add" ), &Config::Get<int>( Vars.AntiaimYawAdd ), 5, yawAdd );


			generalGroup->Register( _( "Yaw range" ), &Config::Get<int>( Vars.AntiaimYawRange ), 2, 180, SHOW( Config::Get<int>( Vars.AntiaimYawAdd ) && Config::Get<int>( Vars.AntiaimYawAdd ) != 3 ) );

			auto& yawRange{ Config::Get<bool>( Vars.AntiaimYawRange ) };
			generalGroup->Register( _( "Yaw speed" ), &Config::Get<int>( Vars.AntiaimYawSpeed ), 1, 60, SHOW( Config::Get<int>( Vars.AntiaimYawAdd ) == 2 || Config::Get<int>( Vars.AntiaimYawAdd ) == 3 ) );

			generalGroup->Register( _( "Flick head" ), &Config::Get<bool>( Vars.AntiaimFlickHead ) );


			generalGroup->Register( _( "Invert flick side" ), true, SHOW( Config::Get<bool>( Vars.AntiaimFlickHead ) ) );
			generalGroup->Register( &Config::Get<keybind_t>( Vars.AntiaimFlickInvert ), SHOW( Config::Get<bool>( Vars.AntiaimFlickHead ) ) );
			generalGroup->Register( _( "Constant flick invert" ), &Config::Get<bool>( Vars.AntiaimConstantInvertFlick ), SHOW( Config::Get<bool>( Vars.AntiaimFlickHead ) ) );
			generalGroup->Register( _( "Flick amount" ), &Config::Get<int>( Vars.AntiaimFlickAdd ), 0, 180, SHOW( Config::Get<bool>( Vars.AntiaimFlickHead ) ) );
			generalGroup->Register( _( "Flick speed" ), &Config::Get<int>( Vars.AntiaimFlickSpeed ), 2, 40, SHOW( Config::Get<bool>( Vars.AntiaimFlickHead ) ) );

			generalGroup->Register( _( "At targets" ), &Config::Get<int>( Vars.AntiaimAtTargets ), 3, atTargets );

			generalGroup->Register( _( "Desync" ), &Config::Get<bool>( Vars.AntiaimDesync ) );

			generalGroup->Register( _( "Avoid overlap" ), &Config::Get<bool>( Vars.AntiaimTrickLBY ) );
			generalGroup->Register( _( "Constant desync invert" ), &Config::Get<bool>( Vars.AntiaimConstantInvert ), SHOW( Config::Get<bool>( Vars.AntiaimDesync ) && !Config::Get<bool>( Vars.AntiaimTrickLBY ) ) );
			generalGroup->Register( _( "Flip desync" ), true, SHOW( Config::Get<bool>( Vars.AntiaimDesync ) && !Config::Get<bool>( Vars.AntiaimConstantInvert ) ) );
			generalGroup->Register( &Config::Get<keybind_t>( Vars.AntiaimInvert ), SHOW( Config::Get<bool>( Vars.AntiaimDesync ) && !Config::Get<bool>( Vars.AntiaimConstantInvert ) ) );

			//generalGroup->Register( _( "Trick LBY" ), &Config::Get<bool>( Vars.AntiaimTrickLBY ) );
			generalGroup->Register( _( "Yaw add" ), &Config::Get<int>( Vars.AntiaimNormalYawAdd ), -180, 180 );
			generalGroup->Register( _( "Inverted yaw add" ), &Config::Get<int>( Vars.AntiaimInvertedYawAdd ), -180, 180 );

			generalGroup->Register( _( "Anti backstab" ), &Config::Get<bool>( Vars.AntiaimAntiBackStab ) );

			generalGroup->Register( _( "Freestanding type" ), &Config::Get<int>( Vars.AntiaimFreestanding ), 4, freestandingType );
			generalGroup->Register( _( "Freestanding key" ), true, SHOW( Config::Get<int>( Vars.AntiaimFreestanding ) ) );
			generalGroup->Register( &Config::Get<keybind_t>( Vars.AntiaimFreestandingKey ), SHOW( Config::Get<int>( Vars.AntiaimFreestanding ) ) );

			generalGroup->Register( _( "Manual direction" ), &Config::Get<bool>( Vars.AntiAimManualDir ) );

			generalGroup->Register( _( "Manual direction indicator" ), &Config::Get<bool>( Vars.AntiAimManualDirInd ) );
			generalGroup->Register( &Config::Get<Color>( Vars.AntiaimManualCol ) );
			generalGroup->Register( _( "Left" ) );
			generalGroup->Register( &Config::Get<keybind_t>( Vars.AntiaimLeft ) );
			Config::Get<keybind_t>( Vars.AntiaimLeft ).mode = EKeyMode::Toggle;
			generalGroup->Register( _( "Right" ) );
			generalGroup->Register( &Config::Get<keybind_t>( Vars.AntiaimRight ) );
			Config::Get<keybind_t>( Vars.AntiaimRight ).mode = EKeyMode::Toggle;
		}

		const auto fakelagGroup{ antiaim->AddGroup( _( "Fake-lag" ), 1.f ) };
		{
			fakelagGroup->Register( _( "Limit" ), &Config::Get<int>( Vars.AntiaimFakeLagLimit ), 0, 15 );
			fakelagGroup->Register( _( "Randomization" ), &Config::Get<int>( Vars.AntiaimFakeLagVariance ), 0, 100 );
			fakelagGroup->Register( _( "Choke peek" ), &Config::Get<bool>( Vars.AntiaimFakeLagInPeek ) );
			fakelagGroup->Register( _( "Break lagcompensation" ), &Config::Get<bool>( Vars.AntiaimFakeLagBreakLC ) );
		}
	}

	/* player */
	m_cTabs[ 1 ].m_vecSubtabs.emplace_back( _( "Enemy" ), 2 );
	m_cTabs[ 1 ].m_vecSubtabs.emplace_back( _( "Team" ), 2 );
	m_cTabs[ 1 ].m_vecSubtabs.emplace_back( _( "Local" ), 2 );
	{
		for ( int i{ }; i < 3; ++i ) {
			auto subTab{ &m_cTabs[ 1 ].m_vecSubtabs.at( i ) };
			auto ESPGroup{ subTab->AddGroup( "ESP", 1.f ) };
			{
				PlayerCheckbox( ESPGroup, _( "Enabled" ), VisEnable, i );
				PlayerCheckbox( ESPGroup, _( "Name" ), VisName, i );
				PlayerColorPicker( ESPGroup, VisNameCol, i );
				PlayerCheckbox( ESPGroup, _( "Box" ), VisBox, i );
				PlayerColorPicker( ESPGroup, VisBoxCol, i );
				PlayerCheckbox( ESPGroup, _( "Health" ), VisHealth, i );
				PlayerCheckbox( ESPGroup, _( "Color override" ), VisHealthOverride, i );
				PlayerColorPicker( ESPGroup, VisHealthCol, i );
				PlayerCheckbox( ESPGroup, _( "Ammo bar" ), VisAmmo, i );
				PlayerColorPicker( ESPGroup, VisAmmoCol, i );
				PlayerCheckbox( ESPGroup, _( "Out of FOV" ), VisOOF, i );
				PlayerColorPicker( ESPGroup, VisOOFCol, i );
				PlayerCheckbox( ESPGroup, _( "Skeleton" ), VisSkeleton, i );
				PlayerColorPicker( ESPGroup, VisSkeletonCol, i );
				PlayerCheckbox( ESPGroup, _( "Glow" ), VisGlow, i );
				PlayerColorPicker( ESPGroup, VisGlowCol, i );

				PlayerMultiCombo( ESPGroup, _( "Weapon" ), weapons, i, 2 );
				ESPGroup->Register( _( "Weapon color" ) );
				PlayerColorPicker( ESPGroup, VisWeapCol, i );

				if ( i == 2 )
					ESPGroup->Register( _( "Flags" ), 6, flagsLocal );
				else {
					PlayerMultiCombo( ESPGroup, _( "Flags" ), flags, i, 8 );
				}
			}

			auto chamGroup{ subTab->AddGroup( "Chams", 1.f ) };
			{
				PlayerCheckbox( chamGroup, _( "Visible" ), ChamVis, i );
				PlayerColorPicker( chamGroup, ChamVisCol, i );
				PlayerCombo( chamGroup, _( "Visible material" ), ChamMatVisible, i, materials, 5 );

				PlayerCheckbox( chamGroup, _( "Hidden" ), ChamHid, i );
				PlayerColorPicker( chamGroup, ChamHidCol, i );
				PlayerCombo( chamGroup, _( "Hidden material" ), ChamMatInvisible, i, materials, 5 );

				PlayerCheckbox( chamGroup, _( "Overlay" ), ChamDouble, i );
				PlayerColorPicker( chamGroup, ChamDoubleCol, i );
				PlayerCheckbox( chamGroup, _( "Overlay ignore Z" ), ChamDoubleZ, i );

				PlayerCombo( chamGroup, _( "Double layer material" ), ChamDoubleMat, i, materials, 5 );

				if ( i == 0 ) {
					chamGroup->Register( _( "Backtrack chams" ), &Config::Get<bool>( Vars.ChamBacktrack ) );
					chamGroup->Register( &Config::Get<Color>( Vars.ChamBacktrackCol ) );
					chamGroup->Register( _( "Backtrack chams material" ), &Config::Get<int>( Vars.ChamBacktrackMat ), 5, materials );

					chamGroup->Register( _( "Hitbox visualisation" ), &Config::Get<bool>( Vars.MiscHitMatrix ) );
					chamGroup->Register( &Config::Get<Color>( Vars.MiscHitMatrixCol ) );

					{
						chamGroup->Register( _( "Hitbox visualisation type" ), &Config::Get<int>( Vars.MiscShotVisualizationType ), 2, hitboxType, SHOW( Config::Get<bool>( Vars.MiscHitMatrix ) ) );
						chamGroup->Register( _( "Hitbox visualisation hidden" ), &Config::Get<bool>( Vars.MiscHitMatrixXQZ ), SHOW( Config::Get<bool>( Vars.MiscHitMatrix ) ) );
						chamGroup->Register( _( "Onshot chams material" ), &Config::Get<int>( Vars.MiscHitMatrixMat ), 5, materials, SHOW( Config::Get<bool>( Vars.MiscHitMatrix ) ) );

						chamGroup->Register( _( "Hitbox visualisation time" ), &Config::Get<float>( Vars.MiscHitMatrixTime ), 0.5f, 10.f, SHOW( Config::Get<bool>( Vars.MiscHitMatrix ) ) );
					}

				}

				if ( i == 2 ) {
					chamGroup->Register( _( "Desync chams" ), &Config::Get<bool>( Vars.ChamDesync ) );
					chamGroup->Register( &Config::Get<Color>( Vars.ChamDesyncCol ) );
					chamGroup->Register( _( "Desync chams material" ), &Config::Get<int>( Vars.ChamDesyncMat ), 5, materials );

				}

				PlayerIntSlider( chamGroup, _( "Glow strength" ), ChamGlowStrength, i, 0.f, 100.f );

				if ( i == 2 )
					chamGroup->Register( _( "Transparency in scope" ), &Config::Get<int>( Vars.ChamLocalTransparencyInScope ), 0, 100 );
			}
		}
	}

	const auto visualsSubtab{ &m_cTabs[ 2 ].m_vecSubtabs.emplace_back( _( "Visuals" ), 2 ) };
	{
		const auto ESPGroup{ visualsSubtab->AddGroup( _( "General" ), 1.f ) };
		{
			ESPGroup->Register( _( "Removals" ), 6, removals );

			ESPGroup->Register( _( "Thirdperson" ), &Config::Get<bool>( Vars.VisThirdPerson ) );
			ESPGroup->Register( &Config::Get<keybind_t>( Vars.VisThirdPersonKey ) );
			ESPGroup->Register( _( "Thirdperson distance" ), &Config::Get<int>( Vars.VisThirdPersonDistance ), 30, 250 );

			ESPGroup->Register( _( "Second Scope Zoom" ), &Config::Get<int>( Vars.SecondZoomAmt ), 0.f, 100.f, SHOW( Config::Get<bool>( Vars.RemovalZoom ) ) );

			ESPGroup->Register( _( "Penetration crosshair" ), &Config::Get<bool>( Vars.VisPenetrationCrosshair ) );

			ESPGroup->Register( _( "Grenade prediction" ), &Config::Get<bool>( Vars.VisGrenadePrediction ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisGrenadePredictionCol ) );

			ESPGroup->Register( _( "Grenade warning" ), &Config::Get<bool>( Vars.VisGrenadeWarning ) );

			ESPGroup->Register( _( "Dropped weapons" ), &Config::Get<bool>( Vars.VisDroppedWeapon ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisDroppedWeaponCol ) );
			ESPGroup->Register( _( "Team grenades" ), &Config::Get<bool>( Vars.VisGrenadesTeam ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisGrenadesTeamCol ) );
			ESPGroup->Register( _( "Enemy grenades" ), &Config::Get<bool>( Vars.VisGrenadesEnemy ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisGrenadesEnemyCol ) );
			ESPGroup->Register( _( "Planted Bomb" ), &Config::Get<bool>( Vars.VisBomb ) );
			ESPGroup->Register( _( "Local client bullet impacts" ), &Config::Get<bool>( Vars.VisClientBulletImpacts ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisClientBulletImpactsCol ) );
			ESPGroup->Register( _( "Local server bullet impacts" ), &Config::Get<bool>( Vars.VisLocalBulletImpacts ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisLocalBulletImpactsCol ) );
			ESPGroup->Register( _( "Enemy bullet impacts" ), &Config::Get<bool>( Vars.VisServerBulletImpacts ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisServerBulletImpactsCol ) );

			ESPGroup->Register( _( "Local bullet tracers" ), &Config::Get<bool>( Vars.VisLocalBulletTracers ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisLocalBulletTracersCol ) );

			ESPGroup->Register( _( "Enemy bullet tracers" ), &Config::Get<bool>( Vars.VisOtherBulletTracers ) );
			ESPGroup->Register( &Config::Get<Color>( Vars.VisOtherBulletTracersCol ) );

			ESPGroup->Register( _( "Bullet tracers type" ), &Config::Get<int>( Vars.VisBulletTracersType ), 2, tracerType );
		}

		const auto modificationsGroup{ visualsSubtab->AddGroup( _( "Modifications" ), 1.f ) };
		{
			modificationsGroup->Register( _( "Player interpolation" ), &Config::Get<int>( Vars.VisPlayerInterpolation ), 0, 16 );

			modificationsGroup->Register( _( "Full bright" ), &Config::Get<bool>( Vars.WorldFullbright ) );
			modificationsGroup->Register( _( "World Modulation" ), &Config::Get<bool>( Vars.WorldModulation ) );
			modificationsGroup->Register( &Config::Get<Color>( Vars.WorldModulationCol ), false );

			modificationsGroup->Register( _( "Skybox Color Modulation" ), &Config::Get<bool>( Vars.VisWorldSkyboxMod ) );
			modificationsGroup->Register( &Config::Get<Color>( Vars.WorldSkyboxCol ), false );

			modificationsGroup->Register( _( "Prop Color Modulation" ), &Config::Get<bool>( Vars.VisWorldPropMod ) );
			modificationsGroup->Register( &Config::Get<Color>( Vars.VisWorldPropCol ) );

			modificationsGroup->Register( _( "Ambient lighting" ), &Config::Get<bool>( Vars.WorldAmbientLighting ) );
			modificationsGroup->Register( &Config::Get<Color>( Vars.WorldAmbientLightingCol ), false );

			modificationsGroup->Register( _( "Bloom" ), &Config::Get<bool>( Vars.VisWorldBloom ) );

			modificationsGroup->Register( _( "Scale" ), &Config::Get<int>( Vars.VisWorldBloomScale ), 0.f, 750.f, SHOW( Config::Get<bool>( Vars.VisWorldBloom ) ) );
			modificationsGroup->Register( _( "Ambience" ), &Config::Get<int>( Vars.VisWorldBloomAmbience ), 0.f, 200.f, SHOW( Config::Get<bool>( Vars.VisWorldBloom ) ) );
			modificationsGroup->Register( _( "Exposure" ), &Config::Get<int>( Vars.VisWorldBloomExposure ), 0.f, 200.f, SHOW( Config::Get<bool>( Vars.VisWorldBloom ) ) );

			modificationsGroup->Register( _( "Weather" ), &Config::Get<int>( Vars.VisWorldWeather ), 6, weatherType );
			modificationsGroup->Register( _( "Fog" ), &Config::Get<bool>( Vars.VisWorldFog ) );
			modificationsGroup->Register( &Config::Get<Color>( Vars.VisWorldFogCol ) );

			modificationsGroup->Register( _( "Fog Distance" ), &Config::Get<int>( Vars.VisWorldFogDistance ), 0.f, 3000.f, SHOW( Config::Get<bool>( Vars.VisWorldFog ) ) );
			modificationsGroup->Register( _( "Fog Density" ), &Config::Get<int>( Vars.VisWorldFogDensity ), 0.f, 100.f, SHOW( Config::Get<bool>( Vars.VisWorldFog ) ) );
			modificationsGroup->Register( _( "Fog HDR" ), &Config::Get<int>( Vars.VisWorldFogHDR ), 0.f, 100.f, SHOW( Config::Get<bool>( Vars.VisWorldFog ) ) );

			modificationsGroup->Register( _( "Skybox changer" ), &Config::Get<int>( Vars.VisWorldSkybox ), 22, skies );

			modificationsGroup->Register( &Config::Get<std::string>( Vars.VisWorldSkyboxCustom ), SHOW( Config::Get<int>( Vars.VisWorldSkybox ) == 21 ) );

			modificationsGroup->Register( _( "Player model changer T" ), &Config::Get<int>( Vars.MiscPlayerModelT ), 13, modelsForChange );

			modificationsGroup->Register( _( "Player model changer CT" ), &Config::Get<int>( Vars.MiscPlayerModelCT ), 13, modelsForChange );
		}
	}

	const auto viewModelSubtab{ &m_cTabs[ 2 ].m_vecSubtabs.emplace_back( _( "Viewmodel" ), 2 ) };
	{
		const auto skinChangerGroup{ viewModelSubtab->AddGroup( _( "Skinchanger" ), 1.f ) };
		{
			skinChangerGroup->Register( _( "Coming soon!" ) );

			/*static std::vector<ComboElement_t> arrayOfData( k_knife_names.size( ) );

			std::transform( k_knife_names.begin( ), k_knife_names.end( ), arrayOfData.begin( ),
				[ ]( const WeaponName_t& v ) -> ComboElement_t {
					return ComboElement_t( v.name );
				} );

			if ( k_knife_names.size( ) )
				skinChangerGroup->Register( "Knife changer", &Config::Get<int>( Vars.SkinKnifeChanger ), k_knife_names.size( ), arrayOfData.data( ) );*/
		}

		const auto chamsGroup{ viewModelSubtab->AddGroup( _( "Chams" ), 1.f ) };
		{
			chamsGroup->Register( _( "Hand chams" ), &Config::Get<bool>( Vars.ChamHand ) );
			chamsGroup->Register( &Config::Get<Color>( Vars.ChamHandCol ) );
			chamsGroup->Register( _( "Hand chams material" ), &Config::Get<int>( Vars.ChamHandMat ), 5, materials );
			chamsGroup->Register( _( "Hand overlay" ), &Config::Get<bool>( Vars.ChamHandOverlay ) );

			chamsGroup->Register( &Config::Get<Color>( Vars.ChamHandOverlayCol ) );
			chamsGroup->Register( _( "Hand overlay material" ), &Config::Get<int>( Vars.ChamHandOverlayMat ), 5, materials );

			chamsGroup->Register( _( "Hand glow strength" ), &Config::Get<int>( Vars.ChamHandGlow ), 0.f, 100.f );

			chamsGroup->Register( _( "Weapon chams" ), &Config::Get<bool>( Vars.ChamWeapon ) );
			chamsGroup->Register( &Config::Get<Color>( Vars.ChamWeaponCol ) );
			chamsGroup->Register( _( "Weapon chams material" ), &Config::Get<int>( Vars.ChamWeaponMat ), 5, materials );

			chamsGroup->Register( _( "Weapon overlay" ), &Config::Get<bool>( Vars.ChamWeaponOverlay ) );
			chamsGroup->Register( &Config::Get<Color>( Vars.ChamWeaponOverlayCol ) );
			chamsGroup->Register( _( "Weapon overlay material" ), &Config::Get<int>( Vars.ChamWeaponOverlayMat ), 5, materials );

			chamsGroup->Register( _( "Weapon glow strength" ), &Config::Get<int>( Vars.ChamWeaponGlow ), 0.f, 100.f );
		}
	}

	auto miscSubTab{ &m_cTabs[ 3 ].m_vecSubtabs.emplace_back( "Misc", 2 ) };
	{
		auto generalGroup{ miscSubTab->AddGroup( "General", 1.f ) };
		{
			/* propaganda section */
			generalGroup->Register( _( "Watermark" ), &Config::Get<bool>( Vars.MiscWatermark ) );
			generalGroup->Register( _( "Accent color" ) );
			//generalGroup->Register( _( "Accent Color" ), Menu::AccentCol, false );
			generalGroup->Register( _( "Clantag" ), &Config::Get<bool>( Vars.MiscClantag ) );

			generalGroup->Register( _( "Keybinds list" ), &Config::Get<bool>( Vars.MiscKeybindList ) );
			generalGroup->Register( _( "Aspect ratio" ), &Config::Get<bool>( Vars.MiscAspectRatio ) );

			generalGroup->Register( _( "Ratio" ), &Config::Get<float>( Vars.MiscAspectRatioAmt ), 0.02f, 5.f, SHOW( Config::Get<bool>( Vars.MiscAspectRatio ) ) );

			generalGroup->Register( _( "Field of view" ), &Config::Get<int>( Vars.MiscFOV ), 0, 70 );

			generalGroup->Register( _( "Only show record anims" ), &Config::Get<bool>( Vars.VisRecordAnims ) );
			generalGroup->Register( _( "Local animation sway" ), &Config::Get<int>( Vars.MiscLocalAnimSway ), 0, 40 );

			generalGroup->Register( _( "Preserve killfeed" ), &Config::Get<bool>( Vars.MiscPreserveKillfeed ) );
			generalGroup->Register( _( "Force crosshair" ), &Config::Get<bool>( Vars.MiscForceCrosshair ) );
			generalGroup->Register( _( "Fake ping" ), &Config::Get<bool>( Vars.MiscFakePing ) );
			generalGroup->Register( _( "Weapon volume" ), &Config::Get<int>( Vars.MiscWeaponVolume ), 0, 100 );

			static MultiElement_t hitMarkers[ 2 ]{
				{ &Config::Get<bool>( Vars.MiscScreenHitmarker ), "Screen" },
				{ &Config::Get<bool>( Vars.MiscWorldHitmarker ), "World" }
			};

			generalGroup->Register( _( "Hit marker" ), 2, hitMarkers );

			{
				generalGroup->Register( _( "Screen hit marker size" ), &Config::Get<int>( Vars.MiscScreenHitmarkerSize ), 1.f, 20.f, SHOW( Config::Get<bool>( Vars.MiscScreenHitmarker ) ) );
				generalGroup->Register( _( "Screen hit marker gap" ), &Config::Get<int>( Vars.MiscScreenHitmarkerGap ), 0.f, 20.f, SHOW( Config::Get<bool>( Vars.MiscScreenHitmarker ) ) );
			}

			{
				generalGroup->Register( _( "World hit marker size" ), &Config::Get<int>( Vars.MiscWorldHitmarkerSize ), 1, 20, SHOW( Config::Get<bool>( Vars.MiscWorldHitmarker ) ) );
				generalGroup->Register( _( "World hit marker gap" ), &Config::Get<int>( Vars.MiscWorldHitmarkerGap ), 0, 20, SHOW( Config::Get<bool>( Vars.MiscWorldHitmarker ) ) );
			}

			generalGroup->Register( _( "Damage marker" ), &Config::Get<bool>( Vars.MiscDamageMarker ) );
			generalGroup->Register( &Config::Get<Color>( Vars.MiscDamageMarkerCol ) );

			static ComboElement_t hitsounds[ 3 ]{ _( "None" ), _( "Metallic" ), _( "Custom" ) };
			generalGroup->Register( _( "Hitsound" ), &Config::Get<int>( Vars.MiscHitSound ), 3, hitsounds );

			generalGroup->Register( _( "Custom hitsound" ), true, SHOW( Config::Get<int>( Vars.MiscHitSound ) == 2 ) );
			generalGroup->Register( &Config::Get<std::string>( Vars.MiscCustomHitSound ), SHOW( Config::Get<int>( Vars.MiscHitSound ) == 2 ) );

			generalGroup->Register( _( "Auto buy" ), &Config::Get<bool>( Vars.MiscBuyBot ) );

			{
				static MultiElement_t buyother[ 7 ]{
					{ &Config::Get<bool>( Vars.MiscBuyBotOtherArmor ), _( "Armor" ) },
					{ &Config::Get<bool>( Vars.MiscBuyBotOtherTaser ), _( "Taser" ) },
					{ &Config::Get<bool>( Vars.MiscBuyBotOtherKit ), _( "Kit" ) },
					{ &Config::Get<bool>( Vars.MiscBuyBotOtherSmoke ), _( "Smoke" ) },
					{ &Config::Get<bool>( Vars.MiscBuyBotOtherNade ), _( "HE Grenade" ) },
					{ &Config::Get<bool>( Vars.MiscBuyBotOtherFlashbang ), _( "Flashbang" ) },
					{ &Config::Get<bool>( Vars.MiscBuyBotOtherMolotov ), _( "Molotov" ) }
				};

				static ComboElement_t primary[ 6 ]{ _( "None" ), _( "Autosniper" ), _( "SSG-08" ), _( "awp" ), _( "negev" ), _( "ak-47/m4" ) };
				static ComboElement_t secondary[ 5 ]{ _( "None" ), _( "Deagle/r8" ), _( "Dualies" ), _( "USP/Glock" ), _( "Tec9/Fiveseven" ) };

				generalGroup->Register( "Primary weapons", &Config::Get<int>( Vars.MiscBuyBotPrimary ), 6, primary, SHOW( Config::Get<bool>( Vars.MiscBuyBot ) ) );
				generalGroup->Register( "Secondary weapons", &Config::Get<int>( Vars.MiscBuyBotSecondary ), 5, secondary, SHOW( Config::Get<bool>( Vars.MiscBuyBot ) ) );
				generalGroup->Register( _( "Other" ), 7, buyother, SHOW( Config::Get<bool>( Vars.MiscBuyBot ) ) );
			}

			generalGroup->Register( _( "Reset menu size" ), ResetMenuSize );
			generalGroup->Register( _( "Unload" ), Unload );
		}

		auto movementGroup{ miscSubTab->AddGroup( "Movement", 0.6f ) };
		{
			movementGroup->Register( _( "Bunnyhop" ), &Config::Get<bool>( Vars.MiscBunnyhop ) );
			movementGroup->Register( _( "Autostrafer" ), &Config::Get<bool>( Vars.MiscAutostrafe ) );

			movementGroup->Register( _( "Autostrafer speed" ), &Config::Get<int>( Vars.MiscAutostrafeSpeed ), 0.f, 100.f, SHOW( Config::Get<bool>( Vars.MiscAutostrafe ) ) );

			movementGroup->Register( _( "Infinite Stamina" ), &Config::Get<bool>( Vars.MiscInfiniteStamina ) );
			movementGroup->Register( _( "Quick stop" ), &Config::Get<bool>( Vars.MiscQuickStop ) );
			movementGroup->Register( _( "Slide walk" ), &Config::Get<bool>( Vars.MiscSlideWalk ) );
			movementGroup->Register( _( "Slow walk" ), &Config::Get<bool>( Vars.MiscSlowWalk ) );
			movementGroup->Register( &Config::Get<keybind_t>( Vars.MiscSlowWalkKey ) );

			movementGroup->Register( _( "Fake duck" ), &Config::Get<bool>( Vars.MiscFakeDuck ), SHOW( Config::Get<bool>( Vars.MiscInfiniteStamina ) ) );
			movementGroup->Register( &Config::Get<keybind_t>( Vars.MiscFakeDuckKey ), SHOW( Config::Get<bool>( Vars.MiscInfiniteStamina ) ) );

			movementGroup->Register( _( "Auto retreat" ), &Config::Get<bool>( Vars.MiscAutoPeek ) );
			movementGroup->Register( &Config::Get<keybind_t>( Vars.MiscAutoPeekKey ) );
			movementGroup->Register( _( "Auto retreat on release" ), &Config::Get<bool>( Vars.MiscAutoPeekOnRelease ), SHOW( Config::Get<bool>( Vars.MiscAutoPeek ) ) );
			movementGroup->Register( _( "Auto retreat color" ) );
			movementGroup->Register( &Config::Get<Color>( Vars.MiscAutoPeekCol ), true );
		}

		auto betaGroup{ miscSubTab->AddGroup( "Beta testing", 0.4f ) };
		{
			betaGroup->Register( _( "Disable yaw extrapolation" ), &Config::Get<bool>( Vars.DBGLC1 ) );
			//betaGroup->Register( _( "Extrapolate players" ), &Config::Get<bool>( Vars.DBGExtrap ) );
			//betaGroup->Register( _( "Valve simtime fix" ), &Config::Get<bool>( Vars.DBGNoPingReducer ) );
			//betaGroup->Register( _( "Record options" ), 2, recordOptions );
			//betaGroup->Register( _( "Smoother gameplay" ), &Config::Get<bool>( Vars.DBGNoLatch ) );
		}
	}

	auto configSubtab{ &m_cTabs[ 4 ].m_vecSubtabs.emplace_back( "Configs", 1 ) };
	{
		auto configGroup{ configSubtab->AddGroup( "Configurations", 1.f ) };

		configGroup->Register( &selectedConfig );
		configGroup->Register( &typedConfig );
		configGroup->Register( "Refresh list", RefreshConfig );
		configGroup->Register( "Create", CreateConfig );
		configGroup->Register( "Remove", RemoveConfig );

		configGroup->Register( "Load", LoadConfig );
		configGroup->Register( "Save", SaveConfig );
	}
}

// L func
void Menu::LerpToCol( Color& col, Color to ) {
	col = Color(
		( to[ 0 ] > col[ 0 ] ? uint8_t( std::ceil( std::min( to[ 0 ], uint8_t( ( to[ 0 ] - col[ 0 ] ) * ANIMATION_SPEED + col[ 0 ] ) ) ) ) : uint8_t( std::floor( std::max( to[ 0 ], uint8_t( ( to[ 0 ] - col[ 0 ] ) * ANIMATION_SPEED + col[ 0 ] ) ) ) ) ),
		( to[ 1 ] > col[ 1 ] ? uint8_t( std::ceil( std::min( to[ 1 ], uint8_t( ( to[ 1 ] - col[ 1 ] ) * ANIMATION_SPEED + col[ 1 ] ) ) ) ) : uint8_t( std::floor( std::max( to[ 1 ], uint8_t( ( to[ 1 ] - col[ 1 ] ) * ANIMATION_SPEED + col[ 1 ] ) ) ) ) ),
		( to[ 2 ] > col[ 2 ] ? uint8_t( std::ceil( std::min( to[ 2 ], uint8_t( ( to[ 2 ] - col[ 2 ] ) * ANIMATION_SPEED + col[ 2 ] ) ) ) ) : uint8_t( std::floor( std::max( to[ 2 ], uint8_t( ( to[ 2 ] - col[ 2 ] ) * ANIMATION_SPEED + col[ 2 ] ) ) ) ) ),
		( to[ 3 ] > col[ 3 ] ? uint8_t( std::ceil( std::min( to[ 3 ], uint8_t( ( to[ 3 ] - col[ 3 ] ) * ANIMATION_SPEED + col[ 3 ] ) ) ) ) : uint8_t( std::floor( std::max( to[ 3 ], uint8_t( ( to[ 3 ] - col[ 3 ] ) * ANIMATION_SPEED + col[ 3 ] ) ) ) ) )
	);
}

void Menu::RenderElements( ) {
	const auto maxSize{ Menu::m_vecSize.y - 80 };
	const auto sizeDenom{ ( maxSize / 5 ) };


	auto& activeTab{ m_cTabs[ m_iSelectedTab ] };

	const auto backupSubtab{ activeTab.m_pSelectedSubtab };

	if ( !activeTab.m_pSelectedSubtab )
		activeTab.m_pSelectedSubtab = &activeTab.m_vecSubtabs.front( );

	const auto tabSize{ Render::GetTextSize( _( "A" ), Fonts::MenuTabs ) };

	// tabs
	{
		for ( int i{ }; i < 5; ++i ) {
			const auto pos{ Menu::m_vecPos + Vector2D( Math::Lerp( m_cTabs[ i ].m_flAnimation, static_cast< float >( BAR_SIZE / 2 ), tabSize.x / 2 + 10 ), 60 + sizeDenom * i ) };

			const auto hovered{ Inputsys::hovered( pos - Vector2D( tabSize.x / 2 + BAR_SIZE / 2, 0 ),  Vector2D{BAR_SIZE, tabSize.y } ) };
			if ( hovered && ( Inputsys::pressed( VK_LBUTTON ) || Inputsys::pressed( VK_RBUTTON ) || Inputsys::pressed( VK_MBUTTON ) ) ) {
				m_iSelectedTab = i;
				if ( Inputsys::pressed( VK_RBUTTON ) && m_cTabs[ i ].m_vecSubtabs.size( ) > 1 )
					m_cTabs[ i ].m_pSelectedSubtab = &m_cTabs[ i ].m_vecSubtabs.at( 1 );
				else if ( Inputsys::pressed( VK_MBUTTON ) && m_cTabs[ i ].m_vecSubtabs.size( ) > 2 )
					m_cTabs[ i ].m_pSelectedSubtab = &m_cTabs[ i ].m_vecSubtabs.at( 2 );
				else
					m_cTabs[ i ].m_pSelectedSubtab = &m_cTabs[ i ].m_vecSubtabs.front( );
			}

			LerpToCol( m_cTabs[ i ].m_cColor, m_iSelectedTab == i ? ( i % 2 == 0 ? ACCENT : ACCENT2 ) : hovered ? Color( 120, 120, 120 ) : Color( 90, 90, 90 ) );

			if ( m_iSelectedTab == i )
				m_cTabs[ i ].m_flAnimation = Math::Interpolate( m_cTabs[ i ].m_flAnimation, 1.f, ANIMATION_SPEED );
			else
				m_cTabs[ i ].m_flAnimation = Math::Interpolate( m_cTabs[ i ].m_flAnimation, 0.f, ANIMATION_SPEED );

			if ( m_cTabs[ i ].m_flAnimation <= 0.005f )
				m_cTabs[ i ].m_flAnimation = 0.f;
			else if ( m_cTabs[ i ].m_flAnimation >= 0.995f )
				m_cTabs[ i ].m_flAnimation = 1.f;

			Render::Text( Fonts::MenuTabs, pos, m_cTabs[ i ].m_cColor, FONT_CENTER, m_cTabs[ i ].m_szName );
		}
	}

	auto& newActiveTab = m_cTabs[ m_iSelectedTab ];

	if ( newActiveTab.m_vecSubtabs.size( ) ) {
		int i{ };
		const char* longest{ };
		for ( auto& subtab : newActiveTab.m_vecSubtabs ) {
			if ( !longest || strlen( subtab.m_szName ) > strlen( longest ) )
				longest = subtab.m_szName;
		}


		if ( longest ) {
			// yikes... but i had to to make it more bold
			Render::Gradient( Menu::m_vecPos.x, Menu::m_vecPos.y + 60 + sizeDenom * m_iSelectedTab, BAR_SIZE, tabSize.y, OUTLINE_DARK.Set<COLOR_A>( 0 ), OUTLINE_DARK.Set<COLOR_A>( newActiveTab.m_flAnimation * 255.f ), true );
			Render::Gradient( Menu::m_vecPos.x, Menu::m_vecPos.y + 60 + sizeDenom * m_iSelectedTab, BAR_SIZE, tabSize.y, OUTLINE_DARK.Set<COLOR_A>( 0 ), OUTLINE_DARK.Set<COLOR_A>( newActiveTab.m_flAnimation * 255.f ), true );
			Render::Gradient( Menu::m_vecPos.x, Menu::m_vecPos.y + 60 + sizeDenom * m_iSelectedTab, BAR_SIZE, tabSize.y, OUTLINE_DARK.Set<COLOR_A>( 0 ), OUTLINE_DARK.Set<COLOR_A>( newActiveTab.m_flAnimation * 255.f ), true );
			if ( Menu::m_bOpened ) {
				Render::Gradient( Menu::m_vecPos.x, Menu::m_vecPos.y + 60 + sizeDenom * m_iSelectedTab, BAR_SIZE, tabSize.y, OUTLINE_DARK.Set<COLOR_A>( 0 ), OUTLINE_DARK.Set<COLOR_A>( newActiveTab.m_flAnimation * 255.f ), true );
				Render::Gradient( Menu::m_vecPos.x, Menu::m_vecPos.y + 60 + sizeDenom * m_iSelectedTab, BAR_SIZE, tabSize.y, OUTLINE_DARK.Set<COLOR_A>( 0 ), OUTLINE_DARK.Set<COLOR_A>( newActiveTab.m_flAnimation * 255.f ), true );
				Render::Gradient( Menu::m_vecPos.x, Menu::m_vecPos.y + 60 + sizeDenom * m_iSelectedTab, BAR_SIZE, tabSize.y, OUTLINE_DARK.Set<COLOR_A>( 0 ), OUTLINE_DARK.Set<COLOR_A>( newActiveTab.m_flAnimation * 255.f ), true );
			}
		}

		for ( auto& subtab : newActiveTab.m_vecSubtabs ) {
			if ( !longest || strlen( subtab.m_szName ) > strlen( longest ) )
				longest = subtab.m_szName;

			const auto textSize{ Render::GetTextSize( subtab.m_szName, Fonts::Menu ) };

			const auto pos{ Menu::m_vecPos + Vector2D( Math::Lerp( newActiveTab.m_flAnimation, BAR_SIZE / 2 + 10, BAR_SIZE - 10 ), tabSize.y / 2 - ( newActiveTab.m_vecSubtabs.size( ) * 16 ) / 2.f + 60 + sizeDenom * m_iSelectedTab + i * 16 ) };

			const auto hovered{ Inputsys::hovered( pos - Vector2D( textSize.x, 0 ), Vector2D( textSize.x, 16 ) ) };
			if ( hovered && Inputsys::pressed( VK_LBUTTON ) && activeTab.m_szName == newActiveTab.m_szName )
				newActiveTab.m_pSelectedSubtab = &subtab;

			LerpToCol( subtab.m_cColor, newActiveTab.m_pSelectedSubtab == &subtab ? ( m_iSelectedTab % 2 ? ( i % 2 == 0 ? ACCENT2 : ACCENT ) : ( i % 2 == 0 ? ACCENT : ACCENT2 ) ) : hovered ? HOVERED_ELEMENT : DIM_ELEMENT );

			Render::Text( Fonts::Menu, pos, subtab.m_cColor.Set<COLOR_A>( newActiveTab.m_flAnimation * 255 ), FONT_RIGHT, subtab.m_szName );
			i++;
		}
	}


	m_vecDrawPos = m_vecPos + Vector2D( BAR_SIZE + MARGIN, MARGIN );

	if ( newActiveTab.m_pSelectedSubtab->m_vecGroups.size( ) ) {
		if ( backupSubtab != newActiveTab.m_pSelectedSubtab ) {
			for ( auto& group : newActiveTab.m_pSelectedSubtab->m_vecGroups )
				group.Reset( );
		}

		int i{ };
		for ( auto& group : newActiveTab.m_pSelectedSubtab->m_vecGroups ) {
			i++;
			group.Render( i, newActiveTab.m_pSelectedSubtab->m_vecGroups );
		}
	}

	Render::FilledRoundedBox( m_vecPos + Vector2D{ BAR_SIZE + MARGIN }, m_vecSize - Vector2D{ BAR_SIZE + MARGIN }, 5, 5, BACKGROUND.Set<COLOR_A>( Menu::m_pFocusItem.m_flFocusAnim * 0.7f * 255.f ) );

	if ( Menu::m_pFocusItem.m_pItem ) {
		auto& element{ *Menu::m_pFocusItem.m_pItem };
		Menu::m_pFocusItem.m_flFocusAnim = Math::Interpolate( Menu::m_pFocusItem.m_flFocusAnim, m_pFocusItem.m_bFadeOut ? 0.f : 1.f, ANIMATION_SPEED );

		element.RenderFocus( );
	}
	else
		Menu::m_pFocusItem.m_flFocusAnim = Math::Interpolate( Menu::m_pFocusItem.m_flFocusAnim, 0.f, ANIMATION_SPEED );

	if ( Menu::m_pFocusItem.m_flFocusAnim <= 0.01f )
		Menu::m_pFocusItem.m_pItem = nullptr;
}