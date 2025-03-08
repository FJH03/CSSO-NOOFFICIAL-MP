//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponGlock C_WeaponGlock
#endif


class CWeaponGlock : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponGlock, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponGlock() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_GLOCK; }

private:
	CWeaponGlock( const CWeaponGlock & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGlock, DT_WeaponGlock )

BEGIN_NETWORK_TABLE( CWeaponGlock, DT_WeaponGlock )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGlock )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_glock, CWeaponGlock );
PRECACHE_WEAPON_REGISTER( weapon_glock );
