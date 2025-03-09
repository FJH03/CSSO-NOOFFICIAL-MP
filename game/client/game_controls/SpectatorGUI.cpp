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

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSpectatorGUI::CSpectatorGUI(IViewPort *pViewPort) : EditablePanel( NULL, PANEL_SPECGUI )
{
	m_bSpecScoreboard = false;

	g_pSpectatorGUI = this;

	// initialize dialog
	SetVisible(false);
	SetProportional(true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );

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
	LoadControlSettings( GetResFile() );

	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor(Color( 0,0,0,0 ) ); // make the background transparent
	// m_pBottomBar->SetBgColor(Color( 0,0,0,0 ));
	SetPaintBorderEnabled(false);

	SetBorder( NULL );

#ifdef CSTRIKE_DLL
	SetZPos(80);	// guarantee it shows above the scope
#endif
}

void CSpectatorGUI::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
 	// reload the .res file so items are rescaled
 	LoadControlSettings( GetResFile() );
}
 
//-----------------------------------------------------------------------------
// Purpose: makes the GUI fill the screen
//-----------------------------------------------------------------------------
void CSpectatorGUI::PerformLayout()
{
	int w,h;
	GetHudSize(w, h);
	
	// fill the screen
	SetBounds(0,0,w,h);
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
		InvalidateLayout( true, true );
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
		// XXX(JohnS): Whyyyyy
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


