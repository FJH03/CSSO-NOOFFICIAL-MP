//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponG3SG1 C_WeaponG3SG1
#endif


class CWeaponG3SG1 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponG3SG1, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponG3SG1() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_G3SG1; }

private:
	CWeaponG3SG1( const CWeaponG3SG1 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponG3SG1, DT_WeaponG3SG1 )

BEGIN_NETWORK_TABLE( CWeaponG3SG1, DT_WeaponG3SG1 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponG3SG1 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_g3sg1, CWeaponG3SG1 );
PRECACHE_WEAPON_REGISTER( weapon_g3sg1 );
