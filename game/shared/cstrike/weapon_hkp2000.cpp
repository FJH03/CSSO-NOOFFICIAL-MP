//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponHKP2000 C_WeaponHKP2000
#endif


class CWeaponHKP2000 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponHKP2000, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponHKP2000() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_HKP2000; }

private:
	CWeaponHKP2000( const CWeaponHKP2000 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHKP2000, DT_WeaponHKP2000 )

BEGIN_NETWORK_TABLE( CWeaponHKP2000, DT_WeaponHKP2000 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponHKP2000 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_hkp2000, CWeaponHKP2000 );
PRECACHE_WEAPON_REGISTER( weapon_hkp2000 );
