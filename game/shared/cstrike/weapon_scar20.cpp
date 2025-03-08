//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponSCAR20 C_WeaponSCAR20
#endif


class CWeaponSCAR20 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSCAR20, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSCAR20() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_SCAR20; }

private:
	CWeaponSCAR20( const CWeaponSCAR20 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSCAR20, DT_WeaponSCAR20 )

BEGIN_NETWORK_TABLE( CWeaponSCAR20, DT_WeaponSCAR20 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSCAR20 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_scar20, CWeaponSCAR20 );
PRECACHE_WEAPON_REGISTER( weapon_scar20 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_sg550, CWeaponSCAR20 ); // for backwards compatibility
#endif

void CWeaponSCAR20::Spawn()
{
	SetClassname( "weapon_scar20" ); // for backwards compatibility
	BaseClass::Spawn();
}
