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
#include "hud_macros.h"
#include "text_message.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar		scr_centertime( "scr_centertime", "4" );

//-----------------------------------------------------------------------------
// Purpose: Implements Center String printing
//-----------------------------------------------------------------------------
class CNotificationPanel : public vgui::EditablePanel, public CGameEventListener
{
	DECLARE_CLASS_SIMPLE( CNotificationPanel, vgui::EditablePanel );

public:
						CNotificationPanel( vgui::VPANEL parent );
	virtual				~CNotificationPanel( void );

	// vgui::Panel
	virtual void		ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void		OnTick( void );
	virtual void		Paint();

	// CGameEventListener
	virtual void		FireGameEvent( IGameEvent * event );

	// CVGuiCenterPrint
	virtual void		SetTextColor( int r, int g, int b, int a );
	virtual void		Print( char *text );
	virtual void		Print( wchar_t *text );
	virtual void		ColorPrint( int r, int g, int b, int a, char *text );
	virtual void		ColorPrint( int r, int g, int b, int a, wchar_t *text );
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

	int						m_iOrigXPos;
	int						m_iOrigYPos;
	float					m_flCentertimeOff;
	float					m_flCentertimeOn;
	bool					m_bIsDrawing;
	bool					m_bIsFirstDraw;
};

static void __MsgFunc_HintText( bf_read &msg )
{
	// Read the string(s)
	char szString[255];
	msg.ReadString( szString, sizeof( szString ) );

	internalCenterPrint->SetIsAlert( false ); // hint equals an info message
	internalCenterPrint->HintPrint( hudtextmessage->LookupString( szString, NULL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *parent - 
//-----------------------------------------------------------------------------
CNotificationPanel::CNotificationPanel( vgui::VPANEL parent ) : 
	BaseClass( NULL, "NotificationPanel" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	SetCursor( null );
	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );
	SetScheme( "ClientScheme" );

	m_flCentertimeOff = m_flCentertimeOn = 0.0;

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );

	m_iOrigXPos = m_iOrigYPos = 0;
	m_bIsAlert = true;
	m_bIsDrawing = false;
	m_bIsFirstDraw = true;

	m_pTextLabel = new vgui::Label( this, "NotificationText", " " );
	m_pAlertLabel = new vgui::Label( this, "AlertTitleLabel", "#UI_Alert" );
	m_pAlertIcon = new vgui::VectorImagePanel( this, "AlertIcon" );
	m_pInfoIcon = new vgui::VectorImagePanel( this, "InfoIcon" );
	m_pBorderBottom = new vgui::ImagePanel( this, "BorderBottom" );

	LoadControlSettings( "resource/hud/notificationpanel.res" );

	ComputeSize();

	ListenForGameEvent( "player_hintmessage" );

	HOOK_MESSAGE( HintText );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNotificationPanel::~CNotificationPanel( void )
{
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

void CNotificationPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	// Use a large font
	m_pAlertLabel->SetFont( pScheme->GetFont( "NotificationTitleFont" ) );
	m_pTextLabel->SetFont( pScheme->GetFont( "NotificationTextFont" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : r - 
//			g - 
//			b - 
//			a - 
//-----------------------------------------------------------------------------
void CNotificationPanel::SetTextColor( int r, int g, int b, int a )
{
	m_pTextLabel->SetFgColor( Color( r, g, b, a ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::Print( char *text )
{
	m_pTextLabel->SetText( text );
	SetAlertVisibility( m_bIsAlert );

	m_bIsDrawing = true;
	
	m_flCentertimeOff = scr_centertime.GetFloat() + gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::Print( wchar_t *text )
{
	m_pTextLabel->SetText( text );
	SetAlertVisibility( m_bIsAlert );

	m_bIsDrawing = true;
	
	m_flCentertimeOff = scr_centertime.GetFloat() + gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::ColorPrint( int r, int g, int b, int a, char *text )
{
	SetTextColor( r, g, b, a );
	Print( text );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::ColorPrint( int r, int g, int b, int a, wchar_t *text )
{
	SetTextColor( r, g, b, a );
	Print( text );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::Clear( void )
{
	m_flCentertimeOff = 0;
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NotificationHide" );
	m_bIsDrawing = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::OnTick( void )
{
	bool bVisibility = !engine->IsDrawingLoadingImage() && m_bIsDrawing;
	if ( bVisibility != IsVisible() )
	{
		SetVisible( bVisibility );
		if ( bVisibility )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NotificationShow" );
		}
	}

	if ( m_bIsDrawing )
	{
		if ( m_flCentertimeOff <= gpGlobals->curtime )
		{
			m_bIsDrawing = false;
			m_bIsFirstDraw = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NotificationHide" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNotificationPanel::Paint()
{
	if ( m_bIsDrawing )
		ComputeSize();

	BaseClass::Paint();
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
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
CCenterPrint::CCenterPrint( void )
{
	vguiNotificationPanel = NULL;
}

void CCenterPrint::SetTextColor( int r, int g, int b, int a )
{
	if ( vguiNotificationPanel )
	{
		vguiNotificationPanel->SetTextColor( r, g, b, a );
	}
}

void CCenterPrint::Print( char *text )
{
	if ( vguiNotificationPanel )
	{
		SetIsAlert( true ); // center print equals an alert
		vguiNotificationPanel->ColorPrint( 255, 255, 255, 255, text );
	}
}

void CCenterPrint::Print( wchar_t *text )
{
	if ( vguiNotificationPanel )
	{
		SetIsAlert( true ); // center print equals an alert
		vguiNotificationPanel->ColorPrint( 255, 255, 255, 255, text );
	}
}

void CCenterPrint::ColorPrint( int r, int g, int b, int a, char *text )
{
	if ( vguiNotificationPanel )
	{
		SetIsAlert( true ); // center print equals an alert
		vguiNotificationPanel->ColorPrint( r, g, b, a, text );
	}
}

void CCenterPrint::ColorPrint( int r, int g, int b, int a, wchar_t *text )
{
	if ( vguiNotificationPanel )
	{
		SetIsAlert( true ); // center print equals an alert
		vguiNotificationPanel->ColorPrint( r, g, b, a, text );
	}
}

void CCenterPrint::HintPrint( char *text )
{
	if ( vguiNotificationPanel )
	{
		SetIsAlert( false );
		vguiNotificationPanel->ColorPrint( 255, 255, 255, 255, text );
	}
}

void CCenterPrint::HintPrint( wchar_t *text )
{
	if ( vguiNotificationPanel )
	{
		SetIsAlert( false );
		vguiNotificationPanel->ColorPrint( 255, 255, 255, 255, text );
	}
}

void CCenterPrint::Clear( void )
{
	if ( vguiNotificationPanel )
	{
		vguiNotificationPanel->Clear();
	}
}

void CCenterPrint::Create( vgui::VPANEL parent )
{
	if ( vguiNotificationPanel )
	{
		Destroy();
	}

	vguiNotificationPanel = new CNotificationPanel( parent );
}

void CCenterPrint::Destroy( void )
{
	if ( vguiNotificationPanel )
	{
		vguiNotificationPanel->SetParent( (vgui::Panel *)NULL );
		delete vguiNotificationPanel;
		vguiNotificationPanel = NULL;
	}
}

void CCenterPrint::SetIsAlert( bool bState )
{
	if ( vguiNotificationPanel )
		vguiNotificationPanel->m_bIsAlert = bState;
}

static CCenterPrint g_CenterString;
CCenterPrint *internalCenterPrint = &g_CenterString;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CCenterPrint, ICenterPrint, VCENTERPRINT_INTERFACE_VERSION, g_CenterString );