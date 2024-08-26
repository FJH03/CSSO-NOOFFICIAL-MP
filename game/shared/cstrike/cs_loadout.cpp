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