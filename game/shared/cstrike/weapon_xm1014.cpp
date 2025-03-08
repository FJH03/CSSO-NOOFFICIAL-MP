//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponXM1014 C_WeaponXM1014
#endif


class CWeaponXM1014 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponXM1014, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponXM1014() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_XM1014; }

private:
	CWeaponXM1014( const CWeaponXM1014 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponXM1014, DT_WeaponXM1014 )

BEGIN_NETWORK_TABLE( CWeaponXM1014, DT_WeaponXM1014 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponXM1014 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_xm1014, CWeaponXM1014 );
PRECACHE_WEAPON_REGISTER( weapon_xm1014 );
