//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponMP5SD C_WeaponMP5SD
#endif


class CWeaponMP5SD : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMP5SD, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMP5SD() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_MP5SD; }

private:
	CWeaponMP5SD( const CWeaponMP5SD & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP5SD, DT_WeaponMP5SD )

BEGIN_NETWORK_TABLE( CWeaponMP5SD, DT_WeaponMP5SD )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP5SD )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp5sd, CWeaponMP5SD );
PRECACHE_WEAPON_REGISTER( weapon_mp5sd );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_mp5navy, CWeaponMP5SD ); // for backwards compatibility
#endif

void CWeaponMP5SD::Spawn()
{
	SetClassname( "weapon_mp5sd" ); // for backwards compatibility
	BaseClass::Spawn();
}
