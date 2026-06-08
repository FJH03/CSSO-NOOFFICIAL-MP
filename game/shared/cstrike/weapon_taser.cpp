//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "cs_gamerules.h"

#if defined( CLIENT_DLL )
	#define CWeaponTaser C_WeaponTaser
	#include "c_cs_player.h"
#else
	#include "cs_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define TASER_BIRTHDAY_PARTICLES	"weapon_confetti"
#define TASER_BIRTHDAY_SOUND		"Weapon_PartyHorn.Single"
#define TASER_THINK_CONTEXT			"TASERTHINK"

ConVar mp_taser_recharge_time( "mp_taser_recharge_time", "-1", FCVAR_REPLICATED, "Determines recharge time for taser. -1 = disabled." );

class CWeaponTaser : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponTaser, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponTaser();

	virtual void Spawn();
	virtual void Precache();
	virtual void PrimaryAttack( void );

#if defined( GAME_DLL )
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void ItemPostFrame();
	void TaserThink();
#else
	virtual float GetTaserRechargePercentage();
#endif

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_TASER; }

private:
	CWeaponTaser( const CWeaponTaser& );
	CNetworkVar( float, m_fFireTime );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponTaser, DT_WeaponTaser )

BEGIN_NETWORK_TABLE( CWeaponTaser, DT_WeaponTaser )
#ifdef CLIENT_DLL
	RecvPropTime( RECVINFO( m_fFireTime ) ),
#else
	SendPropTime( SENDINFO( m_fFireTime ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponTaser )
END_PREDICTION_DATA()

#ifdef GAME_DLL
BEGIN_DATADESC( CWeaponTaser )
	DEFINE_THINKFUNC( TaserThink ),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( weapon_taser, CWeaponTaser );
PRECACHE_WEAPON_REGISTER( weapon_taser );

CWeaponTaser::CWeaponTaser() :
	m_fFireTime(0.0f)
{
}

void CWeaponTaser::Spawn()
{
	BaseClass::Spawn();

#ifdef GAME_DLL
	SetContextThink( &CWeaponTaser::TaserThink, gpGlobals->curtime, TASER_THINK_CONTEXT );
#endif
}

void CWeaponTaser::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem( TASER_BIRTHDAY_PARTICLES );
	PrecacheScriptSound( TASER_BIRTHDAY_SOUND );
}

void CWeaponTaser::PrimaryAttack( void )
{
	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], Primary_Mode ) )
		return;

	m_fFireTime = gpGlobals->curtime;

	if ( UTIL_IsCSSOBirthday() )
	{
		//CPASAttenuationFilter filter( this, params.soundlevel );
		//EmitSound( filter, entindex(), TASER_BIRTHDAY_SOUND, &GetLocalOrigin(), 0.0f );

		CPASAttenuationFilter filter( this );
		filter.UsePredictionRules();
		EmitSound( filter, entindex(), TASER_BIRTHDAY_SOUND );
	}
}

#if defined( GAME_DLL )
bool CWeaponTaser::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( HasAmmo() == false && mp_taser_recharge_time.GetFloat() < 0.0f )
	{
		// just drop it if it's out of ammo and we're trying to switch away
		GetPlayerOwner()->CSWeaponDrop( this );
	}

	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponTaser::ItemPostFrame()
{
	const float kTaserDropDelay = 0.5f;
	BaseClass::ItemPostFrame();

	if ( HasAmmo() == false && gpGlobals->curtime >= m_fFireTime + kTaserDropDelay && mp_taser_recharge_time.GetFloat() < 0.0f )
	{
		// PiMoN: fix for zeus staying as active weapon when you shoot it in DM
		// because it doesnt get switched but it does get removed
		if ( this == GetPlayerOwner()->GetActiveWeapon() )
		{
			GetPlayerOwner()->SwitchToNextBestWeapon( this );
		}
	}
}

void CWeaponTaser::TaserThink()
{
	if ( mp_taser_recharge_time.GetFloat() < 0.0f || m_iClip1 > 0 )
	{
		SetContextThink( &CWeaponTaser::TaserThink, gpGlobals->curtime + 1.0f, TASER_THINK_CONTEXT );
		return;
	}
	if ( gpGlobals->curtime >= m_fFireTime + mp_taser_recharge_time.GetFloat() )
		m_iClip1 = GetDefaultClip1();

	SetContextThink( &CWeaponTaser::TaserThink, gpGlobals->curtime + 1.0f, TASER_THINK_CONTEXT );
}
#else
float CWeaponTaser::GetTaserRechargePercentage()
{
	if ( m_iClip1 > 0 )
		return 1.0f;

	if ( mp_taser_recharge_time.GetFloat() < 0.0f )
		return 0.0f;

	float flTimeDelta = gpGlobals->curtime - m_fFireTime;
	//return clamp( flTimeDelta / mp_taser_recharge_time.GetFloat(), 0.0f, 1.0f);

	// PiMoN: this is some really cringe decompilation code that breaks my brain!
	const float kMaxRecharge = 0.97f;
	if ( mp_taser_recharge_time.GetFloat() == 0.0f )
	{
		if ( flTimeDelta < 0.0f )
			return 0.0f;
		else
			return kMaxRecharge;
	}
	else
	{
		float flPercentage = flTimeDelta / mp_taser_recharge_time.GetFloat();
		if ( flPercentage < 0.0f )
			return 0.0f;

		return MIN( flPercentage, 1.0f ) * kMaxRecharge;
	}
}
#endif