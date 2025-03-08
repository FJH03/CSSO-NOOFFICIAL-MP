//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/VectorImagePanel.h>
#include <vgui/ISurface.h>
#include "clientmode_csnormal.h"
#include "cs_gamerules.h"
#include "hud_numericdisplay.h"


class CHudC4 : public CHudElement, public vgui::VectorImagePanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudC4, vgui::VectorImagePanel );

	CHudC4( const char *name );
	virtual bool ShouldDraw();
	virtual void OnThink();

private:
	CPanelAnimationVar( Color, m_clrIcon, "IconColor", "White" );
	CPanelAnimationVar( Color, m_clrBombZone, "BombZoneColor", "White" );

	bool bInBombZone;
};


DECLARE_HUDELEMENT( CHudC4 );


CHudC4::CHudC4( const char *pName ) :
	vgui::VectorImagePanel( NULL, "HudC4" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	bInBombZone = true;
}

void CHudC4::OnThink()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return;

	if ( bInBombZone != pPlayer->m_bInBombZone )
	{
		bInBombZone = pPlayer->m_bInBombZone;
		SetFgColor( bInBombZone ? m_clrBombZone : m_clrIcon );
	}
}

bool CHudC4::ShouldDraw()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();

	// if we are spectating another player first person, check this player
	if ( pPlayer && (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE) )
	{
		pPlayer = ToCSPlayer( pPlayer->GetObserverTarget() );
	}
	
	// [tj] Added base class call
	return pPlayer && pPlayer->HasC4() && CHudElement::ShouldDraw();
}
