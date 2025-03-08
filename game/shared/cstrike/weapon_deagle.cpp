//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponDeagle C_WeaponDeagle
#endif


class CWeaponDeagle : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponDeagle, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponDeagle() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_DEAGLE; }

private:
	CWeaponDeagle( const CWeaponDeagle & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDeagle, DT_WeaponDeagle )

BEGIN_NETWORK_TABLE( CWeaponDeagle, DT_WeaponDeagle )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponDeagle )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_deagle, CWeaponDeagle );
PRECACHE_WEAPON_REGISTER( weapon_deagle );
