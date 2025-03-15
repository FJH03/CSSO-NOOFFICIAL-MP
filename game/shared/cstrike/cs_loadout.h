//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: player loadout
//
//=============================================================================//

#ifndef CSLOADOUT_H
#define CSLOADOUT_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseplayer_shared.h"
#include "weapon_csbase.h"

#ifdef CLIENT_DLL
#define CCSLoadout C_CSLoadout
#define CBasePlayer C_BasePlayer
#define CCSPlayer C_CSPlayer
#endif

class CCSPlayer;

// Loadout slots
enum LoadoutSlot_t
{
	SLOT_NONE = -1,
	SLOT_M4,
	SLOT_HKP2000,
	SLOT_FIVESEVEN,
	SLOT_TEC9,
	SLOT_MP7_CT,
	SLOT_MP7_T,
	SLOT_DEAGLE_CT,
	SLOT_DEAGLE_T,

	SLOT_MAX = SLOT_DEAGLE_T
};

struct CLoadout
{
	const char*		m_szCommand;
	const char*		m_szFirstWeapon;
	const char*		m_szSecondWeapon;
	CSWeaponID		m_iFirstWeaponID;
	CSWeaponID		m_iSecondWeaponID;
};

class CCSLoadout
{
public:
	CCSLoadout( void );
	~CCSLoadout( void );

public:
	// check if the given weapon is actually a knife
	bool			IsKnife( CSWeaponID weaponid )			{ return (weaponid >= WEAPON_KNIFE) && (weaponid <= WEAPON_KNIFE_LAST); }
	bool			IsKnife( const char* pszWeaponName )	{ return !V_strncmp( pszWeaponName, "weapon_knife", 12 ); }
	// get the loadout slot of the weapon
	LoadoutSlot_t	GetSlotFromWeapon( int team, const char* weaponName );
	// get the weapon from a client's slot
	const char*		GetWeaponFromSlot( CCSPlayer* pPlayer, LoadoutSlot_t slot );

	CSWeaponID		GetLoadoutWeaponID( CCSPlayer* pPlayer, int iTeamNumber, CSWeaponID iWeaponID );
	const char*		GetLoadoutWeapon( CCSPlayer* pPlayer, const char* pszWeaponName );
	
	bool			HasGlovesSet( CCSPlayer* pPlayer, int team );
	int				GetGlovesForPlayer( CCSPlayer* pPlayer, int team );

	bool			HasKnifeSet( CCSPlayer* pPlayer, int team );
	int				GetKnifeForPlayer( CCSPlayer* pPlayer, int team );

	bool			HasAgentSet( CCSPlayer* pPlayer, int team );
	int				GetAgentForPlayer( CCSPlayer* pPlayer, int team );
};


extern CCSLoadout *g_pCSLoadout;

inline CCSLoadout *CSLoadout()
{
	return g_pCSLoadout;
}

#endif