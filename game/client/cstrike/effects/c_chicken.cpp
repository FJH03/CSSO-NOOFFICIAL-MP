//========= Copyright © 1996-2012, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side interactive, shootable chicken
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "c_chicken.h"
#include "c_breakableprop.h"
#include "c_cs_player.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#undef CChicken

IMPLEMENT_CLIENTCLASS_DT( C_CChicken, DT_CChicken, CChicken )
RecvPropInt( RECVINFO( m_jumpedThisFrame ), 0, C_CChicken::RecvProxy_Jumped ),
RecvPropEHandle( RECVINFO( m_leader ) ),
END_RECV_TABLE()


//-----------------------------------------------------------------------------------------------
C_CChicken::C_CChicken()
{
}


//-----------------------------------------------------------------------------------------------
C_CChicken::~C_CChicken()
{
}


//-----------------------------------------------------------------------------------------------
void C_CChicken::Spawn( void )
{
	BaseClass::Spawn();

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_CChicken::RecvProxy_Jumped( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
// 	C_CHostage *pHostage = ( C_CHostage * ) pStruct;
// 
// 	bool jumped = pData->m_Value.m_Int != 0;
// 
// 	if ( jumped )
// 	{
// 		// hostage jumped
// 		pHostage->m_PlayerAnimState->DoAnimationEvent( PLAYERANIMEVENT_JUMP );
// 		pHostage->SetNextClientThink( gpGlobals->curtime );
// 	}
// 
// 	pHostage->m_jumpedThisFrame = jumped;
}

//-----------------------------------------------------------------------------------------------
void C_CChicken::ClientThink()
{
	BaseClass::ClientThink();

	Activity currentActivity = GetSequenceActivity( GetSequence() );
	if ( m_lastActivity != currentActivity )
	{
		if ( currentActivity == ACT_CLIMB_UP )
			SetCycle( 0 );
		m_lastActivity = currentActivity;
	}
	
#if CSGO_CLIENT_CHICKEN_ANTLERS_IN_FREEZE_CAM
	bool bEnableHolidayHat = false;

	//
	// This code was enabling chicken antlers in freezecam
	//

	// Client-side only addons are required for the duration of the freeze shot
	{
		static ConVarRef ref_cl_disablefreezecam( "cl_disablefreezecam" );
		static ConVarRef ref_sv_disablefreezecam( "sv_disablefreezecam" );
		extern bool IsTakingAFreezecamScreenshot( void );
		if ( IsTakingAFreezecamScreenshot() && !ref_cl_disablefreezecam.GetBool() && !ref_sv_disablefreezecam.GetBool() )
		{
			C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
			if ( pLocalPlayer && pLocalPlayer->GetObserverTarget() && ( pLocalPlayer->GetObserverTarget() != pLocalPlayer ) )
			{
				bEnableHolidayHat = true;
			}
		}
	}
#endif
}
