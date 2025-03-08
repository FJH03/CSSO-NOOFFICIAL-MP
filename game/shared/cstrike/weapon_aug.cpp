//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponAUG C_WeaponAUG
#endif


class CWeaponAUG : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponAUG, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponAUG() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_AUG; }

private:
	CWeaponAUG( const CWeaponAUG & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAUG, DT_WeaponAUG )

BEGIN_NETWORK_TABLE( CWeaponAUG, DT_WeaponAUG )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponAUG )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_aug, CWeaponAUG );
PRECACHE_WEAPON_REGISTER( weapon_aug );
