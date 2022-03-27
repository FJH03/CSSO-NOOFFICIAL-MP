//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui/ISurface.h>
#include "clientmode_csnormal.h"
#include "c_cs_player.h"
#include "cs_gamerules.h"

#include "c_cs_hostage.h"
#include "c_plantedc4.h"

extern ConVar cl_draw_only_deathnotices;

#define HOSTAGE_ALIVE_ICON "materials/vgui/hud/svg/hostage_alive.svg"
#define HOSTAGE_DEAD_ICON "materials/vgui/hud/svg/hostage_alive.svg"
#define HOSTAGE_RESCUED_ICON "materials/vgui/hud/svg/hostage_rescued.svg"
#define HOSTAGE_TRANSIT_ICON "materials/vgui/hud/svg/hostage_transit.svg"

class CHudScenarioHostageIcon : public CHudElement, public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudScenarioHostageIcon, vgui::EditablePanel );

	CHudScenarioHostageIcon( const char *name );
	virtual void OnThink();
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );
	virtual bool ShouldDraw();

	virtual void FireGameEvent( IGameEvent * event );

private:
	VectorImagePanel	*m_pHostageIcon1;
	VectorImagePanel	*m_pHostageIcon2;
	VectorImagePanel	*m_pHostageIcon3;
	VectorImagePanel	*m_pHostageIcon4;
	VectorImagePanel	*m_pRescueZoneIcon;

	CPanelAnimationVarAliasType( int, rescuezone_icon_margin_right, "rescuezone_icon_margin_right", "0", "proportional_width" );
	CPanelAnimationVar( Color, m_clrHostageAlive, "hostage_alive_fgcolor", "White" );
	CPanelAnimationVar( Color, m_clrHostageRescued, "hostage_rescued_fgcolor", "White" );
};

DECLARE_HUDELEMENT( CHudScenarioHostageIcon );

CHudScenarioHostageIcon::CHudScenarioHostageIcon( const char *pName ) :
	vgui::EditablePanel( NULL, "HudScenarioHostageIcon" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );

	m_pHostageIcon1 = new VectorImagePanel( this, "HostageIcon1" );
	m_pHostageIcon2 = new VectorImagePanel( this, "HostageIcon2" );
	m_pHostageIcon3 = new VectorImagePanel( this, "HostageIcon3" );
	m_pHostageIcon4 = new VectorImagePanel( this, "HostageIcon4" );
	m_pRescueZoneIcon = new VectorImagePanel( this, "RescueZoneIcon" );

	LoadControlSettings( "resource/hud/hostage.res" );

	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "hostage_rescued" );
}

void CHudScenarioHostageIcon::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
 	// reload the .res file so items are rescaled
 	LoadControlSettings( "resource/hud/hostage.res" );
}

void CHudScenarioHostageIcon::FireGameEvent( IGameEvent * event )
{
	/*const char *eventname = event->GetName();

	if ( !eventname || !eventname[0] )
		return;

	bool bHostage1 = g_Hostages.Count() >= 1;
	bool bHostage2 = g_Hostages.Count() >= 2;
	bool bHostage3 = g_Hostages.Count() >= 3;
	bool bHostage4 = g_Hostages.Count() >= 4;

	// TODO: support for transit and dead icons
	if ( Q_strcmp( "round_start", eventname ) == 0 )
	{
		// reset all hostage icons
		m_pHostageIcon1->SetTexture( HOSTAGE_ALIVE_ICON );
		m_pHostageIcon1->SetFgColor( m_clrHostageAlive );
		m_pHostageIcon2->SetTexture( HOSTAGE_ALIVE_ICON );
		m_pHostageIcon2->SetFgColor( m_clrHostageAlive );
		m_pHostageIcon3->SetTexture( HOSTAGE_ALIVE_ICON );
		m_pHostageIcon3->SetFgColor( m_clrHostageAlive );
		m_pHostageIcon4->SetTexture( HOSTAGE_ALIVE_ICON );
		m_pHostageIcon4->SetFgColor( m_clrHostageAlive );
	}
	else if ( Q_strcmp( "hostage_rescued", eventname ) == 0 )
	{
		if ( bHostage4 && g_Hostages[3]->IsRescued() )
		{
			m_pHostageIcon4->SetTexture( HOSTAGE_RESCUED_ICON );
			m_pHostageIcon4->SetFgColor( m_clrHostageRescued );
		}
		if ( bHostage3 && g_Hostages[2]->IsRescued() )
		{
			m_pHostageIcon3->SetTexture( HOSTAGE_RESCUED_ICON );
			m_pHostageIcon3->SetFgColor( m_clrHostageRescued );
		}
		if ( bHostage2 && g_Hostages[1]->IsRescued() )
		{
			m_pHostageIcon2->SetTexture( HOSTAGE_RESCUED_ICON );
			m_pHostageIcon2->SetFgColor( m_clrHostageRescued );
		}
		if ( bHostage1 && g_Hostages[0]->IsRescued() )
		{
			m_pHostageIcon1->SetTexture( HOSTAGE_RESCUED_ICON );
			m_pHostageIcon1->SetFgColor( m_clrHostageRescued );
		}
	}*/
}

void CHudScenarioHostageIcon::OnThink()
{
	if ( g_Hostages.Count() < 1 )
	{
		m_pHostageIcon1->SetVisible( false );
		m_pHostageIcon2->SetVisible( false );
		m_pHostageIcon3->SetVisible( false );
		m_pHostageIcon4->SetVisible( false );
		m_pRescueZoneIcon->SetVisible( false );
		return;
	}
	
	C_CSPlayer *pPlayer = GetHudPlayer();
	if ( !pPlayer )
		return;

	/*bool bHostage1 = g_Hostages.Count() >= 1;
	bool bHostage2 = g_Hostages.Count() >= 2;
	bool bHostage3 = g_Hostages.Count() >= 3;
	bool bHostage4 = g_Hostages.Count() >= 4;*/
	int numHostages = CSGameRules()->GetNumHostagesRemaining();
	bool bHostage1 = numHostages >= 1;
	bool bHostage2 = numHostages >= 2;
	bool bHostage3 = numHostages >= 3;
	bool bHostage4 = numHostages >= 4;

	m_pHostageIcon1->SetVisible( bHostage1 );
	m_pHostageIcon2->SetVisible( bHostage2 );
	m_pHostageIcon3->SetVisible( bHostage3 );
	m_pHostageIcon4->SetVisible( bHostage4 );
	m_pRescueZoneIcon->SetVisible( pPlayer->IsInHostageRescueZone() );

	// kill me
	if ( bHostage4 )
		m_pRescueZoneIcon->SetPos( m_pHostageIcon4->GetXPos() + m_pHostageIcon4->GetWide() + rescuezone_icon_margin_right,
								   m_pRescueZoneIcon->GetYPos() );
	else if ( bHostage3 )
		m_pRescueZoneIcon->SetPos( m_pHostageIcon3->GetXPos() + m_pHostageIcon3->GetWide() + rescuezone_icon_margin_right,
								   m_pRescueZoneIcon->GetYPos() );
	else if ( bHostage2 )
		m_pRescueZoneIcon->SetPos( m_pHostageIcon2->GetXPos() + m_pHostageIcon2->GetWide() + rescuezone_icon_margin_right,
								   m_pRescueZoneIcon->GetYPos() );
	else if ( bHostage1 )
		m_pRescueZoneIcon->SetPos( m_pHostageIcon1->GetXPos() + m_pHostageIcon1->GetWide() + rescuezone_icon_margin_right,
								   m_pRescueZoneIcon->GetYPos() );
	else
		m_pRescueZoneIcon->SetVisible( false );
}

bool CHudScenarioHostageIcon::ShouldDraw()
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	return CHudElement::ShouldDraw();
}

