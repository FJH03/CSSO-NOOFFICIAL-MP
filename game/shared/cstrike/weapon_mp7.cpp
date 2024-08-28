//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponMP7 C_WeaponMP7
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponMP7 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMP7, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMP7();

	virtual void PrimaryAttack();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_MP7; }


private:
	CWeaponMP7( const CWeaponMP7 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP7, DT_WeaponMP7 )

BEGIN_NETWORK_TABLE( CWeaponMP7, DT_WeaponMP7 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP7 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp7, CWeaponMP7 );
PRECACHE_WEAPON_REGISTER( weapon_mp7 );



CWeaponMP7::CWeaponMP7()
{
}

void CWeaponMP7::PrimaryAttack( void )
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
		pPlayer->KickBack (0.9, 0.475, 0.35, 0.0425, 5, 3, 6);	
	else if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (0.5, 0.275, 0.2, 0.03, 3, 2, 10);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.225, 0.15, 0.1, 0.015, 2, 1, 10);
	else
		pPlayer->KickBack (0.25, 0.175, 0.125, 0.02, 2.25, 1.25, 10);
}