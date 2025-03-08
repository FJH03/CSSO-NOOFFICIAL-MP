//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponFAMAS C_WeaponFAMAS
#endif


class CWeaponFAMAS : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponFAMAS, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponFAMAS() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_FAMAS; }

private:
	CWeaponFAMAS( const CWeaponFAMAS & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFAMAS, DT_WeaponFAMAS )

BEGIN_NETWORK_TABLE( CWeaponFAMAS, DT_WeaponFAMAS )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponFAMAS )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_famas, CWeaponFAMAS );
PRECACHE_WEAPON_REGISTER( weapon_famas );
