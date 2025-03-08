//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponRevolver C_WeaponRevolver
#endif


class CWeaponRevolver : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponRevolver, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponRevolver() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_REVOLVER; }

private:
	CWeaponRevolver( const CWeaponRevolver & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRevolver, DT_WeaponRevolver )

BEGIN_NETWORK_TABLE( CWeaponRevolver, DT_WeaponRevolver )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponRevolver )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_revolver, CWeaponRevolver );
PRECACHE_WEAPON_REGISTER( weapon_revolver );
