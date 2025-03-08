//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponAWP C_WeaponAWP
#endif


class CWeaponAWP : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponAWP, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponAWP() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_AWP; }

private:
	CWeaponAWP( const CWeaponAWP & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAWP, DT_WeaponAWP )

BEGIN_NETWORK_TABLE( CWeaponAWP, DT_WeaponAWP )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponAWP )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_awp, CWeaponAWP );
PRECACHE_WEAPON_REGISTER( weapon_awp );
