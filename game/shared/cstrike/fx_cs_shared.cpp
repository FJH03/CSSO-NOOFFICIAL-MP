//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "fx_cs_shared.h"
#include "weapon_csbase.h"
#include "rumble_shared.h"

#ifndef CLIENT_DLL
	#include "ilagcompensationmanager.h"
#endif

ConVar weapon_accuracy_logging( "weapon_accuracy_logging", "0", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY | FCVAR_ARCHIVE );
ConVar weapon_near_empty_sound( "weapon_near_empty_sound", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar weapon_debug_max_inaccuracy( "weapon_debug_max_inaccuracy", "0", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT, "Force all shots to have maximum inaccuracy" );
ConVar weapon_debug_inaccuracy_only_up( "weapon_debug_inaccuracy_only_up", "0", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT, "Force weapon inaccuracy to be in exactly the up direction" );
ConVar snd_max_pitch_shift_inaccuracy("snd_max_pitch_shift_inaccuracy", "0.08", 0);

#ifdef CLIENT_DLL

#include "fx_impact.h"
#include "c_rumble.h"
#include "inputsystem/iinputsystem.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

	// this is a cheap ripoff from CBaseCombatWeapon::WeaponSound():
	void FX_WeaponSound(
		int iPlayerIndex,
		WeaponSound_t sound_type,
		const Vector &vOrigin,
		const CCSWeaponInfo *pWeaponInfo,
		float flSoundTime,
		int nPitch )
	{
		// If we have some sounds from the weapon classname.txt file, play a random one of them
		const char *shootsound = pWeaponInfo->aShootSounds[ sound_type ];

		if ( !shootsound || !shootsound[0] )
			return;

		CBroadcastRecipientFilter filter; // this is client side only
		if ( !te->CanPredict() )
			return;

		EmitSound_t params;
		params.m_pSoundName = shootsound;
		params.m_flSoundTime = flSoundTime;
		params.m_pOrigin = &vOrigin;
		params.m_pflSoundDuration = nullptr;
		params.m_bWarnOnDirectWaveReference = true;
		params.m_nPitch = nPitch;

		if (nPitch != PITCH_NORM)
		{
			params.m_nFlags = params.m_nFlags;
		}
				
		CBaseEntity::EmitSound( filter, iPlayerIndex, params ); 
	}

	class CGroupedSound
	{
	public:
		string_t m_SoundName;
		Vector m_vPos;
	};

	CUtlVector<CGroupedSound> g_GroupedSounds;

	
	// Called by the ImpactSound function.
	void ShotgunImpactSoundGroup( const char *pSoundName, const Vector &vEndPos )
	{
		int i;
		// Don't play the sound if it's too close to another impact sound.
		for ( i=0; i < g_GroupedSounds.Count(); i++ )
		{
			CGroupedSound *pSound = &g_GroupedSounds[i];

			if ( vEndPos.DistToSqr( pSound->m_vPos ) < 300*300 )
			{
				if ( Q_stricmp( pSound->m_SoundName, pSoundName ) == 0 )
					return;
			}
		}

		// Ok, play the sound and add it to the list.
		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, NULL, pSoundName, &vEndPos );

		i = g_GroupedSounds.AddToTail();
		g_GroupedSounds[i].m_SoundName = pSoundName;
		g_GroupedSounds[i].m_vPos = vEndPos;
	}


	void StartGroupingSounds()
	{
		Assert( g_GroupedSounds.Count() == 0 );
		SetImpactSoundRoute( ShotgunImpactSoundGroup );
	}


	void EndGroupingSounds()
	{
		g_GroupedSounds.Purge();
		SetImpactSoundRoute( NULL );
	}

#else

	#include "te_shotgun_shot.h"

	// Server doesn't play sounds anyway.
	void StartGroupingSounds() {}
	void EndGroupingSounds() {}
	void FX_WeaponSound ( int iPlayerIndex,
		WeaponSound_t sound_type,
		const Vector &vOrigin,
		const CCSWeaponInfo *pWeaponInfo, float flSoundTime, int nPitch ) {};

#endif

ConVar debug_aim_angle("debug_aim_angle", "0", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);
	
// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets( 
	int	iPlayerIndex,
	const Vector &vOrigin,
	const QAngle &vAngles,
	CSWeaponID iWeaponID,
	int	iMode,
	int iSeed,
	float fInaccuracy,
	float fSpread,
	float flSoundTime,
	WeaponSound_t sound_type,
	float flRecoilIndex
	)
{
	bool bDoEffects = true;

	if ( fInaccuracy > 1.0f )
		fInaccuracy = 1.0f;

#ifdef CLIENT_DLL
	C_CSPlayer *pPlayer = ToCSPlayer( ClientEntityList().GetBaseEntity( iPlayerIndex ) );
#else
	CCSPlayer *pPlayer = ToCSPlayer( UTIL_PlayerByIndex( iPlayerIndex) );
#endif

	if ( !pPlayer || iPlayerIndex < 0 )
	{
		// probably an env_gunfire
		const CCSWeaponInfo* pWeaponInfo = GetWeaponInfo( iWeaponID );
		FX_WeaponSound( iPlayerIndex, sound_type, vOrigin, pWeaponInfo, flSoundTime, PITCH_NORM );
#ifndef CLIENT_DLL
		// if this is server code, send the effect over to client as temp entity
		// Dispatch one message for all the bullet impacts and sounds.
		TE_FireBullets( 
			-1,
			vOrigin, 
			vAngles, 
			iWeaponID,
			iMode,
			iSeed,
			fInaccuracy,
			fSpread,
			sound_type,
			flRecoilIndex
			);
#endif
		return;
	}

#ifdef CLIENT_DLL
	CWeaponCSBase* pClientWeapon = pPlayer ? pPlayer->GetActiveCSWeapon() : NULL;
	if ( pClientWeapon )
	{
		if ( gpGlobals->curtime - pClientWeapon->m_flLastClientFireBulletTime > 0.02f ) // this should be enough even for the negev, at ~1000 rof
		{
			pClientWeapon->m_flLastClientFireBulletTime = gpGlobals->curtime;
		}
		else
		{
			return; // we already traced this shot on the client!
		}
	}
#endif

	QAngle adjustedAngles = vAngles;

	if ( pPlayer && debug_aim_angle.GetBool() )
	{
		QAngle old = pPlayer->EyeAngles() + pPlayer->GetAimPunchAngle();
#ifdef CLIENT_DLL
		DevMsg("Client ");
#else
		DevMsg("Server ");
#endif
		DevMsg("old: %f %f new: %f %f\n",
			old[YAW], old[PITCH],
			vAngles[YAW], vAngles[PITCH]
			);
		if ( debug_aim_angle.GetInt() == 2 )
		{
			adjustedAngles = old;
		}
	}

	const char* weaponAlias = WeaponIDToAlias( iWeaponID );

	if ( !weaponAlias )
	{
		DevMsg( "FX_FireBullets: weapon alias for ID %i not found\n", iWeaponID );
		return;
	}

#if !defined(CLIENT_DLL)
	if ( weapon_accuracy_logging.GetBool() )
	{
		char szFlags[256];

		V_strcpy(szFlags, " ");

// #if defined(CLIENT_DLL)
// 		V_strcat(szFlags, "CLIENT ", sizeof(szFlags));
// #else
// 		V_strcat(szFlags, "SERVER ", sizeof(szFlags));
// #endif
// 
		if ( pPlayer->GetMoveType() == MOVETYPE_LADDER )
			V_strcat(szFlags, "LADDER ", sizeof(szFlags));

		if ( FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
			V_strcat(szFlags, "GROUND ", sizeof(szFlags));

		if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING) )
			V_strcat(szFlags, "DUCKING ", sizeof(szFlags));

		float fVelocity = pPlayer->GetAbsVelocity().Length2D();

		Msg("FireBullets @ %10f [ %s ]: inaccuracy=%f  spread=%f  max dispersion=%f  mode=%2i  vel=%10f  seed=%3i  %s\n", 
			gpGlobals->curtime, weaponAlias, fInaccuracy, fSpread, fInaccuracy + fSpread, iMode, fVelocity, iSeed, szFlags);
	}
#endif

	char wpnName[128];
	Q_snprintf( wpnName, sizeof( wpnName ), "weapon_%s", weaponAlias );
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( wpnName );

	if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
	{
		DevMsg( "FX_FireBullets: LookupWeaponInfoSlot failed for weapon %s\n", wpnName );
		return;
	}

	CCSWeaponInfo *pWeaponInfo = static_cast< CCSWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );
	if ( !pWeaponInfo )
	{
		DevMsg( "FX_FireBullets: GetFileWeaponInfoFromHandle failed for weapon %s\n", wpnName );
		return;
	}

	// Do the firing animation event.
#ifndef CLIENT_DLL
	if ( pPlayer && !pPlayer->IsDormant() )
	{
		if ( iMode == Primary_Mode )
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_PRIMARY );
		else
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_SECONDARY );
	}
#endif // CLIENT_DLL


#ifdef CLIENT_DLL
	if ( pPlayer && pPlayer->m_bUseNewAnimstate )
	{
		pPlayer->ProcessMuzzleFlashEvent();
	}
#endif


#ifndef CLIENT_DLL
	// if this is server code, send the effect over to client as temp entity
	// Dispatch one message for all the bullet impacts and sounds.
	TE_FireBullets( 
		iPlayerIndex,
		vOrigin, 
		vAngles, 
		iWeaponID,
		iMode,
		iSeed,
		fInaccuracy,
		fSpread,
		sound_type,
		flRecoilIndex
		);


	// Let the player remember the usercmd he fired a weapon on. Assists in making decisions about lag compensation.
	if ( pPlayer )
		pPlayer->NoteWeaponFired();

	bDoEffects = false; // no effects on server
#endif

	iSeed++;

	CWeaponCSBase* pWeapon = pPlayer ? pPlayer->GetActiveCSWeapon() : NULL;

	int		iDamage = pWeaponInfo->m_iDamage;
	float	flRange = pWeaponInfo->m_flRange;
	float	flPenetration = pWeaponInfo->m_flPenetration;
	float	flRangeModifier = pWeaponInfo->m_flRangeModifier;
	int		iAmmoType = pWeaponInfo->iAmmoType;

	if ( bDoEffects)
	{
		static const float MaxPitchShiftInaccuracy = 0.05f;
		float flPitchShift = pWeaponInfo->m_fInaccuracyPitchShift * (fInaccuracy < MaxPitchShiftInaccuracy ? fInaccuracy : MaxPitchShiftInaccuracy);

		if ( sound_type == SINGLE && pWeaponInfo->m_fInaccuracyAltSoundThreshold > 0.0f && fInaccuracy < pWeaponInfo->m_fInaccuracyAltSoundThreshold )
		{
			sound_type = SINGLE_ACCURATE;
			flPitchShift = 0.0f;
		}

		FX_WeaponSound( iPlayerIndex, sound_type, vOrigin, pWeaponInfo, flSoundTime, PITCH_NORM + int(flPitchShift) );

		// If the gun's nearly empty, also play a subtle "nearly-empty" sound, since the weapon 
		// is lighter and acoustically different when weighed down by fewer bullets.
		// But really it's so you get a fun low ammo warning from an audio cue.
		if ( weapon_near_empty_sound.GetBool() &&
			 pWeapon && pWeapon->GetMaxClip1() > 1 && // not a single-shot weapon
			 (((float)pWeapon->m_iClip1) / ((float)pWeapon->GetMaxClip1()) <= 0.2) ) // 20% or fewer bullets remaining
		{
			FX_WeaponSound( iPlayerIndex, NEARLYEMPTY, vOrigin, pWeaponInfo, flSoundTime, PITCH_NORM );
		}
	}

	// Fire bullets, calculate impacts & effects

	if ( !pPlayer )
		return;
	
	StartGroupingSounds();

#ifdef GAME_DLL
	pPlayer->StartNewBulletGroup();
#endif

#if !defined (CLIENT_DLL)
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, LAG_COMPENSATE_HITBOXES_ALONG_RAY, vOrigin, vAngles, flRange );
#endif

	bool bForceMaxInaccuracy = weapon_debug_max_inaccuracy.GetBool();
	bool bForceInaccuracyDirection = weapon_debug_inaccuracy_only_up.GetBool();

	RandomSeed( iSeed );	// init random system with this seed

	// Accuracy curve density adjustment FOR R8 REVOLVER SECONDARY FIRE, NEGEV WILD BEAST
	float flRadiusCurveDensity = RandomFloat();
	if ( iWeaponID == WEAPON_REVOLVER && iMode == Secondary_Mode ) /*R8 REVOLVER SECONDARY FIRE*/
	{
		flRadiusCurveDensity = 1.0f - flRadiusCurveDensity*flRadiusCurveDensity;
	}
	if ( iWeaponID == WEAPON_NEGEV && flRecoilIndex < 3 ) /*NEGEV WILD BEAST*/
	{
		for ( int j = 3; j > flRecoilIndex; -- j )
		{
			flRadiusCurveDensity *= flRadiusCurveDensity;
		}
		flRadiusCurveDensity = 1.0f - flRadiusCurveDensity;
	}

	if ( bForceMaxInaccuracy )
		flRadiusCurveDensity = 1.0f;

	// Get accuracy displacement
	float fTheta0 = RandomFloat(0.0f, 2.0f * M_PI);
	
	if ( bForceInaccuracyDirection )
		fTheta0 = M_PI * 0.5f;

	float fRadius0 = flRadiusCurveDensity * fInaccuracy;
	float x0 = fRadius0 * cosf(fTheta0);
	float y0 = fRadius0 * sinf(fTheta0);

	const int kMaxBullets = 16;
	float x1[kMaxBullets], y1[kMaxBullets];
	Assert(pWeaponInfo->m_iBullets <= kMaxBullets);

	// the RNG can be desynchronized by FireBullet(), so pre-generate all spread offsets
	for ( int iBullet=0; iBullet < pWeaponInfo->m_iBullets; iBullet++ )
	{
		// Spread curve density adjustment for R8 REVOLVER SECONDARY FIRE, NEGEV WILD BEAST
		float flSpreadCurveDensity = RandomFloat();
		if ( iWeaponID == WEAPON_REVOLVER && iMode == Secondary_Mode )
		{
			flSpreadCurveDensity = 1.0f - flSpreadCurveDensity*flSpreadCurveDensity;
		}
		if ( iWeaponID == WEAPON_NEGEV && flRecoilIndex < 3 ) /*NEGEV WILD BEAST*/
		{
			for ( int j = 3; j > flRecoilIndex; --j )
			{
				flSpreadCurveDensity *= flSpreadCurveDensity;
			}
			flSpreadCurveDensity = 1.0f - flSpreadCurveDensity;
		}

		if ( bForceMaxInaccuracy )
			flSpreadCurveDensity = 1.0f;

		float fTheta1 = RandomFloat(0.0f, 2.0f * M_PI);
		if ( bForceInaccuracyDirection )
			fTheta1 = M_PI * 0.5f;

		float fRadius1 = flSpreadCurveDensity * fSpread;
		x1[iBullet] = fRadius1 * cosf(fTheta1);
		y1[iBullet] = fRadius1 * sinf(fTheta1);
	}

	for ( int iBullet=0; iBullet < pWeaponInfo->m_iBullets; iBullet++ )
	{
		if ( !pPlayer )
			break;

		int nPenetrationCount = 4;

		pPlayer->FireBullet(
			vOrigin,
			adjustedAngles,
			flRange,
			flPenetration,
			nPenetrationCount,
			iAmmoType,
			iDamage,
			flRangeModifier,
			pPlayer,
			bDoEffects,
			x0 + x1[iBullet], y0 + y1[iBullet]
			);
	}

#if !defined (CLIENT_DLL)
	lagcompensation->FinishLagCompensation( pPlayer );
#endif

	EndGroupingSounds();
}

// This runs on both the client and the server.
// On the server, it dispatches a TE_PlantBomb to visible clients.
// On the client, it plays the planting animation.
void FX_PlantBomb( int iPlayerIndex, const Vector &vOrigin, PlantBombOption_t option )
{
#ifdef CLIENT_DLL
	C_CSPlayer *pPlayer = ToCSPlayer( ClientEntityList().GetBaseEntity( iPlayerIndex ) );
#else
	CCSPlayer *pPlayer = ToCSPlayer( UTIL_PlayerByIndex( iPlayerIndex) );
#endif

	// Do the firing animation event.
	if ( pPlayer && !pPlayer->IsDormant() )
	{
		switch ( option )
		{
		case PLANTBOMB_PLANT:
			{
				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_PRIMARY );
			}
			break;

		case PLANTBOMB_ABORT:
			{
				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_CLEAR_FIRING );
			}
			break;
		}
	}

#ifndef CLIENT_DLL
	// if this is server code, send the effect over to client as temp entity
	// Dispatch one message for all the bullet impacts and sounds.
	TE_PlantBomb( iPlayerIndex, vOrigin, option );
#endif
}

