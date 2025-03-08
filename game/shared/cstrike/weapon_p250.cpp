//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponP250 C_WeaponP250
#endif


class CWeaponP250 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponP250, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponP250() {}
	virtual void Spawn();

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_P250; }

private:
	CWeaponP250( const CWeaponP250 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponP250, DT_WeaponP250 )

BEGIN_NETWORK_TABLE( CWeaponP250, DT_WeaponP250 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponP250 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_p250, CWeaponP250 );
PRECACHE_WEAPON_REGISTER( weapon_p250 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_p228, CWeaponP250 ); // for backwards compatibility
#endif

void CWeaponP250::Spawn()
{
	SetClassname( "weapon_p250" ); // for backwards compatibility
	BaseClass::Spawn();
}
