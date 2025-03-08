//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponM4A1 C_WeaponM4A1
#endif


class CWeaponM4A1 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponM4A1, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponM4A1() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_M4A1; }

private:
	CWeaponM4A1( const CWeaponM4A1 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM4A1, DT_WeaponM4A1 )

BEGIN_NETWORK_TABLE( CWeaponM4A1, DT_WeaponM4A1 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM4A1 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m4a1_silencer, CWeaponM4A1 );
PRECACHE_WEAPON_REGISTER( weapon_m4a1_silencer );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_m4a1, CWeaponM4A1 ); // for backwards compatibility
#endif

void CWeaponM4A1::Spawn()
{
	SetClassname( "weapon_m4a1_silencer" ); // for backwards compatibility
	BaseClass::Spawn();
}
