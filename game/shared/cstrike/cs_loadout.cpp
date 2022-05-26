//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: player loadout
//
//=============================================================================//

#include "cbase.h"
#include "cs_loadout.h"
#include "cs_shareddefs.h"
#ifdef CLIENT_DLL
#include "c_cs_player.h"
#else
#include "cs_player.h"
#endif

#ifdef CLIENT_DLL
ConVar loadout_slot_m4_weapon( "loadout_slot_m4_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in M4 slot.\n 0 - M4A4\n 1 - M4A1-S", true, 0, true, 1 );
ConVar loadout_slot_hkp2000_weapon( "loadout_slot_hkp2000_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in HKP2000 slot.\n 0 - HKP2000\n 1 - USP-S", true, 0, true, 1 );
ConVar loadout_slot_knife_weapon_ct( "loadout_slot_knife_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in knife slot for CTs.\n 0 - Default CT knife\n 1 - CS:S knife\n 2 - Karambit\n 3 - Flip\n 4 - Bayonet\n 5 - M9 Bayonet\n 6 - Butterfly\n 7 - Gut\n 8 - Huntsman\n 9 - Falchion\n 10 - Bowie\n 11 - Survival\n 12 - Paracord\n 13 - Navaja\n 14 - Nomad\n 15 - Skeleton\n 16 - Stiletto\n 17 - Ursus\n 18 - Talon", true, 0, true, MAX_KNIVES );
ConVar loadout_slot_knife_weapon_t( "loadout_slot_knife_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in knife slot for Ts.\n 0 - Default T knife\n 1 - CS:S knife\n 2 - Karambit\n 3 - Flip\n 4 - Bayonet\n 5 - M9 Bayonet\n 6 - Butterfly\n 7 - Gut\n 8 - Huntsman\n 9 - Falchion\n 10 - Bowie\n 11 - Survival\n 12 - Paracord\n 13 - Navaja\n 14 - Nomad\n 15 - Skeleton\n 16 - Stiletto\n 17 - Ursus\n 18 - Talon", true, 0, true, MAX_KNIVES );
ConVar loadout_slot_fiveseven_weapon( "loadout_slot_fiveseven_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Five-SeveN slot.\n 0 - Five-SeveN\n 1 - CZ-75", true, 0, true, 1 );
ConVar loadout_slot_tec9_weapon( "loadout_slot_tec9_weapon", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Tec-9 slot.\n 0 - Tec-9\n 1 - CZ-75", true, 0, true, 1 );
ConVar loadout_slot_mp7_weapon_ct( "loadout_slot_mp7_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in MP7 slot for CTs.\n 0 - MP7\n 1 - MP5SD", true, 0, true, 1 );
ConVar loadout_slot_mp7_weapon_t( "loadout_slot_mp7_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in MP7 slot for Ts.\n 0 - MP7\n 1 - MP5SD", true, 0, true, 1 );
ConVar loadout_slot_deagle_weapon_ct( "loadout_slot_deagle_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Deagle slot for CTs.\n 0 - Deagle\n 1 - R8 Revolver", true, 0, true, 1 );
ConVar loadout_slot_deagle_weapon_t( "loadout_slot_deagle_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which weapon to use in Deagle slot for Ts.\n 0 - Deagle\n 1 - R8 Revolver", true, 0, true, 1 );
ConVar loadout_slot_agent_ct( "loadout_slot_agent_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which agent to use for CTs.", true, 0, true, MAX_AGENTS_CT );
ConVar loadout_slot_agent_t( "loadout_slot_agent_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which agent to use for Ts.", true, 0, true, MAX_AGENTS_T );
ConVar loadout_slot_gloves_ct( "loadout_slot_gloves_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which gloves to use for CTs.", true, 0, true, MAX_GLOVES );
ConVar loadout_slot_gloves_t( "loadout_slot_gloves_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which gloves to use for Ts.", true, 0, true, MAX_GLOVES );
ConVar loadout_stattrak( "loadout_stattrak", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Enable or disable StatTrak on weapons.", true, 0, true, 1 );
ConVar loadout_music( "loadout_music", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Which music kit to use.", true, 0, true, MAX_MUSIC - 1 );
ConVar loadout_mainmenu_agent( "loadout_mainmenu_agent", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "What agent to use for main menu.\n 0 - None\n 1 - T\n 2 - CT", true, 0, true, 2 );
ConVar loadout_mainmenu_weapon_t( "loadout_mainmenu_weapon_t", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "What weapon to use for T main menu agent.", true, 0, true, MAX_MAINMENU_WEAPONS_T - 1 );
ConVar loadout_mainmenu_weapon_ct( "loadout_mainmenu_weapon_ct", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "What weapon to use for CT main menu agent.", true, 0, true, MAX_MAINMENU_WEAPONS_CT - 1 );
#endif
ConVar bot_loadout_random_knives( "bot_loadout_random_knives", "0", FCVAR_REPLICATED, "Whether or not the bots will have random knives." );
ConVar bot_loadout_random_gloves( "bot_loadout_random_gloves", "0", FCVAR_REPLICATED, "Whether or not the bots will have random gloves." );
ConVar bot_loadout_random_agents( "bot_loadout_random_agents", "0", FCVAR_REPLICATED, "Whether or not the bots will have random agents." );

CCSLoadout*	g_pCSLoadout = NULL;
CCSLoadout::CCSLoadout()
{
	Assert( !g_pCSLoadout );
	g_pCSLoadout = this;
}
CCSLoadout::~CCSLoadout()
{
	Assert( g_pCSLoadout == this );
	g_pCSLoadout = NULL;
}

CLoadout WeaponLoadout[]
{
	{	"loadout_slot_m4_weapon",			"m4a4",			"m4a1_silencer",	WEAPON_M4A4,		WEAPON_M4A1		},
	{	"loadout_slot_hkp2000_weapon",		"hkp2000",		"usp_silencer",		WEAPON_HKP2000,		WEAPON_USP		},
	{	"loadout_slot_fiveseven_weapon",	"fiveseven",	"cz75a",			WEAPON_FIVESEVEN,	WEAPON_CZ75A	},
	{	"loadout_slot_tec9_weapon",			"tec9",			"cz75a",			WEAPON_TEC9,		WEAPON_CZ75A	},
	{	"loadout_slot_mp7_weapon_ct",		"mp7",			"mp5sd",			WEAPON_MP7,			WEAPON_MP5SD	},
	{	"loadout_slot_mp7_weapon_t",		"mp7",			"mp5sd",			WEAPON_MP7,			WEAPON_MP5SD	},
	{	"loadout_slot_deagle_weapon_ct",	"deagle",		"revolver",			WEAPON_DEAGLE,		WEAPON_REVOLVER	},
	{	"loadout_slot_deagle_weapon_t",		"deagle",		"revolver",			WEAPON_DEAGLE,		WEAPON_REVOLVER	},
};

LoadoutSlot_t CCSLoadout::GetSlotFromWeapon( int team, const char* weaponName )
{
	LoadoutSlot_t slot = SLOT_NONE;

	for ( int i = 0; i < ARRAYSIZE( WeaponLoadout ); i++ )
	{
		if ( Q_strcmp( WeaponLoadout[i].m_szFirstWeapon, weaponName ) == 0 ||
			 Q_strcmp( WeaponLoadout[i].m_szSecondWeapon, weaponName ) == 0 )
			slot = (LoadoutSlot_t)i;

		if ( slot == SLOT_MP7_CT || slot == SLOT_MP7_T )
		{
			slot = (team == TEAM_CT) ? SLOT_MP7_CT : SLOT_MP7_T;
		}
		if ( slot == SLOT_DEAGLE_CT || slot == SLOT_DEAGLE_T )
		{
			slot = (team == TEAM_CT) ? SLOT_DEAGLE_CT : SLOT_DEAGLE_T;
		}

		if ( slot != SLOT_NONE )
			break;
	}

	return slot;
}

const char* CCSLoadout::GetWeaponFromSlot( CCSPlayer* pPlayer, LoadoutSlot_t slot )
{
	if ( slot >= 0 && slot < SLOT_MAX )
	{
		if ( pPlayer->IsBotOrControllingBot() )
		{
			return WeaponLoadout[slot].m_szFirstWeapon;
		}

		int value = 0;
#ifdef CLIENT_DLL
		ConVarRef convar( WeaponLoadout[slot].m_szCommand );
		if (convar.IsValid())
			value = convar.GetInt();
#else
		value = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), WeaponLoadout[slot].m_szCommand ) );
#endif
		return (value > 0) ? WeaponLoadout[slot].m_szSecondWeapon : WeaponLoadout[slot].m_szFirstWeapon;
	}

	return NULL;
}

CSWeaponID CCSLoadout::GetLoadoutWeaponID( CCSPlayer* pPlayer, int iTeamNumber, CSWeaponID iWeaponID )
{
	if ( pPlayer->IsBotOrControllingBot() )
		return iWeaponID;

	LoadoutSlot_t iSlot = SLOT_NONE;

	for ( int i = 0; i < ARRAYSIZE( WeaponLoadout ); i++ )
	{
		if ( WeaponLoadout[i].m_iFirstWeaponID == iWeaponID ||
			 WeaponLoadout[i].m_iSecondWeaponID == iWeaponID )
			iSlot = (LoadoutSlot_t)i;

		if ( iSlot == SLOT_MP7_CT || iSlot == SLOT_MP7_T )
		{
			iSlot = (iTeamNumber == TEAM_CT) ? SLOT_MP7_CT : SLOT_MP7_T;
		}
		if ( iSlot == SLOT_DEAGLE_CT || iSlot == SLOT_DEAGLE_T )
		{
			iSlot = (iTeamNumber == TEAM_CT) ? SLOT_DEAGLE_CT : SLOT_DEAGLE_T;
		}

		if ( iSlot != SLOT_NONE )
			break;
	}

	if ( iSlot != SLOT_NONE )
	{
		int value = 0;
#ifdef CLIENT_DLL
		ConVarRef convar( WeaponLoadout[iSlot].m_szCommand );
		if ( convar.IsValid() )
			value = convar.GetInt();
#else
		value = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), WeaponLoadout[iSlot].m_szCommand ) );
#endif
		return (value > 0) ? WeaponLoadout[iSlot].m_iSecondWeaponID : WeaponLoadout[iSlot].m_iFirstWeaponID;
	}

	return iWeaponID;
}

const char* CCSLoadout::GetLoadoutWeapon( CCSPlayer* pPlayer, const char* pszWeaponName )
{
	if ( pPlayer->IsBotOrControllingBot() )
		return pszWeaponName;

	LoadoutSlot_t iSlot = GetSlotFromWeapon( pPlayer->GetTeamNumber(), pszWeaponName );

	if ( iSlot != SLOT_NONE )
	{
		int value = 0;
#ifdef CLIENT_DLL
		ConVarRef convar( WeaponLoadout[iSlot].m_szCommand );
		if ( convar.IsValid() )
			value = convar.GetInt();
#else
		value = atoi( engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), WeaponLoadout[iSlot].m_szCommand ) );
#endif
		return (value > 0) ? WeaponLoadout[iSlot].m_szSecondWeapon : WeaponLoadout[iSlot].m_szFirstWeapon;
	}

	return pszWeaponName;
}

bool CCSLoadout::HasGlovesSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_gloves.GetBool() )
	{
		return false;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return false;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotGlovesCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotGlovesT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetGlovesForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_gloves.GetBool() )
	{
		return 0;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return 0;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotGlovesCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotGlovesT;
			break;
		default:
			break;
	}

	return value;
}

bool CCSLoadout::HasKnifeSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_knives.GetBool() )
	{
		return false;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return false;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetKnifeForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_knives.GetBool() )
	{
		return 0;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return 0;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotKnifeWeaponT;
			break;
		default:
			break;
	}

	return value - 1; // arrays are started with index 0 not 1
}

bool CCSLoadout::HasAgentSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_agents.GetBool() )
	{
		return false;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return false;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotAgentCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotAgentT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetAgentForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	if ( pPlayer->IsBotOrControllingBot() && !bot_loadout_random_agents.GetBool() )
	{
		return 0;
	}

	if ( pPlayer->IsControllingBot() )
	{
#ifdef CLIENT_DLL
		pPlayer = ToCSPlayer( UTIL_PlayerByIndex( pPlayer->GetControlledBotIndex() ) );
#else
		pPlayer = pPlayer->GetControlledBot();
#endif
		if ( !pPlayer )
			return 0;
	}

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotAgentCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotAgentT;
			break;
		default:
			break;
	}

	return value;
}