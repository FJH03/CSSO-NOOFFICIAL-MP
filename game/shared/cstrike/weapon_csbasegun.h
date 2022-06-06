//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_CSBASE_GUN_H
#define WEAPON_CSBASE_GUN_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_csbase.h"


// This is the base class for pistols and rifles.
#if defined( CLIENT_DLL )

	#define CWeaponCSBaseGun C_WeaponCSBaseGun

#else
#endif


class CWeaponCSBaseGun : public CWeaponCSBase
{
public:
	
	DECLARE_CLASS( CWeaponCSBaseGun, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponCSBaseGun();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void Spawn();
	virtual bool Deploy();
	virtual bool Reload();
	virtual void WeaponIdle();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void Drop( const Vector &vecVelocity );
	virtual bool SendWeaponAnim( int iActivity );

	// Derived classes call this to fire a bullet.
	bool CSBaseGunFire( float flCycleTime, CSWeaponMode weaponMode );

	void BurstFireRemaining( void );

	// Usually plays the shot sound. Guns with silencers can play different sounds.
	virtual void DoFireEffects();
	virtual void ItemPostFrame();
	virtual void ItemBusyFrame( void );

	virtual int GetCSZoomLevel() { return m_zoomLevel; }

	CNetworkVar( int, m_zoomLevel );

	virtual bool HasZoom( void );
	virtual bool IsZoomed( void ) const;

	virtual bool WeaponHasBurst( void ) const { return GetCSWpnData().m_bHasBurst; }
	virtual bool IsInBurstMode() const;

	virtual bool IsFullAuto() const;

	virtual bool IsRevolver() const { return GetCSWpnData().m_bIsRevolver; }
	virtual bool DoesUnzoomAfterShot( void ) const { return GetCSWpnData().m_bDoesUnzoomAfterShot; }

	CNetworkVar( int, m_iBurstShotsRemaining );
	float	m_fNextBurstShot;			// time to shoot the next bullet in burst fire mode

	virtual Activity GetDeployActivity( void );

private:

	CWeaponCSBaseGun( const CWeaponCSBaseGun & );
};


#endif // WEAPON_CSBASE_GUN_H