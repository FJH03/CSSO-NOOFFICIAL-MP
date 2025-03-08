//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponFiveSeven C_WeaponFiveSeven
#endif


class CWeaponFiveSeven : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponFiveSeven, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponFiveSeven() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_FIVESEVEN; }

private:
	CWeaponFiveSeven( const CWeaponFiveSeven & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFiveSeven, DT_WeaponFiveSeven )

BEGIN_NETWORK_TABLE( CWeaponFiveSeven, DT_WeaponFiveSeven )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponFiveSeven )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_fiveseven, CWeaponFiveSeven );
PRECACHE_WEAPON_REGISTER( weapon_fiveseven );
