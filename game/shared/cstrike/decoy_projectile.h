//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef DECOY_PROJECTILE_H
#define DECOY_PROJECTILE_H
#ifdef _WIN32
#pragma once
#endif


#include "basecsgrenade_projectile.h"
#include "cs_weapon_parse.h"


#if defined( CLIENT_DLL )

class C_DecoyProjectile : public C_BaseCSGrenadeProjectile
{
public:
	DECLARE_CLASS( C_DecoyProjectile, C_BaseCSGrenadeProjectile );
	DECLARE_NETWORKCLASS();

	virtual void Simulate();

	virtual void OnNewParticleEffect( const char *pszParticleName, CNewParticleEffect *pNewParticleEffect );
	virtual void OnParticleEffectDeleted( CNewParticleEffect *pParticleEffect );

private:
	CUtlReference<CNewParticleEffect> m_decoyParticleEffect;

};

#else // GAME_DLL

struct DecoyWeaponProfile;

class CDecoyProjectile : public CBaseCSGrenadeProjectile
{
public:
	DECLARE_CLASS( CDecoyProjectile, CBaseCSGrenadeProjectile );
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

// Overrides.
public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void Detonate( void );
	virtual void BounceSound( void );

// Grenade stuff.
	static CDecoyProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner );	

private:
	void Think_Detonate( void );
	void GunfireThink( void );
	void SetTimer( float timer );

	int m_shotsRemaining;
	float m_fExpireTime;
	DecoyWeaponProfile*	m_pProfile;
	CSWeaponID m_decoyWeaponId;
	WeaponSound_t m_decoyWeaponSoundType;
};

#endif // GAME_DLL

#endif // DECOY_PROJECTILE_H
