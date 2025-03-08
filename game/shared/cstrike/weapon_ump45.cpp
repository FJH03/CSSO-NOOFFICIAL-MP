//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponUMP45 C_WeaponUMP45
#endif


class CWeaponUMP45 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponUMP45, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponUMP45() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_UMP45; }

private:
	CWeaponUMP45( const CWeaponUMP45 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponUMP45, DT_WeaponUMP45 )

BEGIN_NETWORK_TABLE( CWeaponUMP45, DT_WeaponUMP45 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponUMP45 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_ump45, CWeaponUMP45 );
PRECACHE_WEAPON_REGISTER( weapon_ump45 );
