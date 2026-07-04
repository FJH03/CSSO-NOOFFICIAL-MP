//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "basecombatweapon_shared.h"
#include "baseviewmodel_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar r_drawtracers( "r_drawtracers", "1", FCVAR_ARCHIVE, "Toggle visibility of weapon tracers" );
ConVar r_drawtracers_firstperson( "r_drawtracers_firstperson", "1", FCVAR_ARCHIVE, "Toggle visibility of first person weapon tracers" );
ConVar r_drawtracers_movetonotintersect( "r_drawtracers_movetonotintersect", "1", FCVAR_CHEAT, "" );
extern ConVar cl_righthand;
void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );

#define	TRACER_SPEED			5000 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector GetTracerOrigin( const CEffectData &data )
{
	Vector vecStart = data.m_vStart;
	QAngle vecAngles;

	int iAttachment = data.m_nAttachmentIndex;;

	// Attachment?
	if ( data.m_fFlags & TRACER_FLAG_USEATTACHMENT )
	{
		C_BaseViewModel *pViewModel = NULL;

		// If the entity specified is a weapon being carried by this player, use the viewmodel instead
		IClientRenderable *pRenderable = data.GetRenderable();
		if ( !pRenderable )
			return vecStart;

		C_BaseEntity *pEnt = data.GetEntity();

// This check should probably be for all multiplayer games, investigate later
#if defined( TF_CLIENT_DLL )
		if ( pEnt && pEnt->IsDormant() )
			return vecStart;
#endif

		C_BaseCombatWeapon *pWpn = dynamic_cast<C_BaseCombatWeapon *>( pEnt );
		if ( pWpn && pWpn->IsCarriedByLocalPlayer() )
		{
			C_BasePlayer *player = ToBasePlayer( pWpn->GetOwner() );

			pViewModel = player ? player->GetViewModel( 0 ) : NULL;
			if ( pViewModel )
			{
				// Get the viewmodel and use it instead
				pRenderable = pViewModel;
			}
		}
		
		// Get the attachment origin
		if ( !pRenderable->GetAttachment( iAttachment, vecStart, vecAngles ) )
		{
			DevMsg( "GetTracerOrigin: Couldn't find attachment %d on model %s\n", iAttachment, 
				modelinfo->GetModelName( pRenderable->GetModel() ) );
		}
	}

	return vecStart;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TracerCallback( const CEffectData &data )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	if ( !r_drawtracers.GetBool() )
		return;

	if ( !r_drawtracers_firstperson.GetBool() )
	{
		C_BaseViewModel *pViewModel = dynamic_cast<C_BaseViewModel*>(data.GetEntity());
		if ( pViewModel )
			return;
	}

	// Grab the data
	Vector vecStart = GetTracerOrigin( data );
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	int iEntIndex = data.entindex();

	if ( iEntIndex && iEntIndex == player->index )
	{
		Vector	foo = data.m_vStart;
		QAngle	vangles;
		Vector	vforward, vright, vup;

		engine->GetViewAngles( vangles );
		AngleVectors( vangles, &vforward, &vright, &vup );

		VectorMA( data.m_vStart, 4, vright, foo );
		foo[2] -= 0.5f;

		FX_PlayerTracer( foo, (Vector&)data.m_vOrigin );
		return;
	}
	
	// Use default velocity if none specified
	if ( !flVelocity )
	{
		flVelocity = TRACER_SPEED;
	}

	// Do tracer effect
	FX_Tracer( (Vector&)vecStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz );
}

DECLARE_CLIENT_EFFECT( "Tracer", TracerCallback )

//-----------------------------------------------------------------------------
// Purpose: HL2DM-style tracer — FX_PlayerTracer (1P) / FX_Tracer (3P).
//          No particle system, won't crash leaf system.
//-----------------------------------------------------------------------------
static int s_nWeaponTracerIndex; // unused, kept for compatibility

void ParticleTracerCallback( const CEffectData &data )
{
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pLocalPlayer )
		return;

	if ( !r_drawtracers.GetBool() )
		return;

	C_BaseEntity *pEntity = data.GetEntity();
	C_BaseViewModel *pViewModel = dynamic_cast< C_BaseViewModel * >( pEntity );

	// Honor first-person tracer toggle
	if ( !r_drawtracers_firstperson.GetBool() && pViewModel )
		return;

	// --- 找到 tracer 所属的玩家 ---
	// entity 可能是: v模 / 武器w模 / C_CSPlayer
	C_BasePlayer *pOwner = NULL;
	if ( pViewModel )
	{
		pOwner = ToBasePlayer( pViewModel->GetOwner() );
	}
	else
	{
		pOwner = ToBasePlayer( pEntity );                    // 玩家自身
		if ( !pOwner )
		{
			C_BaseCombatWeapon *pWpn = dynamic_cast< C_BaseCombatWeapon * >( pEntity );
			if ( pWpn )
				pOwner = ToBasePlayer( pWpn->GetOwner() );   // 武器的持有者
		}
	}

	// --- 本地玩家 / 第一人称观战的 tracer → 1P 弹道 ---
	bool bFirstPerson = ( pOwner == pLocalPlayer ) ||
		( pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE &&
		  pLocalPlayer->GetObserverTarget() == pOwner &&
		  pLocalPlayer->GetObserverInterpState() != C_BasePlayer::OBSERVER_INTERP_TRAVELING );

	if ( bFirstPerson && pOwner )
	{
		C_BaseViewModel *pVM = pOwner->GetViewModel( 0 );
		if ( pVM )
		{
			Vector vecMuzzle;
			int iAttach = pVM->LookupAttachment( "muzzle_flash" );
			if ( iAttach <= 0 )
				iAttach = pVM->LookupAttachment( "1" );

			QAngle angDummy;
			if ( iAttach > 0 && pVM->GetAttachment( iAttach, vecMuzzle, angDummy ) )
			{
				FormatViewModelAttachment( vecMuzzle, true );
				FX_PlayerTracer( vecMuzzle, (Vector &)data.m_vOrigin );
				return;
			}
		}
	}

	// --- Third-person (other players / NPCs) ---
	Vector vecStart = GetTracerOrigin( data );
	Vector vecEnd   = data.m_vOrigin;

	float flVelocity = data.m_flScale;
	if ( !flVelocity )
		flVelocity = TRACER_SPEED;

	bool bWhiz = ( data.m_fFlags & TRACER_FLAG_WHIZ ) != 0;

	FX_Tracer( (Vector &)vecStart, (Vector &)vecEnd, flVelocity, bWhiz );
}

DECLARE_CLIENT_EFFECT( "ParticleTracer", ParticleTracerCallback );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TracerSoundCallback( const CEffectData &data )
{
	// Grab the data
	Vector vecStart = GetTracerOrigin( data );
	
	// Do tracer effect
	FX_TracerSound( vecStart, (Vector&)data.m_vOrigin, data.m_fFlags );
}

DECLARE_CLIENT_EFFECT( "TracerSound", TracerSoundCallback );
