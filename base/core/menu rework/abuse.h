#pragma once
// HAHAHHAAHHAHAH FUCKING CRY ABOUT IT I LOOOOOOOOVE ABUSING MACROS HAHAHAHHAHAHAHHA
#define Convert2PType( name, type ) Vars.##name##type
#define BoolConvert2PType( name, type ) Config::Get<bool>( Convert2PType( name, type ) )
#define IntConvert2PType( name, type ) Config::Get<int>( Convert2PType( name, type ) )
#define FloatConvert2PType( name, type ) Config::Get<float>( Convert2PType( name, type ) )
#define ColorConvert2PType( name, type ) Config::Get<Color>( Convert2PType( name, type ) )


#define PlayerCheckbox( group, name, varname, type ) switch ( type ) { case 2: group->Register( name, &BoolConvert2PType( varname, Local ) );break; case 1: group->Register( name, &BoolConvert2PType( varname, Team ) );break; case 0: group->Register( name, &BoolConvert2PType( varname, Enemy ) );break; }

#define PlayerColorPicker( group, varname, type ) switch ( type ) { case 2: group->Register( &ColorConvert2PType( varname, Local ) );break; case 1: group->Register( &ColorConvert2PType( varname, Team ) );break; case 0: group->Register( &ColorConvert2PType( varname, Enemy ) );break; }

#define PlayerCombo( group, name, varname, type, items, size ) switch ( type ) { case 2: group->Register( name, &IntConvert2PType( varname, Local ), size, items );break; case 1: group->Register( name, &IntConvert2PType( varname, Team ), size, items );break; case 0: group->Register( name, &IntConvert2PType( varname, Enemy ), size, items );break; }

#define PlayerMultiCombo( group, name, items, type, size ) switch ( type ) { case 2: group->Register( name, size, items##Local );break; case 1: group->Register( name, size, items##Team );break; case 0: group->Register( name, size, items##Enemy );break; }

#define PlayerIntSlider( group, name, varname, type, min, max ) switch ( type ) { case 2: group->Register( name, &IntConvert2PType( varname, Local ), min, max );break; case 1: group->Register( name, &IntConvert2PType( varname, Team ), min, max );break; case 0: group->Register( name, &IntConvert2PType( varname, Enemy ), min, max );break; }

#define PlayerFloatSlider( group, name, varname, type, min, max ) switch ( type ) { case 2: group->Slider( name, FloatConvert2PType( varname, Local ), min, max );break; case 1: group->Slider( name, FloatConvert2PType( varname, Team ), min, max );break; case 0: group->Slider( name, FloatConvert2PType( varname, Enemy ), min, max );break; }

#define SHOW( cond ) [ ]( ) { return cond; }

/* rage macros cuz fuck this cfg sys */
#define RAGECHECKBOX( group, name, varname, type ) \
	group->Register( name, &BoolConvert2PType( varname, Pistol ), SHOW( weapGroup == 0 ) ); \
	group->Register( name, &BoolConvert2PType( varname, HeavyPistol ), SHOW( weapGroup == 1 ) ); \
	group->Register( name, &BoolConvert2PType( varname, SMG ), SHOW( weapGroup == 2 ) ); \
	group->Register( name, &BoolConvert2PType( varname, Rifle ), SHOW( weapGroup == 3 ) ); \
	group->Register( name, &BoolConvert2PType( varname, Shotgun ), SHOW( weapGroup == 4 ) );\
	group->Register( name, &BoolConvert2PType( varname, AWP ), SHOW( weapGroup == 5 ) ); \
	group->Register( name, &BoolConvert2PType( varname, Scout ), SHOW( weapGroup == 6 ) ); \
	group->Register( name, &BoolConvert2PType( varname, Auto ), SHOW( weapGroup == 7 ) ); \
	group->Register( name, &BoolConvert2PType( varname, Machine ), SHOW( weapGroup == 8 ) );\

#define RAGECOMBO( group, name, varname, weapGroup, size, items ) \
    group->Register( name, &IntConvert2PType( varname, Pistol ), size, items, SHOW( weapGroup == 0 ) ); \
    group->Register( name, &IntConvert2PType( varname, HeavyPistol ), size, items, SHOW( weapGroup == 1 ) ); \
    group->Register( name, &IntConvert2PType( varname, SMG ), size, items, SHOW( weapGroup == 2 ) ); \
    group->Register( name, &IntConvert2PType( varname, Rifle ), size, items, SHOW( weapGroup == 3 ) ); \
    group->Register( name, &IntConvert2PType( varname, Shotgun ), size, items, SHOW( weapGroup == 4 ) ); \
    group->Register( name, &IntConvert2PType( varname, AWP ), size, items, SHOW( weapGroup == 5 ) ); \
    group->Register( name, &IntConvert2PType( varname, Scout ), size, items, SHOW( weapGroup == 6 ) ); \
    group->Register( name, &IntConvert2PType( varname, Auto ), size, items, SHOW( weapGroup == 7 ) ); \
    group->Register( name, &IntConvert2PType( varname, Machine ), size, items, SHOW( weapGroup == 8 ) );\

#define RAGEMULTICOMBO( group, name, size, items, type ) \
    group->Register( name, size, items##Pistol, SHOW( weapGroup == 0 ) ); \
    group->Register( name, size, items##HeavyPistol, SHOW( weapGroup == 1 ) ); \
    group->Register( name, size, items##SMG, SHOW( weapGroup == 2 ) ); \
    group->Register( name, size, items##Rifle, SHOW( weapGroup == 3 ) ); \
    group->Register( name, size, items##Shotgun, SHOW( weapGroup == 4 ) ); \
    group->Register( name, size, items##AWP, SHOW( weapGroup == 5 ) ); \
    group->Register( name, size, items##Scout, SHOW( weapGroup == 6 ) ); \
    group->Register( name, size, items##Auto, SHOW( weapGroup == 7 ) ); \
    group->Register( name, size, items##Machine, SHOW( weapGroup == 8 ) );\

#define RAGEINTSLIDER( group, name, varname, type, min, max ) \
	group->Register( name, &IntConvert2PType( varname, Pistol ), min, max, SHOW( weapGroup == 0 ) ); \
	group->Register( name, &IntConvert2PType( varname, HeavyPistol ), min, max, SHOW( weapGroup == 1 ) ); \
	group->Register( name, &IntConvert2PType( varname, SMG ), min, max, SHOW( weapGroup == 2 ) ); \
	group->Register( name, &IntConvert2PType( varname, Rifle ), min, max, SHOW( weapGroup == 3 ) ); \
	group->Register( name, &IntConvert2PType( varname, Shotgun ), min, max, SHOW( weapGroup == 4 ) ); \
	group->Register( name, &IntConvert2PType( varname, AWP ), min, max, SHOW( weapGroup == 5 ) ); \
	group->Register( name, &IntConvert2PType( varname, Scout ), min, max, SHOW( weapGroup == 6 ) ); \
	group->Register( name, &IntConvert2PType( varname, Auto ), min, max, SHOW( weapGroup == 7 ) ); \
	group->Register( name, &IntConvert2PType( varname, Machine ), min, max, SHOW( weapGroup == 8 ) );\

// I WANNA DIE
#define RAGEINTSLIDER_SHOWCONDITION( group, name, varname, type, min, max, showcond ) \
	group->Register( name, &IntConvert2PType( varname, Pistol ), min, max, SHOW( weapGroup == 0 && BoolConvert2PType( showcond, Pistol ) ) ); \
	group->Register( name, &IntConvert2PType( varname, HeavyPistol ), min, max, SHOW( weapGroup == 1 && BoolConvert2PType( showcond, HeavyPistol ) ) ); \
	group->Register( name, &IntConvert2PType( varname, SMG ), min, max, SHOW( weapGroup == 2 && BoolConvert2PType( showcond, SMG ) ) ); \
	group->Register( name, &IntConvert2PType( varname, Rifle ), min, max, SHOW( weapGroup == 3 && BoolConvert2PType( showcond, Rifle ) ) ); \
	group->Register( name, &IntConvert2PType( varname, Shotgun ), min, max, SHOW( weapGroup == 4 && BoolConvert2PType( showcond, Shotgun ) ) ); \
	group->Register( name, &IntConvert2PType( varname, AWP ), min, max, SHOW( weapGroup == 5 && BoolConvert2PType( showcond, AWP ) ) ); \
	group->Register( name, &IntConvert2PType( varname, Scout ), min, max, SHOW( weapGroup == 6 && BoolConvert2PType( showcond, Scout ) ) ); \
	group->Register( name, &IntConvert2PType( varname, Auto ), min, max, SHOW( weapGroup == 7 && BoolConvert2PType( showcond, Auto ) ) ); \
	group->Register( name, &IntConvert2PType( varname, Machine ), min, max, SHOW( weapGroup == 8 && BoolConvert2PType( showcond, Machine ) ) );\

#define CHECKRAGEBOOL( type, name, jmp )switch ( type ) { \
case 0: if ( !Config::Get<bool>( Vars.##name##Pistol ) ) goto jmp;break; \
case 1: if ( !Config::Get<bool>( Vars.##name##HeavyPistol ) ) goto jmp;break; \
case 2: if ( !Config::Get<bool>( Vars.##name##SMG ) ) goto jmp;break; \
case 3: if ( !Config::Get<bool>( Vars.##name##Rifle ) ) goto jmp;break; \
case 4: if ( !Config::Get<bool>( Vars.##name##Shotgun ) ) goto jmp;break; \
case 5: if ( !Config::Get<bool>( Vars.##name##AWP ) ) goto jmp;break; \
case 6: if ( !Config::Get<bool>( Vars.##name##Scout ) ) goto jmp;break; \
case 7: if ( !Config::Get<bool>( Vars.##name##Auto ) ) goto jmp;break; \
case 8: if ( !Config::Get<bool>( Vars.##name##Machine ) ) goto jmp;break; }\

// below is the worst usage of fucken memory. i am utterly ashamed that i created the menu without recoding that shitty fucken config sys

MultiElement_t RegisterElements[ 4 ]{
	{ &Config::Get<bool>( Vars.RagebotResolver ), "rezik" },
	{ &Config::Get<bool>( Vars.RagebotLagcompensation ), "legcompenator" },
	{ &Config::Get<bool>( Vars.AntiaimEnable ), "anit-am" },
	{ &Config::Get<bool>( Vars.AntiaimConstantInvert ), "contant infurt" },
};

MultiElement_t weaponsLocal[ 2 ]{
	{ &Config::Get<bool>( Vars.VisWeapIconLocal ), _( "Weapon Text" ) },
	{ &Config::Get<bool>( Vars.VisWeapTextLocal ), _( "Weapon Icon" ) }
};

MultiElement_t weaponsTeam[ 2 ]{
	{ &Config::Get<bool>( Vars.VisWeapIconTeam ), "Weapon Text" },
	{ &Config::Get<bool>( Vars.VisWeapTextTeam ), "Weapon Icon" }
};

MultiElement_t weaponsEnemy[ 2 ]{
	{ &Config::Get<bool>( Vars.VisWeapIconEnemy ), "Weapon Text" },
	{ &Config::Get<bool>( Vars.VisWeapTextEnemy ), "Weapon Icon" }
};

MultiElement_t flagsLocal[ 6 ]{
	{ &Config::Get<bool>( Vars.VisFlagC4Local ), "C4" },
	{ &Config::Get<bool>( Vars.VisFlagArmorLocal ), "Armor" },
	{ &Config::Get<bool>( Vars.VisFlagFlashLocal ), "Flashed" },
	{ &Config::Get<bool>( Vars.VisFlagReloadLocal ), "Reloading" },
	{ &Config::Get<bool>( Vars.VisFlagScopedLocal ), "Scoped" },
	{ &Config::Get<bool>( Vars.VisFlagDefusingLocal ), "Defusing" }
};

MultiElement_t flagsTeam[ 8 ]{
	{ &Config::Get<bool>( Vars.VisFlagExploitTeam ), "Exploit" },
	{ &Config::Get<bool>( Vars.VisFlagBLCTeam ), "Broke lagcompensation" },
	{ &Config::Get<bool>( Vars.VisFlagC4Team ), "Bomb" },
	{ &Config::Get<bool>( Vars.VisFlagArmorTeam ), "Armor" },
	{ &Config::Get<bool>( Vars.VisFlagFlashTeam ), "Flashed" },
	{ &Config::Get<bool>( Vars.VisFlagReloadTeam ), "Reloading" },
	{ &Config::Get<bool>( Vars.VisFlagScopedTeam ), "Scoped" },
	{ &Config::Get<bool>( Vars.VisFlagDefusingTeam ), "Defusing" }
};

MultiElement_t flagsEnemy[ 8 ]{
	{ &Config::Get<bool>( Vars.VisFlagExploitEnemy ), "Exploit" },
	{ &Config::Get<bool>( Vars.VisFlagBLCEnemy ), "Broke lagcompensation" },
	{ &Config::Get<bool>( Vars.VisFlagC4Enemy ), "Bomb" },
	{ &Config::Get<bool>( Vars.VisFlagArmorEnemy ), "Armor" },
	{ &Config::Get<bool>( Vars.VisFlagFlashEnemy ), "Flashed" },
	{ &Config::Get<bool>( Vars.VisFlagReloadEnemy ), "Reloading" },
	{ &Config::Get<bool>( Vars.VisFlagScopedEnemy ), "Scoped" },
	{ &Config::Get<bool>( Vars.VisFlagDefusingEnemy ), "Defusing" }
};

MultiElement_t removals[ 6 ]{
	{ &Config::Get<bool>( Vars.RemovalScope ), "Scope" },
	{ &Config::Get<bool>( Vars.RemovalFlash ), "Flash" },
	{ &Config::Get<bool>( Vars.RemovalPunch ), "Aim punch" },
	{ &Config::Get<bool>( Vars.RemovalPostProcess ), "Post process" },
	{ &Config::Get<bool>( Vars.RemovalZoom ), "Zoom" },
	{ &Config::Get<bool>( Vars.RemovalSmoke ), "Smoke" }
};

MultiElement_t hitboxesPistol[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadPistol ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestPistol ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestPistol ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestPistol ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachPistol ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisPistol ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsPistol ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsPistol ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetPistol ), "Feet" }
}; 

MultiElement_t hitboxesHeavyPistol[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadHeavyPistol ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestHeavyPistol ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestHeavyPistol ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestHeavyPistol ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachHeavyPistol ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisHeavyPistol ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsHeavyPistol ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsHeavyPistol ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetHeavyPistol ), "Feet" }
};

MultiElement_t hitboxesSMG[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadSMG ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestSMG ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestSMG ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestSMG ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachSMG ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisSMG ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsSMG ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsSMG ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetSMG ), "Feet" }
};

MultiElement_t hitboxesRifle[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadRifle ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestRifle ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestRifle ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestRifle ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachRifle ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisRifle ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsRifle ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsRifle ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetRifle ), "Feet" }
};

MultiElement_t hitboxesShotgun[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadShotgun ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestShotgun ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestShotgun ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestShotgun ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachShotgun ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisShotgun ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsShotgun ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsShotgun ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetShotgun ), "Feet" }
};

MultiElement_t hitboxesAWP[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadAWP ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestAWP ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestAWP ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestAWP ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachAWP ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisAWP ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsAWP ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsAWP ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetAWP ), "Feet" }
};

MultiElement_t hitboxesScout[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadScout ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestScout ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestScout ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestScout ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachScout ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisScout ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsScout ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsScout ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetScout ), "Feet" }
};

MultiElement_t hitboxesAuto[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadAuto ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestAuto ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestAuto ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestAuto ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachAuto ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisAuto ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsAuto ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsAuto ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetAuto ), "Feet" }
};

MultiElement_t hitboxesMachine[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotHBHeadMachine ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotHBUpperChestMachine ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotHBChestMachine ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotHBLowerChestMachine ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotHBStomachMachine ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotHBPelvisMachine ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotHBArmsMachine ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotHBLegsMachine ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotHBFeetMachine ), "Feet" }
};

MultiElement_t multipointsPistol[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadPistol ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestPistol ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestPistol ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestPistol ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachPistol ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisPistol ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsPistol ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsPistol ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetPistol ), "Feet" }
};

MultiElement_t multipointsHeavyPistol[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadHeavyPistol ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestHeavyPistol ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestHeavyPistol ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestHeavyPistol ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachHeavyPistol ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisHeavyPistol ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsHeavyPistol ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsHeavyPistol ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetHeavyPistol ), "Feet" }
};

MultiElement_t multipointsSMG[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadSMG ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestSMG ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestSMG ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestSMG ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachSMG ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisSMG ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsSMG ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsSMG ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetSMG ), "Feet" }
};

MultiElement_t multipointsRifle[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadRifle ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestRifle ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestRifle ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestRifle ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachRifle ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisRifle ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsRifle ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsRifle ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetRifle ), "Feet" }
};

MultiElement_t multipointsShotgun[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadShotgun ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestShotgun ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestShotgun ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestShotgun ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachShotgun ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisShotgun ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsShotgun ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsShotgun ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetShotgun ), "Feet" }
};

MultiElement_t multipointsAWP[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadAWP ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestAWP ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestAWP ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestAWP ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachAWP ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisAWP ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsAWP ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsAWP ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetAWP ), "Feet" }
};

MultiElement_t multipointsScout[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadScout ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestScout ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestScout ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestScout ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachScout ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisScout ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsScout ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsScout ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetScout ), "Feet" }
};

MultiElement_t multipointsAuto[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadAuto ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestAuto ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestAuto ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestAuto ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachAuto ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisAuto ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsAuto ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsAuto ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetAuto ), "Feet" }
};

MultiElement_t multipointsMachine[ 9 ]{
	{ &Config::Get<bool>( Vars.RagebotMPHeadMachine ), "Head" },
	{ &Config::Get<bool>( Vars.RagebotMPUpperChestMachine ), "Upper chest" },
	{ &Config::Get<bool>( Vars.RagebotMPChestMachine ), "Chest" },
	{ &Config::Get<bool>( Vars.RagebotMPLowerChestMachine ), "Lower chest" },
	{ &Config::Get<bool>( Vars.RagebotMPStomachMachine ), "Stomach" },
	{ &Config::Get<bool>( Vars.RagebotMPPelvisMachine ), "Pelvis" },
	{ &Config::Get<bool>( Vars.RagebotMPArmsMachine ), "Arms" },
	{ &Config::Get<bool>( Vars.RagebotMPLegsMachine ), "Legs" },
	{ &Config::Get<bool>( Vars.RagebotMPFeetMachine ), "Feet" }
};

MultiElement_t preferBaimPistol[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimPistol ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapPistol ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalPistol ), "Lethal" }
};

MultiElement_t preferBaimHeavyPistol[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimHeavyPistol ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapHeavyPistol ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalHeavyPistol ), "Lethal" }
};

MultiElement_t preferBaimSMG[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimSMG ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapSMG ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalSMG ), "Lethal" }
};

MultiElement_t preferBaimRifle[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimRifle ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapRifle ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalRifle ), "Lethal" }
};

MultiElement_t preferBaimShotgun[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimShotgun ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapShotgun ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalShotgun ), "Lethal" }
};

MultiElement_t preferBaimAWP[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimAWP ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapAWP ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalAWP ), "Lethal" }
};

MultiElement_t preferBaimScout[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimScout ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapScout ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalScout ), "Lethal" }
};

MultiElement_t preferBaimAuto[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimAuto ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapAuto ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalAuto ), "Lethal" }
};

MultiElement_t recordOptions[ ]{
	{ &Config::Get<bool>( Vars.DBGOldPing ), "Old ping" },
	{ &Config::Get<bool>( Vars.DBGNoPingReducer ), "No ping reducer" }
};

MultiElement_t preferBaimMachine[ 3 ]{
	{ &Config::Get<bool>( Vars.RagebotPreferBaimMachine ), "Always" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimDoubletapMachine ), "Doubletap" },
	{ &Config::Get<bool>( Vars.RagebotPreferBaimLethalMachine ), "Lethal" }
};

ComboElement_t materials[ 5 ]{ _( "Regular" ), _( "Flat" ), _( "Glow" ), _( "Metallic" ), _( "Galaxy" ) };

ComboElement_t comboElements[ 4 ]{ "rome", "from", "monebot", "$$$$" };

ComboElement_t hitboxType[ 2 ]{ _( "Cham" ), _( "Hitboxes" ) };

ComboElement_t tracerType[ 2 ]{ _( "Line" ), _( "Laser" ) };

ComboElement_t freestandingType[ ]{ _( "None" ), _( "Classic" ), _( "Desync" ), _( "Desync inverted" ) };

ComboElement_t weatherType[ ]{ _( "None" ), _( "Rain" ), _( "Snow" ), _( "Particle Rain" ), _( "Particle Snow" ), _("Rainstorm" ) };

ComboElement_t pitch[ 5 ]{ _( "Default" ), _( "Up" ), _( "Down" ), _( "Zero" ), _( "Random" ) };
ComboElement_t safePitch[ 5 ]{ _( "Default" ), _( "Up" ), _( "Down" ), _( "Zero" ), _( "Random" ) };

ComboElement_t yaw[ 4 ]{ _( "Default" ), _( "Backward" ), _( "Left" ), _( "Right" ) };

ComboElement_t yawAdd[ 5 ]{ _( "None" ), _( "Jitter" ), _( "Rotate" ), _( "Spin" ), _( "Random" ) };

ComboElement_t atTargets[ 3 ]{ _( "Off" ), _( "FOV" ), _( "Distance" ) };

ComboElement_t weaponGroups[ 9 ]{ _( "Pistol" ), _( "Heavy pistol" ), _( "SMG" ), _( "Rifle" ), _( "Shotgun" ), _( "Awp" ), ( "Scout" ), ( "Auto" ), _( "Machine gun" ) };

ComboElement_t targetSelection[ 4 ]{ _( "Highest damage" ), _( "FOV" ), _( "Distance" ), _( "Lowest health" ) };

ComboElement_t modelsForChange[ ]{ _( "Orange jumpsuit A" ), _( "Orange jumpsuit B" ), _( "Orange jumpsuit C" ), _( "Seal A" ), _( "Seal B" ), _( "Seal C" ), _( "Seal B" ), _("FBI A" ), _("Armed" ), _("Ground rebel"), _("FBI chick"), _("Anarchist"), _("Office terrorist" ) };

ComboElement_t skies[ 22 ]{
				_( "None" ),
				_( "Tibet" ),
				_( "Baggage" ),
				_( "Italy" ),
				_( "Aztec" ),
				_( "Vertigo" ),
				_( "Daylight" ),
				_( "Daylight 2" ),
				_( "Clouds" ),
				_( "Clouds 2" ),
				_( "Gray" ),
				_( "Clear" ),
				_( "Canals" ),
				_( "Cobblestone" ),
				_( "Assault" ),
				_( "Clouds dark" ),
				_( "Night" ),
				_( "Night 2" ),
				_( "Night flat" ),
				_( "Dusty" ),
				_( "Rainy" ),
				_( "Custom" )
};
