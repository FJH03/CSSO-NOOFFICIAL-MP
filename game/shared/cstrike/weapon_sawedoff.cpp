//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "fx_cs_shared.h"

#if defined( CLIENT_DLL )
	#define CWeaponSawedOff C_WeaponSawedOff
	#include "c_cs_player.h"
#else
	#include "cs_player.h"
#endif


class CWeaponSawedOff : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSawedOff, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSawedOff();

	virtual void PrimaryAttack( void );
	virtual bool Reload( void );
	virtual void WeaponIdle( void );

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_SAWEDOFF; }

	virtual int GetShotgunReloadState( void ) { return m_reloadState; }

private:
	CWeaponSawedOff( const CWeaponSawedOff & );

	float m_flPumpTime;
	CNetworkVar( int, m_reloadState );	// special reload state for shotgun
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSawedOff, DT_WeaponSawedOff )

BEGIN_NETWORK_TABLE( CWeaponSawedOff, DT_WeaponSawedOff )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_reloadState ) )
#else
	SendPropInt( SENDINFO( m_reloadState ), 2, SPROP_UNSIGNED )
#endif
END_NETWORK_TABLE()

#if defined(CLIENT_DLL)
BEGIN_PREDICTION_DATA( CWeaponSawedOff )
	DEFINE_PRED_FIELD( m_reloadState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_sawedoff, CWeaponSawedOff );
PRECACHE_WEAPON_REGISTER( weapon_sawedoff );

CWeaponSawedOff::CWeaponSawedOff( void )
{
	m_flPumpTime = 0.0f;
	m_reloadState = 0;
}

void CWeaponSawedOff::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	const CCSWeaponInfo& weaponInfo = GetCSWpnData();

	float flCycleTime = GetCSWpnData().m_flCycleTime[m_weaponMode];

	// don't fire underwater
	if ( pPlayer->GetWaterLevel() == WL_Eyes )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15f;
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		Reload();

		if ( m_iClip1 == 0 )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
		}

		return;
	}

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	//pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	//pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	pPlayer->DoMuzzleFlash();

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Dispatch the FX right away with full accuracy.
	float flCurAttack = CalculateNextAttackTime( flCycleTime );

	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->GetFinalAimAngle(),
		GetCSWeaponID(),
		Primary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetInaccuracy(),
		GetSpread(), // flSpread
		flCurAttack,
		SINGLE,
		m_flRecoilIndex );

	// are we firing the last round in the clip?
	if ( m_iClip1 == 1 )
	{
		SetWeaponIdleTime( gpGlobals->curtime + 0.875f );
	}
	else
	{
		m_flPumpTime = gpGlobals->curtime + 0.5f;
		SetWeaponIdleTime( gpGlobals->curtime + 2.5f );
	}

	m_reloadState = 0;

	// update accuracy
	m_fAccuracyPenalty += weaponInfo.m_fInaccuracyImpulseFire[Primary_Mode];

	// table driven recoil
	Recoil( Primary_Mode );

	++pPlayer->m_iShotsFired;
	m_flRecoilIndex += 1.0f;
	--m_iClip1;
}

bool CWeaponSawedOff::Reload( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 || m_iClip1 == GetMaxClip1() )
		return true;

	// don't reload until recoil is done
	if ( m_flNextPrimaryAttack > gpGlobals->curtime )
		return true;

	//MIKETODO: shotgun reloading (wait until we get content)

	// check to see if we're ready to reload
	if ( m_reloadState == 0 )
	{
		pPlayer->SetAnimation( PLAYER_RELOAD );

		SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );
		m_reloadState = 1;
		pPlayer->m_flNextAttack = gpGlobals->curtime + 0.5f;
		SetWeaponIdleTime( gpGlobals->curtime + 0.5f );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

#ifdef GAME_DLL
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_START );
#endif

		return true;
	}
	else if ( m_reloadState == 1 )
	{
		if ( m_flTimeWeaponIdle > gpGlobals->curtime )
			return true;
		// was waiting for gun to move to side
		m_reloadState = 2;

		SendWeaponAnim( ACT_VM_RELOAD );
		SetWeaponIdleTime( gpGlobals->curtime + 0.5f );
#ifdef GAME_DLL
		// [mlowrance] Only play the looping anim
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_LOOP );
#endif
	}
	else
	{
		// Add them to the clip
		m_iClip1 += 1;

#ifdef GAME_DLL
		SendReloadEvents();
#endif

		GiveReserveAmmo( AMMO_POSITION_PRIMARY, -1, true );
		m_reloadState = 1;
	}

	return true;
}

void CWeaponSawedOff::WeaponIdle( void )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( m_flPumpTime && m_flPumpTime < gpGlobals->curtime )
	{
		// play pumping sound
		m_flPumpTime = 0.0f;
	}

	if ( m_flTimeWeaponIdle < gpGlobals->curtime )
	{
		if ( m_iClip1 == 0 && m_reloadState == 0 && GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) )
		{
			Reload();
		}
		else if ( m_reloadState != 0 )
		{
			if ( m_iClip1 != GetMaxClip1() && GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) )
			{
				Reload();
			}
			else
			{
				// reload debounce has timed out
				//MIKETODO: shotgun anims
				SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );

#if defined( GAME_DLL )
				// [mlowrance] play the finish for 3rd person
				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_END );
#endif
				// play cocking sound
				m_reloadState = 0;
				SetWeaponIdleTime( gpGlobals->curtime + 1.5f );
			}
		}
		else
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}
