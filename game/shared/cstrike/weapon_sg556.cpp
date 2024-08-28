//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"


#if defined( CLIENT_DLL )

	#define CWeaponSG556 C_WeaponSG556
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponSG556 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSG556, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSG556();

	virtual void SecondaryAttack();
	virtual void PrimaryAttack();

	virtual float GetMaxSpeed() const;
	virtual bool Reload();
	virtual bool Deploy();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_SG556; }

#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:

	CWeaponSG556( const CWeaponSG556 & );

	void SG556Fire( float flSpread, bool bZoomed );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSG556, DT_WeaponSG556 )

BEGIN_NETWORK_TABLE( CWeaponSG556, DT_WeaponSG556 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSG556 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sg556, CWeaponSG556 );
PRECACHE_WEAPON_REGISTER( weapon_sg556 );



CWeaponSG556::CWeaponSG556()
{
}


void CWeaponSG556::SecondaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
	{
		pPlayer->SetFOV( pPlayer, 55, 0.2f );
		m_weaponMode = Secondary_Mode;
	}
	else if (pPlayer->GetFOV() == 55)
	{
		pPlayer->SetFOV( pPlayer, 0, 0.15f );
		m_weaponMode = Secondary_Mode;
	}
	else 
	{
		//FIXME: This seems wrong
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
		m_weaponMode = Primary_Mode;
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}

void CWeaponSG556::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	bool bZoomed = pPlayer->GetFOV() < pPlayer->GetDefaultFOV();

	float flCycleTime = GetCSWpnData().m_flCycleTime;

	if ( bZoomed )
		flCycleTime = 0.135f;

	if ( !CSBaseGunFire( flCycleTime, m_weaponMode ) )
		return;
}


float CWeaponSG556::GetMaxSpeed() const
{
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer || pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
		return BaseClass::GetMaxSpeed();
	else
		return 200; // zoomed in.
}	


bool CWeaponSG556::Reload()
{
	m_weaponMode = Primary_Mode;
	return BaseClass::Reload();
}

bool CWeaponSG556::Deploy()
{
	m_weaponMode = Primary_Mode;
	return BaseClass::Deploy();
}
