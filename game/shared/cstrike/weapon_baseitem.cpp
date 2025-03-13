//========= Copyright � 1996-2009, Valve Corporation, All rights reserved. ============//
//
// Purpose: base class for belt items, eg pills and adrenaline
//
// $NoKeywords: $
//=====================================================================================//

#include "cbase.h"
#include "weapon_baseitem.h"
#include "cs_gamerules.h"

#if defined( CLIENT_DLL )
#include "c_cs_player.h"
#include "cs_hud_weaponselection.h"
#else
#include "cs_player.h"
#endif // CLIENT_DLL

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBaseItem, DT_WeaponBaseItem )

BEGIN_NETWORK_TABLE( CWeaponBaseItem, DT_WeaponBaseItem )
#ifndef CLIENT_DLL
SendPropBool( SENDINFO( m_bRedraw ) ),
SendPropBool( SENDINFO( m_bUseVisuallyComplete ) ),
#else
RecvPropBool( RECVINFO( m_bRedraw ) ),
RecvPropBool( RECVINFO( m_bUseVisuallyComplete ) ),
#endif

END_NETWORK_TABLE()

#if defined CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponBaseItem )
DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bUseVisuallyComplete, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif


#ifndef CLIENT_DLL
BEGIN_DATADESC( CWeaponBaseItem )
DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ),
END_DATADESC()
#endif

CWeaponBaseItem::CWeaponBaseItem()
{
	m_bRedraw = false;
	m_bUseVisuallyComplete = false;
}

//--------------------------------------------------------------------------------------------------------
void CWeaponBaseItem::Spawn( void )
{
	m_UseTimer.Invalidate();
	BaseClass::Spawn();
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
}


//--------------------------------------------------------------------------------------------------------
bool CWeaponBaseItem::Deploy( void )
{
	m_bRedraw = false;
	m_UseTimer.Invalidate();
	m_bUseVisuallyComplete = false;
	return BaseClass::Deploy();
}


//--------------------------------------------------------------------------------------------------------
bool CWeaponBaseItem::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_UseTimer.Invalidate();
	return BaseClass::Holster( pSwitchingTo );
}

//--------------------------------------------------------------------------------------------------------
void CWeaponBaseItem::ItemHolsterFrame()
{
	if ( m_bUseVisuallyComplete )
	{
		CBasePlayer* pPlayer = GetPlayerOwner();
		if ( pPlayer && pPlayer->IsAlive() )
		{
			// if we are already done using, then remove the weapon in here instead of waiting for
			// use animation to complete (because we are already holstered, duh)
			// adding this in Holster() won't work because its a process of calling multiple methods
			// and it won't be good to just suddenly remove this weapon in the middle of this processs, isn't it?
			if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
			{
				pPlayer->Weapon_Drop( this, NULL, NULL );
#ifndef CLIENT_DLL	
				UTIL_Remove( this );
#endif
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
// bool CWeaponBaseItem::CanExtendHelpingHand( void ) const
// {
// 	return !m_UseTimer.HasStarted() && BaseClass::CanExtendHelpingHand();
// }


//--------------------------------------------------------------------------------------------------------
void CWeaponBaseItem::PrimaryAttack( void )
{
	CCSPlayer *pPlayer = ToCSPlayer( GetPlayerOwner() );
	if (pPlayer == NULL)
		return;

	if ( m_UseTimer.HasStarted() )
	{
		return;
	}

// 	if ( HelpingHandPrimaryAttack() )
// 	{
// 		return;
// 	}

	if ( !CanUseOnSelf( pPlayer ) )
		return;

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_PRIMARY );

	OnStartUse( pPlayer );

	m_UseTimer.Start( GetUseTimerDuration() );
}

float CWeaponBaseItem::GetUseTimerDuration( void )
{
	return SequenceDuration();
}


extern ConVar z_use_belt_item_tolerance;

//--------------------------------------------------------------------------------------------------------
void CWeaponBaseItem::SecondaryAttack( void )
{
	CCSPlayer *pPlayer = ToCSPlayer( GetPlayerOwner() );
	if (pPlayer == NULL)
		return;

	if ( m_UseTimer.HasStarted() )
		return;

// 	static const float GiveRange = 256.0f;
// 	CCSPlayer *target = ToCSPlayer( pPlayer->FindUseEntity( GiveRange, 0.0f, z_use_belt_item_tolerance.GetFloat(), NULL, true ) ); // Prefer to hit players
// 	if ( target && target->IsOnASurvivorTeam() && !target->IsIncapacitated() )
// 	{
// #ifdef GAME_DLL
// 		pPlayer->GiveActiveWeapon( target );
// #endif
// 		return;
// 	}

	BaseClass::SecondaryAttack();
}


//--------------------------------------------------------------------------------------------------------
/**
* Called when no buttons are pressed
*/
void CWeaponBaseItem::WeaponIdle( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponBaseItem::Reload()
{
	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

		//Mark this as done
		//	m_bRedraw = false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------
/**
* Called each frame by the player PostThink
*/
void CWeaponBaseItem::ItemPostFrame( void )
{
	CCSPlayer *pPlayer = ToCSPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return;

#ifndef CLIENT_DLL
	int buttons = pPlayer->m_nButtons;
#endif

	BaseClass::ItemPostFrame();

	if ( m_UseTimer.HasStarted() && m_UseTimer.IsElapsed() )
	{
		// pills can only help you so much
		if ( !pPlayer->IsAlive() )
		{
			m_UseTimer.Invalidate();
			return;
		}
#ifndef CLIENT_DLL
		if ( !m_bUseVisuallyComplete )
			CompleteUse( pPlayer );
		// BaseClass::ItemPostFrame() clears IN_ATTACK2, so we restore it here to prevent the next weapon from bashing immediately
		pPlayer->m_nButtons = buttons;

		m_bRedraw = true;
#endif
		m_UseTimer.Invalidate();

		if ( !m_bUseVisuallyComplete )
		{
			// remove the ammo
			pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType, true );
		}

		if ( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
		{
			pPlayer->Weapon_Drop( this, NULL, NULL );
#ifndef CLIENT_DLL	
			UTIL_Remove( this );
#endif
		}
		else
		{
			pPlayer->SwitchToNextBestWeapon( this );
		}

#if defined (CLIENT_DLL)
		// when an item is removed, force the local player to update their inventory screen
		C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
		if ( pLocalPlayer && pLocalPlayer == pPlayer )
		{
			CCSHudWeaponSelection *pHudWS = GET_HUDELEMENT( CCSHudWeaponSelection );
			if ( pHudWS )
			{
				int nAmmoCount = pPlayer->GetAmmoCount( m_iPrimaryAmmoType );
				if ( nAmmoCount <= 0 )
				{
					pHudWS->ShowAndUpdateSelection( WEPSELECT_DROP, this );
				}
				else
				{
					// we need to tell the hud that this weapon still exists and then update the selected weapon
					pHudWS->ShowAndUpdateSelection( WEPSELECT_PICKUP, this );
				}
			}
		}
#endif
	}

}

// //--------------------------------------------------------------------------------------------------------
// bool CWeaponBaseItem::OnHit( trace_t &trace, const Vector &swingVector, bool firstTime ) 
// {
// 	if ( trace.m_pEnt && trace.m_pEnt->IsPlayer() && IsASurvivorTeam( trace.m_pEnt->GetTeamNumber() ) )
// 		return false;	// don't hit survivors who are outside of heal range if we're trying to get close and heal them.
// 
// 	return BaseClass::OnHit( trace, swingVector, firstTime );
// }

//--------------------------------------------------------------------------------------------------------
bool CWeaponBaseItem::SendWeaponAnim( int iActivity )
{
	//iActivity = TranslateViewmodelActivity( (Activity)iActivity );
	return BaseClass::SendWeaponAnim( iActivity );
}

//--------------------------------------------------------------------------------------------------------
bool CWeaponBaseItem::CanFidget( void )
{
	return false;
}


//--------------------------------------------------------------------------------------------------------