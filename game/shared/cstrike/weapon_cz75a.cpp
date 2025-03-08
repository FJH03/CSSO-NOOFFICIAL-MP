//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponCZ75A C_WeaponCZ75A
#endif


class CWeaponCZ75A : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponCZ75A, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponCZ75A() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_CZ75A; }

private:
	CWeaponCZ75A( const CWeaponCZ75A & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCZ75A, DT_WeaponCZ75A )

BEGIN_NETWORK_TABLE( CWeaponCZ75A, DT_WeaponCZ75A )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponCZ75A )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_cz75a, CWeaponCZ75A );
PRECACHE_WEAPON_REGISTER( weapon_cz75a );
