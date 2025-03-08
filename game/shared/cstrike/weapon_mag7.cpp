//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponMAG7 C_WeaponMAG7
#endif


class CWeaponMAG7 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMAG7, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMAG7() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_MAG7; }

private:
	CWeaponMAG7( const CWeaponMAG7 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMAG7, DT_WeaponMAG7 )

BEGIN_NETWORK_TABLE( CWeaponMAG7, DT_WeaponMAG7 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMAG7 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mag7, CWeaponMAG7 );
PRECACHE_WEAPON_REGISTER( weapon_mag7 );
