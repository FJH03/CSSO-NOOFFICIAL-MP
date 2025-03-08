//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponSG556 C_WeaponSG556
#endif


class CWeaponSG556 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSG556, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSG556() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_SG556; }

private:
	CWeaponSG556( const CWeaponSG556 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSG556, DT_WeaponSG556 )

BEGIN_NETWORK_TABLE( CWeaponSG556, DT_WeaponSG556 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSG556 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sg556, CWeaponSG556 );
PRECACHE_WEAPON_REGISTER( weapon_sg556 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_sg552, CWeaponSG556 ); // for backwards compatibility
#endif

void CWeaponSG556::Spawn()
{
	SetClassname( "weapon_sg556" ); // for backwards compatibility
	BaseClass::Spawn();
}
