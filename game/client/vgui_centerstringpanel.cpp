//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <stdarg.h>
#include "vguicenterprint.h"
#include "ivrenderview.h"
#include <vgui/IVGui.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include "hudelement.h"
#include "hud_macros.h"
#include "text_message.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar		scr_centertime( "scr_centertime", "4" );
extern ConVar cl_draw_only_deathnotices;

static CNotificationPanel *g_NotificationPanel = NULL;

//-----------------------------------------------------------------------------
// Purpose: Implements Center String printing
//-----------------------------------------------------------------------------
class CNotificationPanel : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CNotificationPanel, vgui::EditablePanel );

public:
						CNotificationPanel( const char *pElementName );
	virtual				~CNotificationPanel( void );

	// vgui::Panel
	virtual void		OnThink( void );
	virtual void		OnScreenSizeChanged( int iOldWide, int iOldTall );
	virtual bool		ShouldDraw();

	// CGameEventListener
	virtual void		FireGameEvent( IGameEvent * event );

	virtual void		MsgFunc_HintText( bf_read &msg );

	// CVGuiCenterPrint
	virtual void		Print( char *text );
	virtual void		Print( wchar_t *text );
	virtual void		Clear( void );

	virtual void		SetAlertVisibility( bool bState );
	bool				m_bIsAlert;

private:
	void ComputeSize( void );

	vgui::Label				*m_pTextLabel;
	vgui::Label				*m_pAlertLabel;
	vgui::VectorImagePanel	*m_pAlertIcon;
	vgui::VectorImagePanel	*m_pInfoIcon;
	vgui::ImagePanel		*m_pBorderBottom;

	CPanelAnimationVarAliasType( int, alert_icon_margin, "alert_icon_margin", "0", "proportional_width" );
	CPanelAnimationVarAliasType( int, bottom_margin, "bottom_margin", "0", "proportional_height" );

	float					m_flCentertimeOff;
};

DECLARE_HUDELEMENT( CNotificationPanel );
DECLARE_HUD_MESSAGE( CNotificationPanel, HintText );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *parent - 
//-----------------------------------------------------------------------------
CNotificationPanel::CNotificationPanel( const char *pElementName ) : CHudElement( pElementName ), EditablePanel(NULL, "NotificationPanel")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_flCentertimeOff = 0.0;

	m_bIsAlert = true;

	m_pTextLabel = new vgui::Label( this, "NotificationText", " " );
	m_pAlertLabel = new vgui::Label( this, "AlertTitleLabel", "#UI_Alert" );
	m_pAlertIcon = new vgui::VectorImagePanel( this, "AlertIcon" );
	m_pInfoIcon = new vgui::VectorImagePanel( this, "InfoIcon" );
	m_pBorderBottom = new vgui::ImagePanel( this, "BorderBottom" );

	LoadControlSettings( "resource/hud/notificationpanel.res" );

	ComputeSize();

	ListenForGameEvent( "player_hintmessage" );
	HOOK_HUD_MESSAGE( CNotificationPanel, HintText );

	g_NotificationPanel = this;
}

void CNotificationPanel::MsgFunc_HintText( bf_read &msg )
{
	// Read the string(s)
	char szString[255];
	msg.ReadString( szString, sizeof( szString ) );

	m_bIsAlert = false; // hint equals an info message
	Print( hudtextmessage->LookupString( szString, NULL ) );
}

void CNotificationPanel::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	// reload the .res file so items are rescaled
	LoadControlSettings( "resource/hud/notificationpanel.res" );
}

bool CNotificationPanel::ShouldDraw()
{
	if ( cl_draw_only_deathnotices.GetBool() )
		return false;

	return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNotificationPanel::~CNotificationPanel( void )
{
	g_NotificationPanel = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::FireGameEvent( IGameEvent * event )
{
	internalCenterPrint->HintPrint( hudtextmessage->LookupString( event->GetString( "hintmessage" ), NULL ) );
}

//-----------------------------------------------------------------------------
// Purpose: Computes panel's desired size and position
//-----------------------------------------------------------------------------
void CNotificationPanel::ComputeSize( void )
{
	m_pTextLabel->TallToContents();
	int tall = m_pTextLabel->GetYPos() + m_pTextLabel->GetTall() + bottom_margin;
	SetTall( tall );

	m_pAlertLabel->WideToContents();
	int iAlertIconWide = m_pAlertIcon->GetWide();
	int iTotalWide = iAlertIconWide + alert_icon_margin + m_pAlertLabel->GetWide();
	int iXPos = (GetWide() / 2) - (iTotalWide / 2);
	m_pAlertIcon->SetPos( iXPos, m_pAlertIcon->GetYPos() );
	m_pAlertLabel->SetPos( iXPos + iAlertIconWide + alert_icon_margin, m_pAlertLabel->GetYPos() );
	m_pBorderBottom->SetPos( m_pBorderBottom->GetXPos(), tall - 1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::Print( char *text )
{
	m_pTextLabel->SetText( text );
	m_pTextLabel->InternalPerformLayout(); // layout it right now so that is resizes this frame
	SetAlertVisibility( m_bIsAlert );

	ComputeSize();
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NotificationShow" );
	
	m_flCentertimeOff = scr_centertime.GetFloat() + gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::Print( wchar_t *text )
{
	m_pTextLabel->SetText( text );
	m_pTextLabel->InternalPerformLayout(); // layout it right now so that is resizes this frame
	SetAlertVisibility( m_bIsAlert );

	ComputeSize();
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NotificationShow" );
	
	m_flCentertimeOff = scr_centertime.GetFloat() + gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::Clear( void )
{
	m_flCentertimeOff = 0;
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NotificationHide" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::OnThink( void )
{
	if ( GetAlpha() != 0 && m_flCentertimeOff <= gpGlobals->curtime )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NotificationHide" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::SetAlertVisibility( bool bState )
{
	m_pAlertIcon->SetVisible( bState );
	m_pAlertLabel->SetVisible( bState );
	m_pInfoIcon->SetVisible( !bState );
}


//-----------------------------------------------------------------------------
void CCenterPrint::Print( char *text )
{
	if ( g_NotificationPanel )
	{
		g_NotificationPanel->m_bIsAlert = true; // center print equals an alert
		g_NotificationPanel->Print( text );
	}
}

void CCenterPrint::Print( wchar_t *text )
{
	if ( g_NotificationPanel )
	{
		g_NotificationPanel->m_bIsAlert = true; // center print equals an alert
		g_NotificationPanel->Print( text );
	}
}

void CCenterPrint::HintPrint( char *text )
{
	if ( g_NotificationPanel )
	{
		g_NotificationPanel->m_bIsAlert = false;
		g_NotificationPanel->Print( text );
	}
}

void CCenterPrint::HintPrint( wchar_t *text )
{
	if ( g_NotificationPanel )
	{
		g_NotificationPanel->m_bIsAlert = false;
		g_NotificationPanel->Print( text );
	}
}

void CCenterPrint::Clear( void )
{
	if ( g_NotificationPanel )
	{
		g_NotificationPanel->Clear();
	}
}

static CCenterPrint g_CenterString;
CCenterPrint *internalCenterPrint = &g_CenterString;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CCenterPrint, ICenterPrint, VCENTERPRINT_INTERFACE_VERSION, g_CenterString );