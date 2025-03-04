//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>
#include <globalvars_base.h>
#include <cdll_util.h>
#include <KeyValues.h>

#include "spectatorgui.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IPanel.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>
#include <vgui_controls/TextImage.h>

#include <stdio.h> // _snprintf define

#include <game/client/iviewport.h>
#include "commandmenu.h"
#include "hltvcamera.h"
#if defined( REPLAY_ENABLED )
#include "replay/replaycamera.h"
#endif

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Menu.h>
#include "IGameUIFuncs.h" // for key bindings
#include <imapoverview.h>
#include <shareddefs.h>
#include <igameresources.h>

#ifdef TF_CLIENT_DLL
#include "tf_gamerules.h"
void AddSubKeyNamed( KeyValues *pKeys, const char *pszName );
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef _XBOX
extern IGameUIFuncs *gameuifuncs; // for key binding details
#endif

// void DuckMessage(const char *str); // from vgui_teamfortressviewport.cpp

ConVar spec_scoreboard( "spec_scoreboard", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );

CSpectatorGUI *g_pSpectatorGUI = NULL;


// NB disconnect between localization text and observer mode enums
static const char *s_SpectatorModes[] =
{
	"#Spec_Mode0",	// 	OBS_MODE_NONE = 0,	
	"#Spec_Mode1",	// 	OBS_MODE_DEATHCAM,	
	"",				// 	OBS_MODE_FREEZECAM,	
	"#Spec_Mode2",	// 	OBS_MODE_FIXED,		
	"#Spec_Mode3",	// 	OBS_MODE_IN_EYE,	
	"#Spec_Mode4",	// 	OBS_MODE_CHASE,		
	"#Spec_Mode_POI",	// 	OBS_MODE_POI, PASSTIME
	"#Spec_Mode5",	// 	OBS_MODE_ROAMING,	
};

using namespace vgui;

ConVar cl_spec_mode(
	"cl_spec_mode",
	"1",
	FCVAR_ARCHIVE | FCVAR_USERINFO | FCVAR_SERVER_CAN_EXECUTE,
	"spectator mode" );

//-----------------------------------------------------------------------------
// main spectator panel



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSpectatorGUI::CSpectatorGUI(IViewPort *pViewPort) : EditablePanel( NULL, PANEL_SPECGUI )
{
// 	m_bHelpShown = false;
//	m_bInsetVisible = false;
//	m_iDuckKey = KEY_NONE;
	SetSize( 10, 10 ); // Quiet "parent not sized yet" spew
	m_bSpecScoreboard = false;

	m_pViewPort = pViewPort;
	g_pSpectatorGUI = this;

	// initialize dialog
	SetVisible(false);
	SetProportional(true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );

	m_pTopBar = new Panel( this, "topbar" );
 	m_pBottomBarBlank = new Panel( this, "bottombarblank" );

	// m_pBannerImage = new ImagePanel( m_pTopBar, NULL );
	m_pPlayerLabel = new Label( this, "playerlabel", "" );
	m_pPlayerLabel->SetVisible( false );
	TextImage *image = m_pPlayerLabel->GetTextImage();
	if ( image )
	{
		HFont hFallbackFont = scheme()->GetIScheme( GetScheme() )->GetFont( "DefaultVerySmallFallBack", false );
		if ( INVALID_FONT != hFallbackFont )
		{
			image->SetUseFallbackFont( true, hFallbackFont );
		}
	}

	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);

	// m_pBannerImage->SetVisible(false);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CSpectatorGUI::~CSpectatorGUI()
{
	g_pSpectatorGUI = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the colour of the top and bottom bars
//-----------------------------------------------------------------------------
void CSpectatorGUI::ApplySchemeSettings(IScheme *pScheme)
{
	KeyValues *pConditions = NULL;

#ifdef TF_CLIENT_DLL
	if ( TFGameRules() && TFGameRules()->IsMannVsMachineMode() )
	{
		pConditions = new KeyValues( "conditions" );
		AddSubKeyNamed( pConditions, "if_mvm" );
	}
#endif

	LoadControlSettings( GetResFile(), NULL, NULL, pConditions );

	if ( pConditions )
	{
		pConditions->deleteThis();
	}

	m_pBottomBarBlank->SetVisible( true );
	m_pTopBar->SetVisible( true );

	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor(Color( 0,0,0,0 ) ); // make the background transparent
	m_pTopBar->SetBgColor(GetBlackBarColor());
	m_pBottomBarBlank->SetBgColor(GetBlackBarColor());
	// m_pBottomBar->SetBgColor(Color( 0,0,0,0 ));
	SetPaintBorderEnabled(false);

	SetBorder( NULL );

#ifdef CSTRIKE_DLL
	SetZPos(80);	// guarantee it shows above the scope
#endif
}

//-----------------------------------------------------------------------------
// Purpose: makes the GUI fill the screen
//-----------------------------------------------------------------------------
void CSpectatorGUI::PerformLayout()
{
	int w,h,x,y;
	GetHudSize(w, h);
	
	// fill the screen
	SetBounds(0,0,w,h);

	// stretch the bottom bar across the screen
	m_pBottomBarBlank->GetPos(x,y);
	m_pBottomBarBlank->SetSize( w, h - y );
}

//-----------------------------------------------------------------------------
// Purpose: checks spec_scoreboard cvar to see if the scoreboard should be displayed
//-----------------------------------------------------------------------------
void CSpectatorGUI::OnThink()
{
	BaseClass::OnThink();

	if ( IsVisible() )
	{
		if ( m_bSpecScoreboard != spec_scoreboard.GetBool() )
		{
			if ( !spec_scoreboard.GetBool() || !gViewPortInterface->GetActivePanel() )
			{
				m_bSpecScoreboard = spec_scoreboard.GetBool();
				gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, m_bSpecScoreboard );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets the image to display for the banner in the top right corner
//-----------------------------------------------------------------------------
void CSpectatorGUI::SetLogoImage(const char *image)
{
	if ( m_pBannerImage )
	{
		m_pBannerImage->SetImage( scheme()->GetImage(image, false) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CSpectatorGUI::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CSpectatorGUI::SetLabelText(const char *textEntryName, wchar_t *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CSpectatorGUI::MoveLabelToFront(const char *textEntryName)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->MoveToFront();
	}
}

//-----------------------------------------------------------------------------
// Purpose: shows/hides the buy menu
//-----------------------------------------------------------------------------
void CSpectatorGUI::ShowPanel(bool bShow)
{
	if ( bShow && !IsVisible() )
	{
		m_bSpecScoreboard = false;
	}
	SetVisible( bShow );
	if ( !bShow && m_bSpecScoreboard )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, false );
	}
}

bool CSpectatorGUI::ShouldShowPlayerLabel( int specmode )
{
	return ( (specmode == OBS_MODE_IN_EYE) ||	(specmode == OBS_MODE_CHASE) );
}
//-----------------------------------------------------------------------------
// Purpose: Updates the gui, rearranges elements
//-----------------------------------------------------------------------------
void CSpectatorGUI::Update()
{
	int wide, tall;
	int bx, by, bwide, btall;

	GetHudSize(wide, tall);
	m_pTopBar->GetBounds( bx, by, bwide, btall );

	IGameResources *gr = GameResources();
	int specmode = GetSpectatorMode();
	int playernum = GetSpectatorTarget();

	IViewPortPanel *overview = gViewPortInterface->FindPanelByName( PANEL_OVERVIEW );

	if ( overview && overview->IsVisible() )
	{
		int mx, my, mwide, mtall;

		VPANEL p = overview->GetVPanel();
		vgui::ipanel()->GetPos( p, mx, my );
		vgui::ipanel()->GetSize( p, mwide, mtall );
				
		if ( my < btall )
		{
			// reduce to bar 
			m_pTopBar->SetSize( wide - (mx + mwide), btall );
			m_pTopBar->SetPos( (mx + mwide), 0 );
		}
		else
		{
			// full top bar
			m_pTopBar->SetSize( wide , btall );
			m_pTopBar->SetPos( 0, 0 );
		}
	}
	else
	{
		// full top bar
		m_pTopBar->SetSize( wide , btall ); // change width, keep height
		m_pTopBar->SetPos( 0, 0 );
	}

	m_pPlayerLabel->SetVisible( ShouldShowPlayerLabel(specmode) );

	// update player name filed, text & color

	if ( playernum > 0 && playernum <= gpGlobals->maxClients && gr )
	{
		Color c = gr->GetTeamColor( gr->GetTeam(playernum) ); // Player's team color

		m_pPlayerLabel->SetFgColor( c );
		
		wchar_t playerText[ 80 ], playerName[ 64 ], health[ 10 ];
		V_wcsncpy( playerText, L"Unable to find #Spec_PlayerItem*", sizeof( playerText ) );
		memset( playerName, 0x0, sizeof( playerName ) );

		g_pVGuiLocalize->ConvertANSIToUnicode( UTIL_SafeName(gr->GetPlayerName( playernum )), playerName, sizeof( playerName ) );
		int iHealth = gr->GetHealth( playernum );
		if ( iHealth > 0  && gr->IsAlive(playernum) )
		{
			_snwprintf( health, ARRAYSIZE( health ), L"%i", iHealth );
			g_pVGuiLocalize->ConstructString( playerText, sizeof( playerText ), g_pVGuiLocalize->Find( "#Spec_PlayerItem_Team" ), 2, playerName,  health );
		}
		else
		{
			g_pVGuiLocalize->ConstructString( playerText, sizeof( playerText ), g_pVGuiLocalize->Find( "#Spec_PlayerItem" ), 1, playerName );
		}

		m_pPlayerLabel->SetText( playerText );
	}
	else
	{
		m_pPlayerLabel->SetText( L"" );
	}

	// update extra info field
	wchar_t szEtxraInfo[1024];
	wchar_t szTitleLabel[1024];
	char tempstr[128];

	if ( engine->IsHLTV() )
	{
		// set spectator number and HLTV title
		Q_snprintf(tempstr,sizeof(tempstr),"Spectators : %d", HLTVCamera()->GetNumSpectators() );
		g_pVGuiLocalize->ConvertANSIToUnicode(tempstr,szEtxraInfo,sizeof(szEtxraInfo));
		
		Q_strncpy( tempstr, HLTVCamera()->GetTitleText(), sizeof(tempstr) );
		g_pVGuiLocalize->ConvertANSIToUnicode(tempstr,szTitleLabel,sizeof(szTitleLabel));
	}
	else
	{
		// otherwise show map name
		Q_FileBase( engine->GetLevelName(), tempstr, sizeof(tempstr) );

		wchar_t wMapName[64];
		g_pVGuiLocalize->ConvertANSIToUnicode(tempstr,wMapName,sizeof(wMapName));
		g_pVGuiLocalize->ConstructString( szEtxraInfo,sizeof( szEtxraInfo ), g_pVGuiLocalize->Find("#Spec_Map" ),1, wMapName );

		g_pVGuiLocalize->ConvertANSIToUnicode( "" ,szTitleLabel,sizeof(szTitleLabel));
	}

	SetLabelText("extrainfo", szEtxraInfo );
	SetLabelText("titlelabel", szTitleLabel );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the timer label if one exists
//-----------------------------------------------------------------------------
void CSpectatorGUI::UpdateTimer()
{
	wchar_t szText[ 63 ];

	int timer = 0;

	V_swprintf_safe ( szText, L"%d:%02d\n", (timer / 60), (timer % 60) );

	SetLabelText("timerlabel", szText );
}

static void ForwardSpecCmdToServer( const CCommand &args )
{
	if ( engine->IsPlayingDemo() )
		return;

	if ( args.ArgC() == 1 )
	{
		// just forward the command without parameters
		engine->ServerCmd( args[ 0 ] );
	}
	else if ( args.ArgC() == 2 )
	{
		// forward the command with parameter
		char command[128];
		Q_snprintf( command, sizeof(command), "%s \"%s\"", args[ 0 ], args[ 1 ] );
		engine->ServerCmd( command );
	}
}

CON_COMMAND_F( spec_next, "Spectate next player", FCVAR_CLIENTCMD_CAN_EXECUTE )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() )
	{
		// handle the command clientside
		if ( !HLTVCamera()->IsPVSLocked() )
		{
			HLTVCamera()->SpecNextPlayer( false );
		}
	}
	else
	{
		ForwardSpecCmdToServer( args );
	}
}

CON_COMMAND_F( spec_prev, "Spectate previous player", FCVAR_CLIENTCMD_CAN_EXECUTE )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() )
	{
		// handle the command clientside
		if ( !HLTVCamera()->IsPVSLocked() )
		{
			HLTVCamera()->SpecNextPlayer( true );
		}
	}
	else
	{
		ForwardSpecCmdToServer( args );
	}
}

CON_COMMAND_F( spec_mode, "Set spectator mode", FCVAR_CLIENTCMD_CAN_EXECUTE )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() )
	{
		if ( HLTVCamera()->IsPVSLocked() )
		{
			// in locked mode we can only switch between first and 3rd person
			HLTVCamera()->ToggleChaseAsFirstPerson();
		}
		else
		{
			// we can choose any mode, not loked to PVS
			int mode;

			if ( args.ArgC() == 2 )
			{
				// set specifc mode
				mode = Q_atoi( args[1] );
			}
			else
			{
				// set next mode 
				mode = HLTVCamera()->GetMode()+1;

				if ( mode > LAST_PLAYER_OBSERVERMODE )
					mode = OBS_MODE_IN_EYE;
				else if ( mode == OBS_MODE_POI ) // PASSTIME skip POI mode since hltv doesn't have the entity data required to make it work
					mode = OBS_MODE_ROAMING;
			}
			
			// handle the command clientside
			HLTVCamera()->SetMode( mode );
		}

			// turn off auto director once user tried to change view settings
		HLTVCamera()->SetAutoDirector( false );
	}
	else
	{
		// we spectate on a game server, forward command
		ForwardSpecCmdToServer( args );
	}
}

CON_COMMAND_F( spec_player, "Spectate player by name", FCVAR_CLIENTCMD_CAN_EXECUTE )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( args.ArgC() != 2 )
		return;

	if ( engine->IsHLTV() )
	{
		// we can only switch primary spectator targets is PVS isnt locked by auto-director
		if ( !HLTVCamera()->IsPVSLocked() )
		{
			HLTVCamera()->SpecNamedPlayer( args[1] );
		}
	}
	else
	{
		ForwardSpecCmdToServer( args );
	}
}



