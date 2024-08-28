//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponMP9 C_WeaponMP9
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponMP9 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMP9, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMP9();

	virtual void PrimaryAttack();
	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_MP9; }
	virtual bool IsSilenced( void ) const				{ return false; }

 	virtual float GetInaccuracy() const;

private:

	CWeaponMP9( const CWeaponMP9 & );

	void DoFireEffects( void );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP9, DT_WeaponMP9 )

BEGIN_NETWORK_TABLE( CWeaponMP9, DT_WeaponMP9 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP9 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp9, CWeaponMP9 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_tmp, CWeaponMP9 ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_mp9 );


CWeaponMP9::CWeaponMP9()
{
}


float CWeaponMP9::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;
	
		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			return 0.25f * m_flAccuracy;
		else
			return 0.03f * m_flAccuracy;
	}
	else
		return BaseClass::GetInaccuracy();
}

void CWeaponMP9::PrimaryAttack( void )
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

	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack (1.1, 0.5, 0.35, 0.045, 4.5, 3.5, 6);
	else if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (0.8, 0.4, 0.2, 0.03, 3, 2.5, 7);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.7, 0.35, 0.125, 0.025, 2.5, 2, 10);
	else
		pPlayer->KickBack (0.725, 0.375, 0.15, 0.025, 2.75, 2.25, 9);
}

void CWeaponMP9::DoFireEffects( void )
{
	// TMP is silenced, so do nothing
}
