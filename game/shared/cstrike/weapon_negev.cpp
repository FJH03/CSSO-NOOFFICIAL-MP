//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponNegev C_WeaponNegev
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponNegev : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponNegev, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponNegev();

	virtual void PrimaryAttack();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_NEGEV; }


private:
	CWeaponNegev( const CWeaponNegev & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponNegev, DT_WeaponNegev )

BEGIN_NETWORK_TABLE( CWeaponNegev, DT_WeaponNegev )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponNegev )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_negev, CWeaponNegev );
PRECACHE_WEAPON_REGISTER( weapon_negev );



CWeaponNegev::CWeaponNegev()
{
}

void CWeaponNegev::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime, Primary_Mode ) )
		return;
	
	pPlayer = GetPlayerOwner();

	// CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
	if ( !pPlayer )
		return;

	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack (1.8, 0.65, 0.225, 0.075, 5, 3.5, 8);
	
	else if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (1.1, 0.5, 0.15, 0.03, 4, 3, 8);
	
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.75, 0.325, 0.125, 0.0125, 3.5, 2.5, 9);
	
	else
		pPlayer->KickBack (0.8, 0.35, 0.15, 0.015, 3.75, 3, 9);
}
