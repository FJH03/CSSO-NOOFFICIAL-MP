//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponM4A4 C_WeaponM4A4
#endif


class CWeaponM4A4 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponM4A4, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponM4A4() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_M4A4; }

private:
	CWeaponM4A4( const CWeaponM4A4 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM4A4, DT_WeaponM4A4 )

BEGIN_NETWORK_TABLE( CWeaponM4A4, DT_WeaponM4A4 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM4A4 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m4a4, CWeaponM4A4 );
PRECACHE_WEAPON_REGISTER( weapon_m4a4 );
