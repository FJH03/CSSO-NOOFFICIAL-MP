//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponAK47 C_WeaponAK47
#endif


class CWeaponAK47 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponAK47, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponAK47() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_AK47; }

private:
	CWeaponAK47( const CWeaponAK47 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAK47, DT_WeaponAK47 )

BEGIN_NETWORK_TABLE( CWeaponAK47, DT_WeaponAK47 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponAK47 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_ak47, CWeaponAK47 );
PRECACHE_WEAPON_REGISTER( weapon_ak47 );
