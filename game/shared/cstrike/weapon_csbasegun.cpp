//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "fx_cs_shared.h"

#ifdef CLIENT_DLL
	#include "c_cs_player.h"
#else
	#include "cs_player.h"
#endif


IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCSBaseGun, DT_WeaponCSBaseGun )

BEGIN_NETWORK_TABLE( CWeaponCSBaseGun, DT_WeaponCSBaseGun )
#if defined( GAME_DLL )
SendPropInt( SENDINFO( m_zoomLevel ), 2, SPROP_UNSIGNED ),
SendPropInt( SENDINFO( m_iBurstShotsRemaining ) ),
#else
RecvPropInt( RECVINFO( m_zoomLevel ) ),
RecvPropInt( RECVINFO( m_iBurstShotsRemaining ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponCSBaseGun )
DEFINE_PRED_FIELD( m_zoomLevel, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_iBurstShotsRemaining, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_fNextBurstShot, FIELD_FLOAT, 0 ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_csbase_gun, CWeaponCSBaseGun );



CWeaponCSBaseGun::CWeaponCSBaseGun()
{
	m_zoomLevel = 0;
}

void CWeaponCSBaseGun::Spawn()
{
	BaseClass::Spawn();

	m_bBurstMode = false;
	m_iBurstShotsRemaining = 0;
	m_fNextBurstShot = 0.0f;
	ResetPostponeFireReadyTime();
}


bool CWeaponCSBaseGun::Deploy()
{
	// don't allow weapon switching to shortcut cycle time (quickswitch exploit)
	float fOldNextPrimaryAttack = m_flNextPrimaryAttack;
	float fOldNextSecondaryAttack = m_flNextSecondaryAttack;

	m_flDoneSwitchingSilencer = 0.0f;
	m_iBurstShotsRemaining = 0;
	m_fNextBurstShot = 0.0f;

	if ( !BaseClass::Deploy() )
		return false;

	if ( HasZoom() )
	{
		m_zoomLevel = 0;
		m_weaponMode = Primary_Mode;
	}

	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	pPlayer->m_iShotsFired = 0;

	if ( IsRevolver() )
	{
		m_weaponMode = Secondary_Mode;
	}

	m_flNextPrimaryAttack	= Max( m_flNextPrimaryAttack.Get(), fOldNextPrimaryAttack );
	m_flNextSecondaryAttack	= Max( m_flNextSecondaryAttack.Get(), fOldNextSecondaryAttack );
	return true;
}

Activity CWeaponCSBaseGun::GetDeployActivity( void )
{
	if ( HasSilencer() == Silencer_Removable && IsSilenced() )
	{
		return ACT_VM_DRAW_SILENCED;
	}
	else
	{
		return BaseClass::GetDeployActivity();
	}
}

void CWeaponCSBaseGun::Drop( const Vector &vecVelocity )
{
	// re-deploying the weapon is punishment enough for canceling a silencer attach/detach before completion
	if ( (GetActivity() == ACT_VM_ATTACH_SILENCER && m_bSilencerOn == false) ||
		 (GetActivity() == ACT_VM_DETACH_SILENCER && m_bSilencerOn == true ) )
	{
		m_flDoneSwitchingSilencer = gpGlobals->curtime;
		m_flNextSecondaryAttack = gpGlobals->curtime;
		m_flNextPrimaryAttack = gpGlobals->curtime;
	}

	//make sure the world-model silencer bodygroup is correct, we might have hidden/unhidden it prematurely to make the 3rd-person animation look correct
	else if ( (GetActivity() == ACT_VM_ATTACH_SILENCER) || (GetActivity() == ACT_VM_DETACH_SILENCER) )
	{
		int iBodyGroup = FindBodygroupByName( "silencer" );
		if ( iBodyGroup != -1 )
			SetBodygroup( iBodyGroup, m_bSilencerOn ? 0 : 1 );
	}

	BaseClass::Drop( vecVelocity );
}

bool CWeaponCSBaseGun::SendWeaponAnim( int iActivity )
{
	// PiMoN: some addons want to use ACT_VM_DRYFIRE just as it works on Deagle in CS:GO so I'm backporting this "feature"
	if ( !IsRevolver() && iActivity == ACT_VM_PRIMARYATTACK && m_iClip1 == 1 )
	{
		if ( IsSilenced() && LookupActivity( "ACT_VM_DRYFIRE_SILENCED" ) != ACT_INVALID )
			iActivity = ACT_VM_DRYFIRE_SILENCED;
		if ( !IsSilenced() && LookupActivity( "ACT_VM_DRYFIRE" ) != ACT_INVALID )
			iActivity = ACT_VM_DRYFIRE;
	}

	return BaseClass::SendWeaponAnim( iActivity );
}

void CWeaponCSBaseGun::ItemBusyFrame()
{
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer )
		return;

	// if we're scoped during a reload, pull us out of the scope for the duration (and set resumezoom so we'll re-zoom when reloading is done)
	if ( HasZoom() && (IsZoomed() || pPlayer->m_bIsScoped) && m_bInReload )
	{
		//m_zoomLevel = 0; //don't affect zoom level, so it'll restore when reloading is done
		pPlayer->m_bIsScoped = false;
		pPlayer->m_bResumeZoom = true;
		pPlayer->SetFOV( pPlayer, GetZoomFOV( 0 ), GetZoomTime( 0 ) );
		m_weaponMode = Primary_Mode;
	}

	BaseClass::ItemBusyFrame();
}

void CWeaponCSBaseGun::ItemPostFrame()
{
	CCSPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer )
		return;

	//GOOSEMAN : Return zoom level back to previous zoom level before we fired a shot. This is used only for the AWP.
	// And Scout.
	if ( (m_flNextPrimaryAttack <= gpGlobals->curtime) && (pPlayer->m_bResumeZoom == TRUE)
		&& m_zoomLevel > 0 ) // only need to re-zoom the zoom when there's a zoom to re-zoom to. who knew?
	{		
		if ( m_iClip1 != 0 || ( GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD ) )
		{
			m_weaponMode = Secondary_Mode;
			// the zoom amount is taking care of below
			pPlayer->SetFOV( pPlayer, GetZoomFOV( m_zoomLevel ), 0.1f );
			pPlayer->m_bIsScoped = true;
		}

		pPlayer->m_bResumeZoom = false;
	}

	if ( WeaponHasBurst() )
	{
		if ( m_iBurstShotsRemaining > 0 && gpGlobals->curtime >= m_fNextBurstShot )
		{
			BurstFireRemaining();
		}
	}

	BaseClass::ItemPostFrame();
}


void CWeaponCSBaseGun::PrimaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	float flCycleTime = GetCSWpnData().m_flCycleTime[m_weaponMode];

	// change a few things if we're in burst mode
	if ( IsInBurstMode() )
	{
		flCycleTime = GetCSWpnData().m_flCycleTimeInBurst;

		m_iBurstShotsRemaining = 2;

		m_fNextBurstShot = gpGlobals->curtime + GetCSWpnData().m_flTimeBetweenBurstShots;
	}
																	
	if ( !CSBaseGunFire( flCycleTime, m_weaponMode ) )								// <--	'PEW PEW' HAPPENS HERE
		return;

	if ( IsSilenced() )
		SendWeaponAnim( ACT_VM_PRIMARYATTACK_SILENCED );

	// Does this gun unzoom after a shot, as in a bolt action rifle?
	if ( IsZoomed() && ( DoesUnzoomAfterShot() ) )
	{
		pPlayer->m_bIsScoped = false;
		pPlayer->m_bResumeZoom = true;
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), 0.05f );
		m_weaponMode = Primary_Mode;
	}
}

void CWeaponCSBaseGun::SecondaryAttack()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer == NULL )
	{
		Assert( pPlayer != NULL );
		return;
	}

	if ( HasZoom() )
	{
		if ( ++m_zoomLevel > GetCSWpnData().m_iZoomLevels )
			m_zoomLevel = 0;

		bool bIsSniperRifle = GetWeaponType() == WEAPONTYPE_SNIPER_RIFLE;

		if ( IsZoomed() )
		{
			m_weaponMode = Secondary_Mode;

			//float flFOV = GetFOVForAccuracy();

			if ( bIsSniperRifle )
				pPlayer->SetFOV( pPlayer, GetZoomFOV( m_zoomLevel ), GetZoomTime( m_zoomLevel ) );

			pPlayer->m_bIsScoped = true;

			if ( pPlayer->GetActiveCSWeapon() )
			{
				CIronSightController *pIronSightController = pPlayer->GetActiveCSWeapon()->GetIronSightController();
				if (pIronSightController)
				{
					pPlayer->GetActiveCSWeapon()->UpdateIronSightController();
					pPlayer->SetFOV(pPlayer, pIronSightController->GetIronSightIdealFOV(), pIronSightController->GetIronSightPullUpDuration());
					pIronSightController->SetState( IronSight_should_approach_sighted );

					//stop looking at weapon when going into ironsights
#ifndef CLIENT_DLL
					pPlayer->StopLookingAtWeapon();

					//force idle animation
					CBaseViewModel *pViewModel = pPlayer->GetViewModel();
					if (pViewModel)
					{
						int nSequence = pViewModel->LookupSequence("idle");
						if (nSequence != ACTIVITY_NOT_AVAILABLE)
						{
							pViewModel->ForceCycle(0);
							pViewModel->ResetSequence(nSequence);
						}
					}
#endif
				}
			}
		}
		else
		{
			m_weaponMode = Primary_Mode;

			if ( bIsSniperRifle )
			{
				pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), GetZoomTime( 0 ) );
			}

			pPlayer->m_bIsScoped = false;

			if ( pPlayer->GetActiveCSWeapon() )
			{
				CIronSightController *pIronSightController = pPlayer->GetActiveCSWeapon()->GetIronSightController();
				if (pIronSightController)
				{
					pPlayer->GetActiveCSWeapon()->UpdateIronSightController();
					pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), pIronSightController->GetIronSightPutDownDuration() );
					pIronSightController->SetState(IronSight_should_approach_unsighted);
					SendWeaponAnim(ACT_VM_FIDGET);
				}
			}
		}

#ifdef CLIENT_DLL
		/*
		if ( GetPlayerOwner() && ( bIsSniperRifle && IsZoomed() && m_zoomLevel == 1 ) )
		{
			ScreenFade_t		fade;
			fade.duration = ( unsigned short )( ( float )( 1 << SCREENFADE_FRACBITS ) * 0.22 );
			fade.holdTime = ( unsigned short )( ( float )( 1 << SCREENFADE_FRACBITS ) * 0 );

			fade.fadeFlags = 0;
			fade.fadeFlags |= FFADE_IN;

			fade.r = 0;
			fade.g = 0;
			fade.b = 0;
			fade.a = 255;

			clientdll->View_Fade( &fade );
		}
		*/
#endif

#ifndef CLIENT_DLL

		
		// If this isn't guarded, the sound will be emitted twice, once by the server and once by the client.
		// Let the server play it since if only the client plays it, it's liable to get played twice cause of
		// a prediction error. joy.

		// [tj] Playing this from the player so that we don't try to play the sound outside the level.
		if ( GetPlayerOwner() )
		{
			if ( IsZoomed() )
			{
				const char *pszZoomSound = GetCSWpnData().m_szZoomINSound;
				if ( pszZoomSound && pszZoomSound[0] )
				{
					GetPlayerOwner()->EmitSound( pszZoomSound );
				}

				//if ( !bIsSniperRifle )
				//{
				//	color32 clr = {0, 0, 0, 200};
				//	float flZoomTime = weaponInfo.m_fZoomTime[m_zoomLevel];
				//	float flBlackTime = MAX( flZoomTime/15, 0.02 );
				//	UTIL_ScreenFade( pPlayer, clr, flBlackTime, (flZoomTime - (flZoomTime/5)) - flBlackTime, FFADE_IN );
				//}
			}
			else
			{
				const char *pszZoomSound = GetCSWpnData().m_szZoomOUTSound;
				if ( pszZoomSound && pszZoomSound[0] )
				{
					GetPlayerOwner()->EmitSound( pszZoomSound );
				}

				//if ( !bIsSniperRifle )
				//{
				//	color32 clr = {0, 0, 0, 175};
				//	float flZoomTime = weaponInfo.m_fZoomTime[0];
				//	float flBlackTime = MAX( flZoomTime/15, 0.02 );
				//	UTIL_ScreenFade( pPlayer, clr, flBlackTime, flZoomTime - flBlackTime, FFADE_OUT );
				//}
			}

			if ( bIsSniperRifle )
			{
				// let the bots hear the sniper rifle zoom
				IGameEvent * event = gameeventmanager->CreateEvent( "weapon_zoom" );
				if ( event )
				{
					event->SetInt( "userid", pPlayer->GetUserID() );
					gameeventmanager->FireEvent( event );
				}
			}
		}

#endif
	}
#ifndef CLIENT_DLL
	else if ( WeaponHasBurst() )
	{
		if ( IsInBurstMode() )
		{
			if ( GetCSWpnData().m_bFullAuto ) // have to check for weapon script value since IsFullAuto() checks for burst mode
				pPlayer->HintMessage( "#Cstrike_TitlesTXT_Switch_To_FullAuto", false );
			else
				pPlayer->HintMessage( "#Cstrike_TitlesTXT_Switch_To_SemiAuto", false );
			m_bBurstMode = false;
			m_weaponMode = Primary_Mode;
		}
		else
		{
			pPlayer->HintMessage( "#Cstrike_TitlesTXT_Switch_To_BurstFire", false );
			m_bBurstMode = true;
			m_weaponMode = Secondary_Mode;
		}

		pPlayer->EmitSound( "Weapon.AutoSemiAutoSwitch" );
	}
#endif
	else if ( HasSilencer() == Silencer_Removable && m_flDoneSwitchingSilencer <= gpGlobals->curtime )
	{
		if ( m_bSilencerOn )
		{
			SendWeaponAnim( ACT_VM_DETACH_SILENCER );

#ifndef CLIENT_DLL
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_SILENCER_DETACH );
#endif
		}
		else
		{
			SendWeaponAnim( ACT_VM_ATTACH_SILENCER );

#ifndef CLIENT_DLL
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_SILENCER_ATTACH );
#endif
		}

		float nextAttackTime = gpGlobals->curtime + SequenceDuration();

		m_flDoneSwitchingSilencer = nextAttackTime;
		m_flNextSecondaryAttack = nextAttackTime;
		m_flNextPrimaryAttack = nextAttackTime;
		SetWeaponIdleTime( nextAttackTime );

	}
	else if ( IsRevolver() && m_flNextSecondaryAttack < gpGlobals->curtime )
	{
		float flCycletimeAlt = GetCSWpnData().m_flCycleTime[Secondary_Mode];
		m_weaponMode = Secondary_Mode;
		UpdateAccuracyPenalty();
#ifndef CLIENT_DLL
		// Logic for weapon_fire event mimics weapon_csbase.cpp CWeaponCSBase::ItemPostFrame() primary fire implementation
		IGameEvent * event = gameeventmanager->CreateEvent( (HasAmmo()) ? "weapon_fire" : "weapon_fire_on_empty" );
		if ( event )
		{
			const char *weaponName = STRING( m_iClassname );
			if ( IsWeaponClassname( weaponName ) )
			{
				weaponName += 7;
			}

			event->SetInt( "userid", pPlayer->GetUserID() );
			event->SetString( "weapon", weaponName );
			event->SetBool( "silenced", IsSilenced() );
			gameeventmanager->FireEvent( event );
		}
#endif
		CSBaseGunFire( flCycletimeAlt, Secondary_Mode );								// <--	'PEW PEW' HAPPENS HERE
		m_flNextSecondaryAttack = gpGlobals->curtime + flCycletimeAlt;
		return;
	}

	else
	{
		BaseClass::SecondaryAttack();
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3f;
}

void CWeaponCSBaseGun::BurstFireRemaining()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer || m_iClip1 <= 0 )
	{
		m_iClip1 = 0;
		m_iBurstShotsRemaining = 0;
		m_fNextBurstShot = 0.0f;
		return;
	}
	
	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->GetFinalAimAngle(),
		GetCSWeaponID(),
		m_weaponMode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetInaccuracy(),
		GetSpread(), 
		m_fNextBurstShot,
		(HasSilencer() > 0 && IsSilenced()) ? SPECIAL1 : SINGLE,
		m_flRecoilIndex );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->DoMuzzleFlash();
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	--m_iBurstShotsRemaining;

	if ( m_iBurstShotsRemaining > 0 )
	{
		m_fNextBurstShot = gpGlobals->curtime + GetCSWpnData().m_flTimeBetweenBurstShots;
	}
	else
	{
		m_fNextBurstShot = 0.0f;
	}

	const CCSWeaponInfo& weaponInfo = GetCSWpnData();

	// update accuracy
	m_fAccuracyPenalty += weaponInfo.m_fInaccuracyImpulseFire[m_weaponMode];

	// table driven recoil
	Recoil( Secondary_Mode );

	++pPlayer->m_iShotsFired;
	m_flRecoilIndex += 1.0f;
	--m_iClip1;
}

bool CWeaponCSBaseGun::CSBaseGunFire( float flCycleTime, CSWeaponMode weaponMode )
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( m_iClip1 == 0 )
	{
		if ( m_bFireOnEmpty )
		{
			PlayEmptySound();

			m_iNumEmptyAttacks++;

			// NOTE[pmf]: we don't want to actually play the dry fire animations, as most seem to depict the weapon actually firing.
			// SendWeaponAnim( ACT_VM_DRYFIRE );

			//++pPlayer->m_iShotsFired;	// don't play "auto" empty clicks -- make the player release the trigger before clicking again
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;

			if ( IsRevolver() )
			{
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetCSWpnData().m_flCycleTime[weaponMode];
				SendWeaponAnim( ACT_VM_DRYFIRE ); // empty!
			}
			m_bFireOnEmpty = false;
		}

		return false;
	}

	float flCurAttack = CalculateNextAttackTime( flCycleTime );

	if ( (GetWeaponType() != WEAPONTYPE_SNIPER_RIFLE && IsZoomed()) || (IsRevolver() && weaponMode == Secondary_Mode) )
	{
		SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	}
	else if ( IsRevolver() )
	{
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	}
	else
	{
		CIronSightController* pIronSightController = GetIronSightController();
		if ( pIronSightController )
			// PiMoN: hacky but for some reason IsInIronSight() returns false server-side resulting in a wrong anim
			//SendWeaponAnim( pIronSightController->IsInIronSight() ? ACT_VM_SECONDARYATTACK : ACT_VM_PRIMARYATTACK );
			SendWeaponAnim( (weaponMode == Secondary_Mode) ? ACT_VM_SECONDARYATTACK : ACT_VM_PRIMARYATTACK );
		else
			SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	}

	++pPlayer->m_iShotsFired;
	m_iClip1--;

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->GetFinalAimAngle(),
		GetCSWeaponID(),
		weaponMode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetInaccuracy(),
		GetSpread(), 
		flCurAttack,
		(HasSilencer() > 0 && IsSilenced()) ? SPECIAL1 : SINGLE,
		m_flRecoilIndex );

	DoFireEffects();

#ifdef CLIENT_DLL
	if ( GetIronSightController() )
	{
		GetIronSightController()->IncreaseDotBlur( RandomFloat( 0.22f, 0.28f ) );
	}
#endif

	SetWeaponIdleTime( gpGlobals->curtime + GetCSWpnData().m_flTimeToIdleAfterFire );

	// update accuracy
	m_fAccuracyPenalty += GetCSWpnData().m_fInaccuracyImpulseFire[weaponMode];

	// table driven recoil
	Recoil( weaponMode );

	m_flRecoilIndex += 1.0f;

	return true;
}

bool CWeaponCSBaseGun::IsFullAuto() const
{
	if ( BaseClass::IsFullAuto() )
	{
		return !IsInBurstMode();
	}
	else
	{
		return false;
	}
}

void CWeaponCSBaseGun::DoFireEffects()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	
	if ( pPlayer )
		 pPlayer->DoMuzzleFlash();
}


bool CWeaponCSBaseGun::Reload()
{
	CCSPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if ( GetReserveAmmoCount( AMMO_POSITION_PRIMARY ) <= 0 )
		return false;

	int iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), m_iReloadActivityIndex );
	if ( !iResult )
		return false;

	m_iIronSightMode = IronSight_should_approach_unsighted;

	pPlayer->SetAnimation( PLAYER_RELOAD );

	if ( HasZoom() )
	{
		m_zoomLevel = 0;
		m_weaponMode = Primary_Mode;
	}

	if ( pPlayer->GetFOV() != pPlayer->GetDefaultFOV() && pPlayer->m_bIsScoped )
	{
		pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), 0.0f );
		pPlayer->m_bIsScoped = false;
	}

	pPlayer->m_iShotsFired = 0;
	m_flRecoilIndex += 1.0f;

	return BaseClass::Reload();
}

void CWeaponCSBaseGun::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	if ( m_iClip1 != 0 )
	{
		SetWeaponIdleTime( gpGlobals->curtime + GetCSWpnData().m_flIdleInterval );
		SendWeaponAnim( ACT_VM_IDLE );
	}
}

bool CWeaponCSBaseGun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	// re-deploying the weapon is punishment enough for canceling a silencer attach/detach before completion
	if ( (GetActivity() == ACT_VM_ATTACH_SILENCER && m_bSilencerOn == false) ||
		 (GetActivity() == ACT_VM_DETACH_SILENCER && m_bSilencerOn == true) )
	{
		m_flDoneSwitchingSilencer = gpGlobals->curtime;
		m_flNextSecondaryAttack = gpGlobals->curtime;
		m_flNextPrimaryAttack = gpGlobals->curtime;
	}

	if ( HasZoom() )
	{
		m_zoomLevel = 0;
		m_weaponMode = Primary_Mode;
	}

	// not sure we want to fully support animation cancelling
	if ( m_bInReload && !m_bReloadVisuallyComplete )
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime;
	}
	return BaseClass::Holster( pSwitchingTo );
}

bool CWeaponCSBaseGun::HasZoom()
{
	return GetCSWpnData().m_iZoomLevels != 0;
}

bool CWeaponCSBaseGun::IsInBurstMode() const
{
	return m_bBurstMode;
}

bool CWeaponCSBaseGun::IsZoomed( void ) const
{
	return (m_zoomLevel > 0);
}
