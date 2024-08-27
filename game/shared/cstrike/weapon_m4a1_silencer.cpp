//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"

#if defined( CLIENT_DLL )

	#define CWeaponM4A1 C_WeaponM4A1
	#include "c_cs_player.h"

#else

	#include "cs_player.h"

#endif


class CWeaponM4A1 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponM4A1, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	
	CWeaponM4A1();

	virtual void Spawn();
	virtual void Precache();

	virtual void SecondaryAttack();
	virtual void PrimaryAttack();
	virtual bool Deploy();
	virtual bool Reload();
	virtual void WeaponIdle();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void Drop( const Vector &vecVelocity );

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_M4A1; }

	// return true if this weapon has a silencer equipped
	virtual bool IsSilenced( void ) const				{ return m_bSilencerOn; }

	virtual Activity GetDeployActivity( void );

#ifdef CLIENT_DLL
	virtual int GetMuzzleFlashStyle( void );
#endif

	virtual const char		*GetWorldModel( void ) const;
	virtual int				GetWorldModelIndex( void );

private:

	CWeaponM4A1( const CWeaponM4A1 & );

	void DoFireEffects();

	CNetworkVar( bool, m_bSilencerOn );
	CNetworkVar( float, m_flDoneSwitchingSilencer );	// soonest time switching the silencer will be complete

	int m_silencedModelIndex;
	bool m_inPrecache;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM4A1, DT_WeaponM4A1 )

BEGIN_NETWORK_TABLE( CWeaponM4A1, DT_WeaponM4A1 )
	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bSilencerOn ) ),
		RecvPropTime( RECVINFO( m_flDoneSwitchingSilencer ) ),
	#else
		SendPropBool( SENDINFO( m_bSilencerOn ) ),
		SendPropTime( SENDINFO( m_flDoneSwitchingSilencer ) ),
	#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM4A1 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m4a1_silencer, CWeaponM4A1 );
#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( weapon_m4a1, CWeaponM4A1 ); // for backwards compatibility
#endif
PRECACHE_WEAPON_REGISTER( weapon_m4a1_silencer );



CWeaponM4A1::CWeaponM4A1()
{
	m_bSilencerOn = false;
	m_flDoneSwitchingSilencer = 0.0f;
	m_inPrecache = false;
}


void CWeaponM4A1::Spawn( )
{
	BaseClass::Spawn();

	m_bSilencerOn = false;
	m_weaponMode = Primary_Mode;
	m_flDoneSwitchingSilencer = 0.0f;
	m_bDelayFire = true;
}


void CWeaponM4A1::Precache()
{
	m_inPrecache = true;
	BaseClass::Precache();

	m_silencedModelIndex = CBaseEntity::PrecacheModel( GetCSWpnData().m_szSilencerModel );
	m_inPrecache = false;
}


int CWeaponM4A1::GetWorldModelIndex( void )
{
	if ( !m_bSilencerOn || m_inPrecache )
	{
		return m_iWorldModelIndex;
	}
	else
	{
		return m_silencedModelIndex;
	}
}


const char * CWeaponM4A1::GetWorldModel( void ) const
{
	if ( !m_bSilencerOn || m_inPrecache )
	{
		return BaseClass::GetWorldModel();
	}
	else
	{
		return GetCSWpnData().m_szSilencerModel;
	}
}


bool CWeaponM4A1::Deploy()
{
	bool ret = BaseClass::Deploy();

	m_flDoneSwitchingSilencer = 0.0f;
	m_bDelayFire = true;

	return ret;
}

Activity CWeaponM4A1::GetDeployActivity( void )
{
	if( IsSilenced() )
	{
		return ACT_VM_DRAW_SILENCED;
	}
	else
	{
		return ACT_VM_DRAW;
	}
}

bool CWeaponM4A1::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( gpGlobals->curtime < m_flDoneSwitchingSilencer )
	{
		// still switching the silencer.  Cancel the switch.
		m_bSilencerOn = !m_bSilencerOn;
		m_weaponMode = m_bSilencerOn ? Secondary_Mode : Primary_Mode;
		SetWeaponModelIndex( GetWorldModel() );
	}

	return BaseClass::Holster( pSwitchingTo );
}

void CWeaponM4A1::Drop( const Vector &vecVelocity )
{
	if ( gpGlobals->curtime < m_flDoneSwitchingSilencer )
	{
		// still switching the silencer.  Cancel the switch.
		m_bSilencerOn = !m_bSilencerOn;
		m_weaponMode = m_bSilencerOn ? Secondary_Mode : Primary_Mode;
		SetWeaponModelIndex( GetWorldModel() );
	}

	BaseClass::Drop( vecVelocity );
}

void CWeaponM4A1::SecondaryAttack()
{
	if ( m_bSilencerOn )
	{
		m_bSilencerOn = false;
		m_weaponMode = Primary_Mode;
		SendWeaponAnim( ACT_VM_DETACH_SILENCER );
	}
	else
	{
		m_bSilencerOn = true;
		m_weaponMode = Secondary_Mode;
		SendWeaponAnim( ACT_VM_ATTACH_SILENCER );
	}
	m_flDoneSwitchingSilencer = gpGlobals->curtime + 2;

	m_flNextSecondaryAttack = gpGlobals->curtime + 2;
	m_flNextPrimaryAttack = gpGlobals->curtime + 2;
	SetWeaponIdleTime( gpGlobals->curtime + 2 );

	SetWeaponModelIndex( GetWorldModel() );
}

void CWeaponM4A1::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], m_weaponMode ) )
		return;

	if ( m_bSilencerOn )
		 SendWeaponAnim( ACT_VM_PRIMARYATTACK_SILENCED );
}


void CWeaponM4A1::DoFireEffects()
{
	if ( !m_bSilencerOn )
	{
		CCSPlayer *pPlayer = GetPlayerOwner();
		if ( pPlayer )
		{
			pPlayer->DoMuzzleFlash();
		}
	}
}

bool CWeaponM4A1::Reload()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if (pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= 0)
		return false;

	int iResult = 0;
	
	if ( m_bSilencerOn )
		 iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD_SILENCED );
	else
		 iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );

	if ( !iResult )
		return false;

	pPlayer->SetAnimation( PLAYER_RELOAD );

	if ((iResult) && (pPlayer->GetFOV() != pPlayer->GetDefaultFOV()))
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
	}

	pPlayer->m_iShotsFired = 0;
	m_bDelayFire = false;
	return true;
}


void CWeaponM4A1::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	if ( m_iClip1 != 0 )
	{
		SetWeaponIdleTime( gpGlobals->curtime + GetCSWpnData().m_flIdleInterval );
		if ( m_bSilencerOn )
			SendWeaponAnim( ACT_VM_IDLE_SILENCED );
		else
			SendWeaponAnim( ACT_VM_IDLE );
	}
}


#ifdef CLIENT_DLL
int CWeaponM4A1::GetMuzzleFlashStyle( void )
{
	if( m_bSilencerOn )
	{
		return CS_MUZZLEFLASH_NONE;
	}
	else
	{
		return CS_MUZZLEFLASH_X;
	}
}
#endif
