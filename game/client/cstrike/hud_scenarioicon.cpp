//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "clientmode_csnormal.h"
#include "c_cs_player.h"
#include "cs_gamerules.h"

#include "c_cs_hostage.h"
#include "c_plantedc4.h"

class CHudScenarioHostageIcon : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudScenarioHostageIcon, vgui::Panel );

	CHudScenarioHostageIcon( const char *name );

	virtual bool ShouldDraw();	
	virtual void Paint();

private:
	CPanelAnimationVar( Color, m_clrIcon, "IconColor", "IconColor" );	

	CHudTexture *m_pIcon;
};


DECLARE_HUDELEMENT( CHudScenarioHostageIcon );


CHudScenarioHostageIcon::CHudScenarioHostageIcon( const char *pName ) :
	vgui::Panel( NULL, "HudScenarioHostageIcon" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );
	m_pIcon = NULL;

	SetHiddenBits( HIDEHUD_PLAYERDEAD );
}

bool CHudScenarioHostageIcon::ShouldDraw()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	return pPlayer && pPlayer->IsAlive();
}

void CHudScenarioHostageIcon::Paint()
{
	CCSGameRules *pRules = CSGameRules();

	// If there are hostages, draw how many there are
	if( pRules && pRules->GetNumHostagesRemaining() )
	{
		if ( !m_pIcon )
		{
			m_pIcon = gHUD.GetIcon( "scenario_hostage" );
		}

		if( m_pIcon )
		{
			int xpos = 0;
			int iconWidth = m_pIcon->Width();

			for(int i=0;i<pRules->GetNumHostagesRemaining();i++)
			{
				m_pIcon->DrawSelf( xpos, 0, m_clrIcon );
				xpos += iconWidth;
			}
		}
	}
}


