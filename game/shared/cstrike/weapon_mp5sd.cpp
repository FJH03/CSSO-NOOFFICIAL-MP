//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponMP5SD C_WeaponMP5SD
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponMP5SD : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponMP5SD, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMP5SD();

	virtual void Spawn();
	virtual void PrimaryAttack();
	virtual bool Deploy();
	virtual bool Reload();

 	virtual float GetInaccuracy() const;

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_MP5SD; }


private:
	CWeaponMP5SD( const CWeaponMP5SD & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP5SD, DT_WeaponMP5SD )

BEGIN_NETWORK_TABLE( CWeaponMP5SD, DT_WeaponMP5SD )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP5SD )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp5sd, CWeaponMP5SD );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_mp5navy, CWeaponMP5SD ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_mp5sd );



CWeaponMP5SD::CWeaponMP5SD()
{
}

void CWeaponMP5SD::Spawn()
{
	BaseClass::Spawn();

	m_flAccuracy = 0.0;
}


bool CWeaponMP5SD::Deploy( )
{
	bool ret = BaseClass::Deploy();

	m_flAccuracy = 0.0;

	return ret;
}

bool CWeaponMP5SD::Reload( )
{
	bool ret = BaseClass::Reload();

	m_flAccuracy = 0.0;

	return ret;
}

float CWeaponMP5SD::GetInaccuracy() const
{
	if ( weapon_accuracy_model.GetInt() == 1 )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return 0.0f;
	
		if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			return 0.2f * m_flAccuracy;
		else
			return 0.04f * m_flAccuracy;
	}
	else
		return BaseClass::GetInaccuracy();
}

void CWeaponMP5SD::PrimaryAttack( void )
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
