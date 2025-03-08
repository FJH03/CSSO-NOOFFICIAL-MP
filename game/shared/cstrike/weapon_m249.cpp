//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponM249 C_WeaponM249
#endif


class CWeaponM249 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponM249, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponM249() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_M249; }

private:
	CWeaponM249( const CWeaponM249 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM249, DT_WeaponM249 )

BEGIN_NETWORK_TABLE( CWeaponM249, DT_WeaponM249 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM249 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m249, CWeaponM249 );
PRECACHE_WEAPON_REGISTER( weapon_m249 );
