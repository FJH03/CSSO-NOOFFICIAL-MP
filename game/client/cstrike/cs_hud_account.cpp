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
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/VectorImagePanel.h>

using namespace vgui;

#include "hudelement.h"
#include "c_cs_player.h"

#include "convar.h"

extern ConVar cl_hud_background_alpha;
extern ConVar mp_maxmoney;

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

private:
	float	m_flBackgroundAlpha;

	int		m_iAccount;

	VectorImagePanel	*m_pBuyZoneIcon;
	Label				*m_pAccountLabel;

	CPanelAnimationVarAliasType( int, buyzone_icon_xpos, "buyzone_icon_xpos", "0", "proportional_xpos" );
	CPanelAnimationVarAliasType( int, buyzone_icon_ypos, "buyzone_icon_ypos", "0", "proportional_ypos" );

	CPanelAnimationVar( Color, m_clrBuyZoneIconFg, "BuyZoneIconFg", "FgColor" );
	CPanelAnimationVar( int, margin_right, "margin_right", "0" );
	CPanelAnimationVar( int, buyzone_icon_margin_right, "buyzone_icon_margin_right", "0" );
};

DECLARE_HUDELEMENT( CHudAccount );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAccount::CHudAccount( const char *pElementName ) : CHudElement( pElementName ), EditablePanel(NULL, "HudAccount")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_pAccountLabel = new Label( this, "AccountLabel", "" );
	m_pBuyZoneIcon = new VectorImagePanel( this, "BuyZoneIcon" );

	LoadControlSettings( "resource/hud/account.res" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAccount::Init()
{
	m_iAccount			= -1;
	m_flBackgroundAlpha = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAccount::OnThink()
{
	if ( m_flBackgroundAlpha != cl_hud_background_alpha.GetFloat() )
	{
		Color oldColor = GetBgColor();
		Color newColor( oldColor.r(), oldColor.g(), oldColor.b(), cl_hud_background_alpha.GetFloat() * 255 );
		SetBgColor( newColor );
	}

	int realAccount = 0;
	C_CSPlayer *local = C_CSPlayer::GetLocalCSPlayer();
	if ( !local )
		return;

	// Never below zero
	realAccount = MAX( local->GetAccount(), 0 );

	if ( realAccount != m_iAccount )
	{
		m_iAccount = realAccount;
		const char *szMoneyText = UTIL_VarArgs( "$%d", realAccount );
		m_pAccountLabel->SetText( szMoneyText );
		m_pAccountLabel->WideToContents();

		SetWide( m_pAccountLabel->GetXPos() + m_pAccountLabel->GetWide() + margin_right );

		m_pBuyZoneIcon->SetPos( m_pAccountLabel->GetXPos() + m_pAccountLabel->GetWide() + buyzone_icon_margin_right, m_pBuyZoneIcon->GetYPos() );
	}

	m_pBuyZoneIcon->SetVisible( m_pBuyZoneIcon && local->IsInBuyZone() && !CSGameRules()->IsBuyTimeElapsed() );
}

bool CHudAccount::ShouldDraw()
{
	C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
	if ( !pPlayer )
		return false;

	if ( pPlayer->IsObserver() )
		return false;

	if ( mp_maxmoney.GetInt() <= 0 )
		return false;

	return CHudElement::ShouldDraw();
}
