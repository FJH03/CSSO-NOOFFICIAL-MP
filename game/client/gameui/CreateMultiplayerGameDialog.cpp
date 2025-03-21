//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "CreateMultiplayerGameDialog.h"
#include "CreateMultiplayerGameServerPage.h"
#include "CreateMultiplayerGameGameplayPage.h"
#include "CreateMultiplayerGameBotPage.h"

#include "EngineInterface.h"
#include "ModInfo.h"
#include "GameUI_Interface.h"

#include <stdio.h>

using namespace vgui;

#include "vgui_controls/ComboBox.h"
#include <vgui/ILocalize.h>

#include "filesystem.h"
#include <KeyValues.h>

#include "gametypes.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCreateMultiplayerGameDialog::CCreateMultiplayerGameDialog(vgui::Panel *parent) : PropertyDialog(parent, "CreateMultiplayerGameDialog")
{
	SetDeleteSelfOnClose(true);

	int w = 424;
	int h = 460;
	if (IsProportional())
	{
		w = scheme()->GetProportionalScaledValueEx(GetScheme(), w);
		h = scheme()->GetProportionalScaledValueEx(GetScheme(), h);
	}

	SetSize(w, h);
	
	SetTitle("#GameUI_CreateServer", true);
	SetOKButtonText("#GameUI_Start");

	// create KeyValues object to load/save config options
	m_pSavedData = new KeyValues( "ServerConfig" );
	
	int nGameType = 0;
	int nGameMode = 0;
	bool bAllMaps = false;
	// load the config data
	if (m_pSavedData)
	{
		m_pSavedData->LoadFromFile( g_pFullFileSystem, "ServerConfig.vdf", "GAME" ); // this is game-specific data, so it should live in GAME, not CONFIG
		
		nGameType = m_pSavedData->GetInt( "game_type" );
		nGameMode = m_pSavedData->GetInt( "game_mode" );
		bAllMaps = m_pSavedData->GetBool( "all_maps" );
	}

	m_pServerPage = new CCreateMultiplayerGameServerPage(this, "ServerPage", nGameType, nGameMode, bAllMaps);

	m_pGameplayPage = new CCreateMultiplayerGameGameplayPage(this, "GameplayPage");
	m_pBotPage = NULL;

	AddPage(m_pServerPage, "#GameUI_Server");
	AddPage(m_pGameplayPage, "#GameUI_Game");

	m_pServerPage->UpdateGameplayPage(); // do it AFTER m_pGameplayPage has been added

	if ( m_pSavedData )
	{
		const char *startMap = m_pSavedData->GetString("map", "");
		if (startMap[0])
		{
			m_pServerPage->SetMap(startMap);
		}
		const char *hostname = m_pSavedData->GetString("hostname", "");
		if (hostname[0])
		{
			m_pServerPage->SetHostName(hostname);
		}
		const char *maxplayers = m_pSavedData->GetString("maxplayers", "");
		if (maxplayers[0])
		{
			m_pServerPage->SetMaxPlayers(maxplayers);
		}
		const char *sv_password = m_pSavedData->GetString("sv_password", "");
		if (sv_password[0])
		{
			m_pServerPage->SetPassword(sv_password);
		}
	}

	if ( ModInfo().UseBots() )
	{
		// add a page of advanced bot controls
		// NOTE: These controls will use the bot keys to initialize their values
		m_pBotPage = new CCreateMultiplayerGameBotPage( this, "BotPage", m_pSavedData );
		AddPage( m_pBotPage, "#GameUI_CPUPlayerOptions" );
		m_pServerPage->EnableBots( m_pSavedData );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CCreateMultiplayerGameDialog::~CCreateMultiplayerGameDialog()
{
	if (m_pSavedData)
	{
		m_pSavedData->deleteThis();
		m_pSavedData = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: runs the server when the OK button is pressed
//-----------------------------------------------------------------------------
bool CCreateMultiplayerGameDialog::OnOK(bool applyOnly)
{
	// reset server enforced cvars
	g_pCVar->RevertFlaggedConVars( FCVAR_REPLICATED );	

	// Cheats were disabled; revert all cheat cvars to their default values.
	// This must be done heading into multiplayer games because people can play
	// demos etc and set cheat cvars with sv_cheats 0.
	g_pCVar->RevertFlaggedConVars( FCVAR_CHEAT );

	DevMsg( "FCVAR_CHEAT cvars reverted to defaults.\n" );

	BaseClass::OnOK(applyOnly);

	// get these values from m_pServerPage and store them temporarily
	char szMapName[64], szHostName[64], szPassword[64];
	int iGameTypeID = m_pServerPage->GetGameTypeID();
	int iGameModeID = m_pServerPage->GetGameModeID();
	int iMaxPlayers = m_pServerPage->GetMaxPlayers();
	Q_strncpy(szMapName, m_pServerPage->GetMapName(), sizeof( szMapName ));
	Q_strncpy(szHostName, m_pServerPage->GetHostName(), sizeof( szHostName ));
	Q_strncpy(szPassword, m_pServerPage->GetPassword(), sizeof( szPassword ));

	// save the config data
	if (m_pSavedData)
	{
		if (m_pServerPage->IsRandomMapSelected())
		{
			// it's set to random map, just save an
			m_pSavedData->SetString("map", "");
		}
		else
		{
			m_pSavedData->SetString("map", szMapName);
		}

		m_pSavedData->SetInt( "game_type", iGameTypeID );
		m_pSavedData->SetInt( "game_mode", iGameModeID );
		m_pSavedData->SetBool( "all_maps", m_pServerPage->IsAllMaps() );
		m_pSavedData->SetString( "hostname", szHostName );
		m_pSavedData->SetInt( "maxplayers", iMaxPlayers );
		m_pSavedData->SetString( "sv_password", szPassword );

		// save config to a file
		m_pSavedData->SaveToFile( g_pFullFileSystem, "ServerConfig.vdf", "GAME" );
	}

	char szMapCommand[1024];

	// create the command to execute
	Q_snprintf(szMapCommand, sizeof( szMapCommand ), "disconnect\nwait\nwait\nsv_lan 1\nsetmaster enable\nmaxplayers %i\nsv_password \"%s\"\nhostname \"%s\"\nprogress_enable\ngame_type %d\ngame_mode %d\ngame_online 0\nmap %s\n",
		iMaxPlayers,
		szPassword,
		szHostName,
		iGameTypeID,
		iGameModeID,
		szMapName
	);

	// exec
	engine->ClientCmd_Unrestricted(szMapCommand);

	return true;
}

void CCreateMultiplayerGameDialog::OnKeyCodePressed( vgui::KeyCode code )
{
	// Handle close here, CBasePanel parent doesn't support "DialogClosing" command
	ButtonCode_t nButtonCode = GetBaseButtonCode( code );

	if ( nButtonCode == KEY_XBUTTON_B )
	{
		OnCommand( "Close" );
	}
	else if ( nButtonCode == KEY_XBUTTON_A || nButtonCode == STEAMCONTROLLER_A )
	{
		OnOK( false );
	}
	else if ( nButtonCode == KEY_XBUTTON_UP || 
			  nButtonCode == KEY_XSTICK1_UP ||
			  nButtonCode == KEY_XSTICK2_UP ||
			  nButtonCode == STEAMCONTROLLER_DPAD_UP ||
			  nButtonCode == KEY_UP )
	{
		int nItem = m_pServerPage->GetMapList()->GetSelectedItem(0) - 1;
		if ( nItem < 0 )
		{
			nItem = m_pServerPage->GetMapList()->GetItemCount() - 1;
		}
		m_pServerPage->GetMapList()->SetSingleSelectedItem( nItem );
	}
	else if ( nButtonCode == KEY_XBUTTON_DOWN || 
			  nButtonCode == KEY_XSTICK1_DOWN ||
			  nButtonCode == KEY_XSTICK2_DOWN || 
			  nButtonCode == STEAMCONTROLLER_DPAD_DOWN ||
			  nButtonCode == KEY_DOWN )
	{
		int nItem = m_pServerPage->GetMapList()->GetSelectedItem(0) + 1;
		if ( nItem >= m_pServerPage->GetMapList()->GetItemCount() )
		{
			nItem = 0;
		}
		m_pServerPage->GetMapList()->SetSingleSelectedItem( nItem );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}