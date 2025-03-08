//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponTec9 C_WeaponTec9
#endif


class CWeaponTec9 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponTec9, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponTec9() {}

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_TEC9; }

private:
	CWeaponTec9( const CWeaponTec9 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponTec9, DT_WeaponTec9 )

BEGIN_NETWORK_TABLE( CWeaponTec9, DT_WeaponTec9 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponTec9 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_tec9, CWeaponTec9 );
PRECACHE_WEAPON_REGISTER( weapon_tec9 );
