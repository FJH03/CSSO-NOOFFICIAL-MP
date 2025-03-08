//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponUSP C_WeaponUSP
#endif


class CWeaponUSP : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponUSP, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponUSP() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_USP; }

private:
	CWeaponUSP( const CWeaponUSP & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponUSP, DT_WeaponUSP )

BEGIN_NETWORK_TABLE( CWeaponUSP, DT_WeaponUSP )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponUSP )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_usp_silencer, CWeaponUSP );
PRECACHE_WEAPON_REGISTER( weapon_usp_silencer );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_usp, CWeaponUSP ); // for backwards compatibility
#endif

void CWeaponUSP::Spawn()
{
	SetClassname( "weapon_usp_silencer" ); // for backwards compatibility
	BaseClass::Spawn();
}
