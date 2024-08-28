//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponBizon C_WeaponBizon
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponBizon : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponBizon, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponBizon();

	virtual void PrimaryAttack();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_BIZON; }


private:
	CWeaponBizon( const CWeaponBizon & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBizon, DT_WeaponBizon )

BEGIN_NETWORK_TABLE( CWeaponBizon, DT_WeaponBizon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBizon )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_bizon, CWeaponBizon );
PRECACHE_WEAPON_REGISTER( weapon_bizon );



CWeaponBizon::CWeaponBizon()
{
}

void CWeaponBizon::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime, Primary_Mode ) )
		return;

	// CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
	pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// Kick the gun based on the state of the player.
	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack (0.9, 0.475, 0.4375, 0.053125, 5, 3, 6);	
	else if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (0.5, 0.275, 0.25, 0.0375, 3, 2, 10);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.225, 0.15, 0.125, 0.01875, 2, 1, 10);
	else
		pPlayer->KickBack (0.25, 0.175, 0.15625, 0.025, 2.25, 1.25, 10);
}