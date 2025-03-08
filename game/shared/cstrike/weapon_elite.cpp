//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )
	#define CWeaponElite C_WeaponElite
#endif


class CWeaponElite : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponElite, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponElite() {}

	virtual void Precache();

	virtual void PrimaryAttack();

	// We overload this so we can translate left/right fire activities
	virtual bool SendWeaponAnim( int iActivity );

	virtual void WeaponIdle();
	
	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_ELITE; }

#ifdef CLIENT_DLL
	virtual int GetMuzzleAttachmentIndex( C_BaseAnimating* pAnimating, bool isThirdPerson );
#endif

protected:
	bool FiringLeft() const;

private:
	
	CWeaponElite( const CWeaponElite & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponElite, DT_WeaponElite )

BEGIN_NETWORK_TABLE( CWeaponElite, DT_WeaponElite )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponElite )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_elite, CWeaponElite );
PRECACHE_WEAPON_REGISTER( weapon_elite );


void CWeaponElite::Precache()
{
	BaseClass::Precache();

	PrecacheModel( "models/weapons/w_eq_eholster_elite.mdl" );
	PrecacheModel( "models/weapons/w_eq_eholster.mdl" );
	PrecacheModel( "models/weapons/w_pist_elite_single.mdl" );
}


bool CWeaponElite::FiringLeft() const
{
	// fire left-hand gun with even number of bullets left
	return (m_iClip1 & 1) == 0;
}

void CWeaponElite::PrimaryAttack()
{
	m_weaponMode = FiringLeft() ? Primary_Mode : Secondary_Mode;
	CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], m_weaponMode );
}

bool CWeaponElite::SendWeaponAnim( int iActivity )
 {
 	if ( iActivity == ACT_VM_PRIMARYATTACK )
 	{
 		if ( FiringLeft() )
 		{
 			if ( m_iClip1 > 2 )
 				iActivity = ACT_VM_PRIMARYATTACK;
 			else
 				iActivity = ACT_VM_DRYFIRE_LEFT;
 		}
 		else
 		{
 			if ( m_iClip1 > 2 )
 				iActivity = ACT_VM_SECONDARYATTACK;
 			else
 				iActivity = ACT_VM_DRYFIRE;
 		}
 	}
 	return BaseClass::SendWeaponAnim( iActivity );
 }
 
void CWeaponElite::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	if ( m_iClip1 < 2 )
		return;

//	NB. If we show one or more empty guns when idling, we'll get visual pops when holstering or drawing weapons
// 	if ( m_iClip1 == 1 )
// 		SendWeaponAnim( ACT_VM_IDLE_EMPTY_LEFT );

	SendWeaponAnim( ACT_VM_IDLE );
	SetWeaponIdleTime( gpGlobals->curtime + GetCSWpnData().m_flIdleInterval );
}

#ifdef CLIENT_DLL

int CWeaponElite::GetMuzzleAttachmentIndex( C_BaseAnimating* pAnimating, bool isThirdPerson )
{
	if ( !pAnimating )
		return -1;

	if ( isThirdPerson )
	{
		if ( FiringLeft() )
			return pAnimating->LookupAttachment( "muzzle_flash2" );
		else
			return pAnimating->LookupAttachment( "muzzle_flash" );
	}
	else
	{
		if ( FiringLeft() )
			return pAnimating->LookupAttachment( "1" );
		else
			return pAnimating->LookupAttachment( "2" );
	}
}

#endif