#include "cbase.h"
#include "cs_loadout.h"
#include "cs_shareddefs.h"
#ifdef CLIENT_DLL
#include "c_cs_player.h"
#else
#include "cs_player.h"
#endif

CCSLoadout*	g_pCSLoadout = NULL;
CCSLoadout::CCSLoadout() : CAutoGameSystemPerFrame("CCSLoadout")
{
	Assert( !g_pCSLoadout );
	g_pCSLoadout = this;
}
CCSLoadout::~CCSLoadout()
{
	Assert( g_pCSLoadout == this );
	g_pCSLoadout = NULL;
}

bool CCSLoadout::HasGlovesSet( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return false;


	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotGlovesCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotGlovesT;
			break;
		default:
			break;
	}

	return (value > 0) ? true : false;
}

int CCSLoadout::GetGlovesForPlayer( CCSPlayer* pPlayer, int team )
{
	if ( !pPlayer )
		return 0;

	int value = 0;
	switch ( team )
	{
		case TEAM_CT:
			value = pPlayer->m_iLoadoutSlotGlovesCT;
			break;
		case TEAM_TERRORIST:
			value = pPlayer->m_iLoadoutSlotGlovesT;
			break;
		default:
			break;
	}

	return value;
}