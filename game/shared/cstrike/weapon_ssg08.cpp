//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponSSG08 C_WeaponSSG08
#endif


class CWeaponSSG08 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSSG08, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSSG08() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_SSG08; }

private:
	CWeaponSSG08( const CWeaponSSG08 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSSG08, DT_WeaponSSG08 )

BEGIN_NETWORK_TABLE( CWeaponSSG08, DT_WeaponSSG08 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSSG08 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_ssg08, CWeaponSSG08 );
PRECACHE_WEAPON_REGISTER( weapon_ssg08 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_scout, CWeaponSSG08 );
#endif

void CWeaponSSG08::Spawn()
{
	SetClassname( "weapon_ssg08" ); // for backwards compatibility
	BaseClass::Spawn();
}
