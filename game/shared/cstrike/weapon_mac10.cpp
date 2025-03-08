//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponMAC10 C_WeaponMAC10
#endif


class CWeaponMAC10 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMAC10, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMAC10() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_MAC10; }

private:
	CWeaponMAC10( const CWeaponMAC10 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMAC10, DT_WeaponMAC10 )

BEGIN_NETWORK_TABLE( CWeaponMAC10, DT_WeaponMAC10 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMAC10 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mac10, CWeaponMAC10 );
PRECACHE_WEAPON_REGISTER( weapon_mac10 );
