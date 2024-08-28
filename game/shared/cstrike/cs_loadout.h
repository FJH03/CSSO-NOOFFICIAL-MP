#ifndef CSLOADOUT_H
#define CSLOADOUT_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CCSLoadout C_CSLoadout
#define CBasePlayer C_BasePlayer
#endif

#include "cbase.h"
#include "baseplayer_shared.h"
#include "weapon_csbase.h"
#define MAX_KNIVES					18 // any new knives? add them here
class CCSPlayer;

class CCSLoadout : public CAutoGameSystemPerFrame
{
public:
	DECLARE_CLASS_GAMEROOT( CCSLoadout, CAutoGameSystemPerFrame );

	virtual char const *Name() { return "CCSLoadout"; }

	CCSLoadout( void );
	~CCSLoadout( void );

public:
	// check if the given weapon is actually a knife
	bool			IsKnife( CSWeaponID weaponid ) { return (weaponid >= WEAPON_KNIFE) && (weaponid <= WEAPON_KNIFE + MAX_KNIVES + 1); }

	bool			HasGlovesSet( CCSPlayer* pPlayer, int team );
	int			GetGlovesForPlayer( CCSPlayer* pPlayer, int team );
};
extern CCSLoadout *g_pCSLoadout;

inline CCSLoadout *CSLoadout()
{
	return g_pCSLoadout;
}

#endif