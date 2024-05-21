#include "interfaces.h"

bool Interfaces::Setup()
{
	Client =			Capture<IBaseClientDll>(CLIENT_DLL, _("VClient"));
	ClientEntityList =	Capture<IClientEntityList>(CLIENT_DLL, _("VClientEntityList"));
	Effects =			Capture<IEffects>(CLIENT_DLL, _("IEffects"));
	GameMovement =		Capture<IGameMovement>(CLIENT_DLL, _("GameMovement"));
	Prediction =		Capture<IPrediction>(CLIENT_DLL, _("VClientPrediction"));
	GameConsole =		Capture<IGameConsole>(CLIENT_DLL, _("GameConsole"));
	GameUI =			Capture<IGameUI>(CLIENT_DLL, _("GameUI"));
	Engine =			Capture<IEngineClient>(ENGINE_DLL, _("VEngineClient"));
	EngineVGui =		Capture<IEngineVGui>(ENGINE_DLL, _("VEngineVGui"));
	EngineTrace =		Capture<IEngineTrace>(ENGINE_DLL, _("EngineTraceClient"));
	EngineSound =		Capture<IEngineSound>(ENGINE_DLL, _("IEngineSoundClient"));
	NetworkContainer =	Capture<INetworkContainer>(ENGINE_DLL, _("VEngineClientStringTable"));
	GameEvent =			Capture<IGameEventManager2>(ENGINE_DLL, _("GAMEEVENTSMANAGER002"));
	RenderView =		Capture<IVRenderView>(ENGINE_DLL, _("VEngineRenderView"));
	DebugOverlay =		Capture<IVDebugOverlay>(ENGINE_DLL, _("VDebugOverlay"));
	EngineEffects =		Capture<IVEngineEffects>(ENGINE_DLL, _("VEngineEffects"));
	ModelInfo =			Capture<IVModelInfo>(ENGINE_DLL, _("VModelInfoClient"));
	ModelRender =		Capture<IVModelRender>(ENGINE_DLL, _("VEngineModel"));
	MDLCache =			Capture<IMDLCache>(DATACACHE_DLL, _("MDLCache"));
	StudioRender =		Capture<IStudioRender>(STUDIORENDER_DLL, _("VStudioRender"));
	ConVar =			Capture<IConVar>(VSTDLIB_DLL, _("VEngineCvar"));
	PhysicsProps =		Capture<IPhysicsSurfaceProps>(PHYSICS_DLL, _("VPhysicsSurfaceProps"));
	PhysicsCollision = Capture<IPhysicsCollision>( PHYSICS_DLL, _( "VPhysicsCollision" ) );
	MaterialSystem =	Capture<IMaterialSystem>(MATERIALSYSTEM_DLL, _("VMaterialSystem"));
	Surface =			Capture<ISurface>(VGUI_DLL, _("VGUI_Surface"));
	Panel =				Capture<IVPanel>(VGUI2_DLL, _("VGUI_Panel"));
	InputSystem =		Capture<IInputSystem>(INPUTSYSTEM_DLL, _("InputSystemVersion"));
	Localize =			Capture<ILocalize>(LOCALIZE_DLL, _("Localize_"));
	MatchFramework =	Capture<IMatchFramework>(MATCHMAKING_DLL, _("MATCHFRAMEWORK_"));
	GameTypes =			Capture<IGameTypes>(MATCHMAKING_DLL, _("VENGINE_GAMETYPES_VERSION"));
	Server =			Capture<IServerGameDLL>(SERVER_DLL, _("ServerGameDLL"));

	if ( !PhysicsCollision )
		return false;

	SteamClient = Engine->GetSteamAPIContext()->pSteamClient;
	if (SteamClient == nullptr)
		return false;

	SteamUser =	Engine->GetSteamAPIContext()->pSteamUser;
	if (SteamUser == nullptr)
		return false;

	const void* hSteamAPI = MEM::GetModuleBaseHandle(STEAM_API_DLL);
	const HSteamUser hSteamUser = reinterpret_cast<std::add_pointer_t<HSteamUser()>>(MEM::GetExportAddress(hSteamAPI, _("SteamAPI_GetHSteamUser")))();
	const HSteamPipe hSteamPipe = reinterpret_cast<std::add_pointer_t<HSteamPipe()>>(MEM::GetExportAddress(hSteamAPI, _("SteamAPI_GetHSteamPipe")))();

	SteamGameCoordinator = static_cast<ISteamGameCoordinator*>(SteamClient->GetISteamGenericInterface(hSteamUser, hSteamPipe, _("SteamGameCoordinator001")));
	if (SteamGameCoordinator == nullptr)
		return false;

	ClientMode = **reinterpret_cast<IClientModeShared***>(MEM::GetVFunc<std::uintptr_t>(Client, 10) + 0x5); // get it from CHLClient::HudProcessInput
	if (ClientMode == nullptr)
		return false;

	Globals = **reinterpret_cast<IGlobalVarsBase***>(MEM::GetVFunc<std::uintptr_t>(Client, 11) + 0xA); // get it from CHLClient::HudUpdate @xref: "(time_int)", "(time_float)"
	if (Globals == nullptr)
		return false;

	MemAlloc = *static_cast<IMemAlloc**>(MEM::GetExportAddress(MEM::GetModuleBaseHandle(TIER0_DLL), _("g_pMemAlloc")));
	if (MemAlloc == nullptr)
		return false;

	KeyValuesSystem = reinterpret_cast<KeyValuesSystemFn>(MEM::GetExportAddress(MEM::GetModuleBaseHandle(VSTDLIB_DLL), _("KeyValuesSystem")))();
	if (KeyValuesSystem == nullptr)
		return false;

	DirectDevice = **reinterpret_cast<IDirect3DDevice9***>(MEM::FindPattern(SHADERPIDX9_DLL, _("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 0x1); // @xref: "HandleLateCreation"
	if (DirectDevice == nullptr)
		return false;	

	MoveHelper = ( IMoveHelper* )**( std::uintptr_t** )( MEM::FindPattern( CLIENT_DLL, _( "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01" ) ) + 2 );
	if ( MoveHelper == nullptr)
		return false;

	ViewRender = **reinterpret_cast<IViewRender***>(MEM::FindPattern(CLIENT_DLL, _("8B 0D ? ? ? ? FF 75 0C 8B 45 08")) + 0x2);
	if (ViewRender == nullptr)
		return false;

	ViewRenderBeams = *reinterpret_cast<IViewRenderBeams**>(MEM::FindPattern(CLIENT_DLL, _("B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9")) + 0x1); // @xref: "r_drawbrushmodels"
	if (ViewRenderBeams == nullptr)
		return false;	

	Input =	*reinterpret_cast<IInput**>(MEM::FindPattern(CLIENT_DLL, _("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10")) + 0x1); // @note: or address of some indexed input function in chlclient class (like IN_ActivateMouse, IN_DeactivateMouse, IN_Accumulate, IN_ClearStates) + 0x1 (jmp to m_pInput)
	if (Input == nullptr)
		return false;

	ClientState = **reinterpret_cast<IClientState***>(MEM::FindPattern(ENGINE_DLL, _("A1 ? ? ? ? 8B 88 ? ? ? ? 85 C9 75 07")) + 0x1);
	if (ClientState == nullptr)
		return false;

	WeaponSystem = *reinterpret_cast<IWeaponSystem**>(MEM::FindPattern(CLIENT_DLL, _("8B 35 ? ? ? ? FF 10 0F B7 C0")) + 0x2);
	if (WeaponSystem == nullptr)
		return false;

	GlowManager = *reinterpret_cast<IGlowObjectManager**>(MEM::FindPattern(CLIENT_DLL, _("0F 11 05 ? ? ? ? 83 C8 01")) + 0x3);
	if (GlowManager == nullptr)
		return false;

	return true;
}

// just a stub for erasing
#define CHECK_INTERFACE( interface ) if ( !interface ) return false; else return true;
bool Interfaces::CheckInterfaces( ) {
	CHECK_INTERFACE( Client );
	CHECK_INTERFACE( ClientEntityList );
	CHECK_INTERFACE( Effects );
	CHECK_INTERFACE( GameMovement );
	CHECK_INTERFACE( Prediction );
	CHECK_INTERFACE( GameConsole );
	CHECK_INTERFACE( GameUI );
	CHECK_INTERFACE( Engine );
	CHECK_INTERFACE( EngineVGui );
	CHECK_INTERFACE( EngineTrace );
	CHECK_INTERFACE( EngineSound );
	CHECK_INTERFACE( NetworkContainer );
	CHECK_INTERFACE( GameEvent );
	CHECK_INTERFACE( RenderView );
	CHECK_INTERFACE( DebugOverlay );
	CHECK_INTERFACE( EngineEffects );
	CHECK_INTERFACE( ModelInfo );
	CHECK_INTERFACE( ModelRender );
	CHECK_INTERFACE( MDLCache );
	CHECK_INTERFACE( StudioRender );
	CHECK_INTERFACE( ConVar );
	CHECK_INTERFACE( PhysicsProps );
	CHECK_INTERFACE( MaterialSystem );
	CHECK_INTERFACE( Surface );
	CHECK_INTERFACE( Panel );
	CHECK_INTERFACE( InputSystem );
	CHECK_INTERFACE( Localize );
	CHECK_INTERFACE( MatchFramework );
	CHECK_INTERFACE( GameTypes );
	CHECK_INTERFACE( Server );
}

template <typename T>
T* Interfaces::Capture(const std::string_view szModuleName, const std::string_view szInterface)
{
	const auto GetRegisterList = [&szModuleName]() -> CInterfaceRegister*
	{
		void* oCreateInterface = nullptr;

		if (const auto hModule = MEM::GetModuleBaseHandle(szModuleName); hModule != nullptr)
			oCreateInterface = MEM::GetExportAddress(hModule, _("CreateInterface"));

		if (oCreateInterface == nullptr)
			throw std::runtime_error(_("failed get createinterface address"));

		const std::uintptr_t uCreateInterfaceRelative = reinterpret_cast<std::uintptr_t>(oCreateInterface) + 0x5;
		const std::uintptr_t uCreateInterface = uCreateInterfaceRelative + 4U + *reinterpret_cast<std::int32_t*>(uCreateInterfaceRelative);
		return **reinterpret_cast<CInterfaceRegister***>(uCreateInterface + 0x6);
	};

	for (auto pRegister = GetRegisterList(); pRegister != nullptr; pRegister = pRegister->pNext)
	{
		// found needed interface
		if ((std::string_view(pRegister->szName).compare(0U, szInterface.length(), szInterface) == 0 &&
			// and it have digits after name
			std::atoi(pRegister->szName + szInterface.length()) > 0) ||
			// or given full name with hardcoded digits
			szInterface.compare(pRegister->szName) == 0)
		{
			// capture our interface
			auto pInterface = pRegister->pCreateFn();

			return static_cast<T*>(pInterface);
		}
	}

	return nullptr;
}
