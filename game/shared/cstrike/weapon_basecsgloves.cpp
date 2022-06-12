//========= Copyright PiMoNFeeD, CS:SO, All rights reserved. ==================//
//
// Purpose: Base class for all in-game gloves
//
//=============================================================================//

#include "cbase.h"
#include "weapon_basecsgloves.h"
#include "cs_loadout.h"
#ifdef CLIENT_DLL
#include "c_cs_player.h"
#else
#include "cs_player.h"
#endif

LINK_ENTITY_TO_CLASS( cs_base_glove, CBaseCSGloves );
IMPLEMENT_NETWORKCLASS_ALIASED( BaseCSGloves, DT_BaseCSGloves )

BEGIN_NETWORK_TABLE( CBaseCSGloves, DT_BaseCSGloves )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_nGloveID ) ),
#else
	SendPropInt( SENDINFO( m_nGloveID ), 8 ),
#endif
END_NETWORK_TABLE()

BEGIN_DATADESC( CBaseCSGloves )
END_DATADESC()

CBaseCSGloves::CBaseCSGloves()
{
#ifndef CLIENT_DLL
	m_nGloveID = 0;
#endif
}

#ifdef GAME_DLL
int CBaseCSGloves::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}
#endif

void CBaseCSGloves::Equip( CBaseAnimating* pOwner )
{
	if ( !pOwner )
		return;

	if ( !pOwner->IsAlive() )
		return;

	FollowEntity( pOwner, true );
	SetOwnerEntity( pOwner );
	AddEffects( EF_BONEMERGE_FASTCULL );
	SetSolid( SOLID_NONE );

	UpdateGlovesModel();

#ifdef CLIENT_DLL
	SetUseParentLightingOrigin( true );
#endif

	// assuming that before equipping them, a DoesModelSupportGloves() check was made
	pOwner->SetBodygroup( pOwner->FindBodygroupByName( "gloves" ), 1 ); // hide default gloves
}

void CBaseCSGloves::UnEquip()
{
	CCSPlayer *pPlayerOwner = ToCSPlayer( GetOwnerEntity() );

	if ( !pPlayerOwner )
	{
		return;
	}

	pPlayerOwner->SetBodygroup( pPlayerOwner->FindBodygroupByName( "gloves" ), 0 ); // restore default gloves

	SetOwnerEntity( NULL );
}

void CBaseCSGloves::UpdateGlovesModel()
{
	CCSPlayer *pPlayerOwner = ToCSPlayer( GetOwnerEntity() );
	if ( !pPlayerOwner )
		return;

#ifdef CLIENT_DLL
	MDLCACHE_CRITICAL_SECTION();
#endif
	const char *pszModel = GetGlovesInfo( m_nGloveID )->szWorldModel;
	SetModel( pszModel );

#ifdef CLIENT_DLL
	if ( pPlayerOwner->m_pViewmodelArmConfig != NULL )
		m_nSkin = pPlayerOwner->m_pViewmodelArmConfig->iSkintoneIndex;
	else
#endif
	{
		CStudioHdr *pHdr = pPlayerOwner->GetModelPtr();
		if ( pHdr )
			m_nSkin = GetPlayerViewmodelArmConfigForPlayerModel( pHdr->pszName() )->iSkintoneIndex;
	}
}
