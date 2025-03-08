//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponGalilAR C_WeaponGalilAR
#endif


class CWeaponGalilAR : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponGalilAR, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponGalilAR() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_GALILAR; }

private:
	CWeaponGalilAR( const CWeaponGalilAR & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGalilAR, DT_WeaponGalilAR )

BEGIN_NETWORK_TABLE( CWeaponGalilAR, DT_WeaponGalilAR )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGalilAR )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_galilar, CWeaponGalilAR );
PRECACHE_WEAPON_REGISTER( weapon_galilar );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_galil, CWeaponGalilAR ); // for backwards compatibility
#endif

void CWeaponGalilAR::Spawn()
{
	SetClassname( "weapon_galilar" ); // for backwards compatibility
	BaseClass::Spawn();
}
