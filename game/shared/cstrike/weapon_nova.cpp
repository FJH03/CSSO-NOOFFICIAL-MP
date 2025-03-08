//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponNova C_WeaponNova
#endif


class CWeaponNova : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponNova, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponNova() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_NOVA; }

private:
	CWeaponNova( const CWeaponNova & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponNova, DT_WeaponNova )

BEGIN_NETWORK_TABLE( CWeaponNova, DT_WeaponNova )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponNova )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_nova, CWeaponNova );
PRECACHE_WEAPON_REGISTER( weapon_nova );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_m3, CWeaponNova );
#endif

void CWeaponNova::Spawn()
{
	SetClassname( "weapon_nova" ); // for backwards compatibility
	BaseClass::Spawn();
}
