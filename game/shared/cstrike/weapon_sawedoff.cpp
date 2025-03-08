//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponSawedOff C_WeaponSawedOff
#endif


class CWeaponSawedOff : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSawedOff, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSawedOff() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_SAWEDOFF; }

private:
	CWeaponSawedOff( const CWeaponSawedOff & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSawedOff, DT_WeaponSawedOff )

BEGIN_NETWORK_TABLE( CWeaponSawedOff, DT_WeaponSawedOff )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSawedOff )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sawedoff, CWeaponSawedOff );
PRECACHE_WEAPON_REGISTER( weapon_sawedoff );
