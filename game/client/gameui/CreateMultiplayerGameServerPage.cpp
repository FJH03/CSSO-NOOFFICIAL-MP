//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "CreateMultiplayerGameServerPage.h"

using namespace vgui;

#include <KeyValues.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/CheckButton.h>
#include "filesystem.h"
#include "tier1/convar.h"
#include "EngineInterface.h"
#include "CvarToggleCheckButton.h"
#include "gametypes.h"

#include "ModInfo.h"

// for SRC
#include <vstdlib/random.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define RANDOM_MAP "#GameUI_RandomMap"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCreateMultiplayerGameServerPage::CCreateMultiplayerGameServerPage(vgui::Panel *parent, const char *name, int nGameType, int nGameMode, bool bAllMaps) : PropertyPage(parent, name)
{
	m_pSavedData = NULL;

	// we can use this if we decide we want to put "listen server" at the end of the game name
	m_pMapList = new ComboBox(this, "MapList", 12, false);

	m_pAllMapsCheck = new CheckButton( this, "AllMapsCheck", "" );
	m_pAllMapsCheck->SetSelected( bAllMaps );

	m_pGameTypeList = new ComboBox( this, "GameTypeList", 5, false );
	m_pGameModeList = new ComboBox( this, "GameModeList", 5, false );
	m_pBotSkillList = new ComboBox( this, "BotSkillList", 5, false );

	int iGameTypeCount = g_pGameTypes->GetGameTypesCount();
	for ( int i = 0; i < iGameTypeCount; i++ )
	{
		const char* pszGameTypeNameID = g_pGameTypes->GetGameTypeNameID( i );
		if ( pszGameTypeNameID )
			m_pGameTypeList->AddItem( pszGameTypeNameID, new KeyValues( "data", "game_type", i ) );
	}
	m_pGameTypeList->SetEnabled( iGameTypeCount > 1 );

	int iBotDifficultyCount = g_pGameTypes->GetCustomBotDifficultyCount();
	for ( int i = 0; i < iBotDifficultyCount; i++ )
	{
		const char* pszBotDifficultyNameID = g_pGameTypes->GetCustomBotDifficultyNameID( i );
		if ( pszBotDifficultyNameID )
			m_pBotSkillList->AddItem( pszBotDifficultyNameID, new KeyValues( "data", "custom_bot_difficulty", i ) );
	}
	m_pBotSkillList->SetEnabled( iBotDifficultyCount > 1 );

	LoadControlSettings("Resource/CreateMultiplayerGameServerPage.res");

	m_szMapName[0]  = 0;

	// initialize hostname
	SetControlString("ServerNameEdit", ModInfo().GetGameName());//szHostName);

	// initialize password
//	SetControlString("PasswordEdit", engine->pfnGetCvarString("sv_password"));
	ConVarRef var( "sv_password" );
	if ( var.IsValid() )
	{
		SetControlString("PasswordEdit", var.GetString() );
	}
	
	SetGameTypeID( nGameType );
	SetGameModeID( nGameMode );
	SetAllMaps( bAllMaps );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CCreateMultiplayerGameServerPage::~CCreateMultiplayerGameServerPage()
{
}

void CCreateMultiplayerGameServerPage::OnKeyCodePressed( vgui::KeyCode code )
{
	if ( code == KEY_XBUTTON_LEFT || code == KEY_XSTICK1_LEFT || code == KEY_XSTICK2_LEFT )
	{
		int nItem = m_pMapList->GetActiveItem();
		nItem -= 1;
		if ( nItem < 0 )
		{
			nItem = m_pMapList->GetItemCount() - 1;
		}

		m_pMapList->SilentActivateItem( nItem );
	}
	else if ( code == KEY_XBUTTON_RIGHT || code == KEY_XSTICK1_RIGHT || code == KEY_XSTICK2_RIGHT )
	{
		int nItem = m_pMapList->GetActiveItem();
		nItem += 1;
		if ( nItem >= m_pMapList->GetItemCount() )
		{
			nItem = 0;
		}

		m_pMapList->SilentActivateItem( nItem );
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
	}
}

void CCreateMultiplayerGameServerPage::OnThink()
{
	bool bReloadGameModes = false;
	bool bReloadMaps = false;
	if ( GetGameTypeID() != m_nGameTypeID )
		bReloadGameModes = true;
	if ( GetGameModeID() != m_nGameModeID || IsAllMaps() != m_bAllMaps )
		bReloadMaps = true;

	if ( bReloadGameModes )
	{
		m_nGameTypeID = GetGameTypeID();

		LoadGameModesList();
	}

	if ( bReloadGameModes || bReloadMaps )
	{
		m_nGameModeID = GetGameModeID();
		m_bAllMaps = IsAllMaps();

		LoadMapList();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::EnableBots( KeyValues *data )
{
	m_pSavedData = data;

	int quota = data->GetInt( "bot_quota", 0 );
	SetControlInt( "BotQuotaCombo", quota );

	int difficulty = data->GetInt( "custom_bot_difficulty", 0 );
	difficulty = max( difficulty, 0 );
	difficulty = min( g_pGameTypes->GetCustomBotDifficultyCount(), difficulty );

	m_pBotSkillList->ActivateItem( difficulty );
}

//-----------------------------------------------------------------------------
// Purpose: called to get the info from the dialog
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::OnApplyChanges()
{
	KeyValues *kv = m_pMapList->GetActiveItemUserData();
	Q_strncpy(m_szMapName, kv->GetString("mapname", ""), DATA_STR_LENGTH);

	if ( m_pSavedData )
	{
		int quota = GetControlInt( "BotQuotaCombo", 0 );
		m_pSavedData->SetInt( "bot_quota", quota );
		ConVarRef bot_quota( "bot_quota" );
		bot_quota.SetValue( quota );

		int difficulty = m_pBotSkillList->GetActiveItem();
		m_pSavedData->SetInt( "custom_bot_difficulty", difficulty );
		g_pGameTypes->SetCustomBotDifficulty( difficulty );
	}
}

//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::LoadMaps( const char *pszPathID )
{
	FileFindHandle_t findHandle = NULL;

	KeyValues *hiddenMaps = ModInfo().GetHiddenMaps();

	const char *pszFilename = g_pFullFileSystem->FindFirstEx( "maps/*.bsp", pszPathID, &findHandle );
	while ( pszFilename )
	{
		char mapname[256];
		char *ext, *str;

		// FindFirst ignores the pszPathID, so check it here
		// TODO: this doesn't find maps in fallback dirs
		Q_snprintf( mapname, sizeof(mapname), "maps/%s", pszFilename );
		if ( !g_pFullFileSystem->FileExists( mapname, pszPathID ) )
		{
			goto nextFile;
		}

		// remove the text 'maps/' and '.bsp' from the file name to get the map name
		
		str = Q_strstr( pszFilename, "maps" );
		if ( str )
		{
			Q_strncpy( mapname, str + 5, sizeof(mapname) - 1 );	// maps + \\ = 5
		}
		else
		{
			Q_strncpy( mapname, pszFilename, sizeof(mapname) - 1 );
		}
		ext = Q_strstr( mapname, ".bsp" );
		if ( ext )
		{
			*ext = 0;
		}

		//!! hack: strip out single player HL maps
		// this needs to be specified in a seperate file
		if ( !stricmp( ModInfo().GetGameName(), "Half-Life" ) && ( mapname[0] == 'c' || mapname[0] == 't') && mapname[2] == 'a' && mapname[1] >= '0' && mapname[1] <= '5' )
		{
			goto nextFile;
		}

		// strip out maps that shouldn't be displayed
		if ( hiddenMaps )
		{
			if ( hiddenMaps->GetInt( mapname, 0 ) )
			{
				goto nextFile;
			}
		}

		const char* pszGameType = g_pGameTypes->GetGameTypeFromInt( m_nGameTypeID );
		const char* pszGameMode = g_pGameTypes->GetGameModeFromInt( m_nGameTypeID, m_nGameModeID );
		if ( !m_pAllMapsCheck->IsSelected() && !g_pGameTypes->IsValidMapForTypeAndMode(mapname, pszGameType, pszGameMode) )
		{
			goto nextFile;
		}

		// add to the map list
		const char* pszUIName = g_pGameTypes->GetMapNameID( mapname );
		if ( pszUIName )
			m_pMapList->AddItem( pszUIName, new KeyValues( "data", "mapname", mapname ) );
		else
			m_pMapList->AddItem( mapname, new KeyValues( "data", "mapname", mapname ) );

		// get the next file
	nextFile:
		pszFilename = g_pFullFileSystem->FindNext( findHandle );
	}
	g_pFullFileSystem->FindClose( findHandle );
}



//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::LoadMapList()
{
	char szOldMapName[64];
	bool bOldMap = false;
	if ( GetMapName(false) )
	{
		bOldMap = true;
		V_strcpy( szOldMapName, GetMapName(false) );
	}

	// clear the current list (if any)
	m_pMapList->DeleteAllItems();

	// add special "name" to represent loading a randomly selected map
	m_pMapList->AddItem( RANDOM_MAP, new KeyValues( "data", "mapname", RANDOM_MAP ) );

	// Load the GameDir maps
	LoadMaps( "GAME" ); 

	if ( bOldMap )
		SetMap( szOldMapName );
	else
		m_pMapList->ActivateItem( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: loads the list of available game modes into the game modes list
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::LoadGameModesList()
{
	m_pGameModeList->DeleteAllItems();

	m_pAllMapsCheck->SetEnabled( m_nGameTypeID != CS_GameType_Custom );

	int iGameModeCount = g_pGameTypes->GetGameModesCount( m_nGameTypeID );
	for ( int i = 0; i < iGameModeCount; i++ )
	{
		const char* pszGameModeNameID = g_pGameTypes->GetGameModeNameID( m_nGameTypeID, i );
		if ( pszGameModeNameID )
			m_pGameModeList->AddItem( pszGameModeNameID, new KeyValues( "data", "game_mode", i ) );
	}
	m_pGameModeList->ActivateItem( 0 );
	m_pGameModeList->SetEnabled( iGameModeCount > 1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CCreateMultiplayerGameServerPage::IsRandomMapSelected()
{
	const char *mapname = m_pMapList->GetActiveItemUserData()->GetString("mapname");
	if (!stricmp( mapname, RANDOM_MAP ))
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CCreateMultiplayerGameServerPage::GetMapName(bool bAllowRandom)
{
	int count = m_pMapList->GetItemCount();

	// if there is only one entry it's the special "select random map" entry
	if( count <= 1 )
		return NULL;

	const char *mapname = m_pMapList->GetActiveItemUserData()->GetString("mapname");
	if (!strcmp( mapname, RANDOM_MAP ))
	{
		if ( !bAllowRandom )
			return NULL;

		int which = RandomInt( 1, count - 1 );
		mapname = m_pMapList->GetItemUserData( which )->GetString("mapname");
	}

	return mapname;
}

//-----------------------------------------------------------------------------
// Purpose: Sets currently selected map in the map combobox
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetMap(const char *mapName)
{
	for (int i = 0; i < m_pMapList->GetItemCount(); i++)
	{
		if (!m_pMapList->IsItemIDValid(i))
			continue;

		if (!stricmp(m_pMapList->GetItemUserData(i)->GetString("mapname"), mapName))
		{
			m_pMapList->ActivateItem(i);
			return;
		}
	}

	// just select the first one if mapName isn't in the list
	m_pMapList->ActivateItem( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CCreateMultiplayerGameServerPage::GetGameTypeID()
{
	return m_pGameTypeList->GetActiveItem();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetGameTypeID( int gametypeid )
{
	m_pGameTypeList->ActivateItem( gametypeid );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CCreateMultiplayerGameServerPage::GetGameModeID()
{
	return m_pGameModeList->GetActiveItem();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetGameModeID( int gamemodeid )
{
	m_pGameModeList->ActivateItem( gamemodeid );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CCreateMultiplayerGameServerPage::IsAllMaps()
{
	return m_pAllMapsCheck->IsSelected();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetAllMaps( bool bState )
{
	m_pAllMapsCheck->SetSelected( bState );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::OnTextChanged( Panel *panel )
{
	if ( panel == m_pBotSkillList )
	{
		SetControlEnabled( "BotQuotaCombo", m_pBotSkillList->GetActiveItem() != 0 );
		SetControlEnabled( "BotQuotaLabel", m_pBotSkillList->GetActiveItem() != 0 );
		SetControlEnabled( "BotDifficultyLabel", m_pBotSkillList->GetActiveItem() != 0 );
	}
}
