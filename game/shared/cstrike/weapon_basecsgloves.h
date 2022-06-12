//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: Base class for all in-game gloves
//
//=============================================================================//

#ifndef WEAPON_BASECSGLOVES_H
#define WEAPON_BASECSGLOVES_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CBaseCSGloves C_BaseCSGloves

#include "c_baseanimating.h"
#else
#include "baseanimating.h"
#endif

class CBaseCSGloves: public CBaseAnimating
{
	DECLARE_CLASS( CBaseCSGloves, CBaseAnimating );
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

public:
	CBaseCSGloves();

#ifdef GAME_DLL
	virtual int UpdateTransmitState();
#endif

	void Equip( CBaseAnimating *pOwner );
	void UnEquip();

	void UpdateGlovesModel();

	int GetGloveID() { return m_nGloveID; }
	void SetGloveID( int iGloveID ) { m_nGloveID = iGloveID; }

private:
	CNetworkVar( int, m_nGloveID );
};

#endif // WEAPON_BASECSGLOVES_H