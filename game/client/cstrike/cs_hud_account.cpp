//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
//
// implementation of CHudAccount class
//
#include "cbase.h"
#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/VectorImagePanel.h>

using namespace vgui;

#include "hudelement.h"
#include "c_cs_player.h"
#include "cs_gamerules.h"

#include "convar.h"

extern ConVar cl_hud_background_alpha;
extern ConVar mp_maxmoney;
extern ConVar cl_hud_color;
extern ConVar cl_draw_only_deathnotices;

//-----------------------------------------------------------------------------
// Purpose: Money panel
//-----------------------------------------------------------------------------
class CHudAccount : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudAccount, EditablePanel );

public:
	CHudAccount( const char *pElementName );
	virtual void Init( void );
	virtual void OnThink();
	virtual bool ShouldDraw();
	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall );

private:
	int		m_iAccount;

	VectorImagePanel	*m_pBuyZoneIcon;
	Label				*m_pAccountLabel;

	CPanelAnimationVarAliasType( int, buyzone_icon_xpos, "buyzone_icon_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, buyzone_icon_ypos, "buyzone_icon_ypos", "0", "proportional_ypos" );
	CPanelAnimationVarAliasType( int, margin_right, "margin_right", "0", "proportional_width" );
};

DECLARE_HUDELEMENT( CHudAccount );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAccount::CHudAccount( const char *pElementName ) : CHudElement( pElementName ), EditablePanel(NULL, "HudAccount")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_NOT_OBSERVING_PLAYERS );

	m_pAccountLabel = new Label( this, "AccountLabel", "" );
	m_pBuyZoneIcon = new VectorImagePanel( this, "BuyZoneIcon" );

	LoadControlSettings( "resource/hud/account.res" );
}

void CHudAccount::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
 	// reload the .res file so items are rescaled
 	LoadControlSettings( "resource/hud/account.res" );
 
 	// force recalculation of some stuff
 	m_iHUDColor = -1;
 	m_flBackgroundAlpha = 0.0f;
 	m_iStyle = -1;
 	m_iAccount = -1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAccount::Init()
{
	m_iAccount = -1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAccount::OnThink()
{
	if ( m_flBackgroundAlpha != cl_hud_background_alpha.GetFloat() )
	{
		Color newColor = GetBgColor();
 		newColor[3] = cl_hud_background_alpha.GetFloat() * 255;
		SetBgColor( newColor );
	}

	if ( m_iHUDColor != cl_hud_color.GetInt() )
	{
		m_iHUDColor = cl_hud_color.GetInt();
		Color clr = gHUD.GetHUDColor( m_iHUDColor );

		m_pBuyZoneIcon->SetFgColor( clr );
		m_pAccountLabel->SetFgColor( clr );
	}

	int realAccount = 0;
	C_CSPlayer *pPlayer = GetHudPlayer();
	if ( !pPlayer )
	{
		SetPaintEnabled( false );
		SetPaintBackgroundEnabled( false );
		return;
	}

	SetPaintEnabled( true );
	SetPaintBackgroundEnabled( true );
	
	// Never below zero
	realAccount = MAX( pPlayer->GetAccount(), 0 );

	if ( realAccount != m_iAccount )
	{
		m_iAccount = realAccount;
		const char *szMoneyText = UTIL_VarArgs( "$%d", realAccount );
		m_pAccountLabel->SetText( szMoneyText );
		m_pAccountLabel->WideToContents();

		SetWide( m_pAccountLabel->GetXPos() + m_pAccountLabel->GetWide() + margin_right );
	}

	m_pBuyZoneIcon->SetVisible( m_pBuyZoneIcon && pPlayer->IsInBuyZone() && !CSGameRules()->IsBuyTimeElapsed() );
}

bool CHudAccount::ShouldDraw()
{
	C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return false;

	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	if ( mp_maxmoney.GetInt() <= 0 )
		return false;

	if ( !CSGameRules() )
		return false;

	if ( CSGameRules()->m_bCTCantBuy && (pPlayer->GetTeamNumber() == TEAM_CT) )
		return false;

	if ( CSGameRules()->m_bTCantBuy && (pPlayer->GetTeamNumber() == TEAM_TERRORIST) )
		return false;

	return CHudElement::ShouldDraw();
}
