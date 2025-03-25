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
#include <vgui_controls/PropertySheet.h>
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
	m_pMapList = new ListPanel(this, "MapList");

	m_pAllMapsCheck = new CheckButton( this, "AllMapsCheck", "" );
	m_pAllMapsCheck->SetSelected( bAllMaps );

	m_pGameTypeList = new ComboBox( this, "GameTypeList", 5, false );
	m_pGameModeList = new ComboBox( this, "GameModeList", 5, false );
	m_pBotSkillList = new ComboBox( this, "BotSkillList", 5, false );
	m_pHostNameEdit = new TextEntry( this, "HostNameEdit" );
	m_pMaxPlayersEdit = new TextEntry( this, "MaxPlayersEdit" );
	m_pServerPasswordEdit = new TextEntry( this, "ServerPasswordEdit" );

	int iGameTypeCount = g_pGameTypes->GetGameTypesCount();
	for ( int i = 0; i < iGameTypeCount; i++ )
	{
		const char* pszGameTypeNameID = g_pGameTypes->GetGameTypeNameID( i );
		if ( pszGameTypeNameID )
			m_pGameTypeList->AddItem( pszGameTypeNameID, new KeyValues( "data", "game_type", i ) );
	}
	m_pGameTypeList->ActivateItem( 0 );
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

	m_pMapList->AddColumnHeader( 0, "uimapname", "#GameUI_Map", m_pMapList->GetWide() );

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
	
	m_nGameTypeID = 0;
	m_nGameModeID = 0;
	m_bAllMaps = false;
	SetGameTypeID( nGameType, true, true );
	SetGameModeID( nGameMode, true, true );
	SetAllMaps( bAllMaps, true, true );
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
		int nItem = m_pMapList->GetSelectedItem(0);
		nItem -= 1;
		if ( nItem < 0 )
		{
			nItem = m_pMapList->GetItemCount() - 1;
		}

		m_pMapList->SetSingleSelectedItem( nItem );
	}
	else if ( code == KEY_XBUTTON_RIGHT || code == KEY_XSTICK1_RIGHT || code == KEY_XSTICK2_RIGHT )
	{
		int nItem = m_pMapList->GetSelectedItem(0);
		nItem += 1;
		if ( nItem >= m_pMapList->GetItemCount() )
		{
			nItem = 0;
		}

		m_pMapList->SetSingleSelectedItem( nItem );
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
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
// Purpose: 
//-----------------------------------------------------------------------------
int CCreateMultiplayerGameServerPage::GetMaxPlayers()
{
	wchar_t wszValue[128];
	m_pMaxPlayersEdit->GetText( wszValue, sizeof( wszValue ) );

	return _wtoi( wszValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetMaxPlayers( const char* pszValue )
{
	m_pMaxPlayersEdit->SetText( pszValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CCreateMultiplayerGameServerPage::GetPassword()
{
	static char szValue[128];
	m_pServerPasswordEdit->GetText( szValue, sizeof( szValue ) );

	return szValue;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetPassword( const char* pszValue )
{
	m_pServerPasswordEdit->SetText( pszValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CCreateMultiplayerGameServerPage::GetHostName()
{
	static char szValue[128];
	m_pHostNameEdit->GetText( szValue, sizeof( szValue ) );

	return szValue;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetHostName( const char* pszValue )
{
	m_pHostNameEdit->SetText( pszValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::UpdateGameplayPage()
{
	PropertySheet* pParent = dynamic_cast<PropertySheet*>(GetParent());
	if ( pParent )
	{
		if ( m_nGameTypeID == CS_GameType_Custom )
			pParent->EnablePage( "GameplayPage" );
		else
			pParent->DisablePage( "GameplayPage" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: called to get the info from the dialog
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::OnApplyChanges()
{
	int selectedItemID = m_pMapList->GetSelectedItem(0);
	if (selectedItemID >= 0)
	{
		KeyValues *kv = m_pMapList->GetItem(selectedItemID);
		Q_strncpy(m_szMapName, kv->GetString("mapname", ""), DATA_STR_LENGTH);
	}

	if ( m_pSavedData )
	{
		int quota = GetControlInt( "BotQuotaCombo", 0 );
		m_pSavedData->SetInt( "bot_quota", quota );

		int difficulty = m_pBotSkillList->GetActiveItem();
		m_pSavedData->SetInt( "custom_bot_difficulty", difficulty );
		g_pGameTypes->SetCustomBotDifficulty( difficulty );

		ConVarRef bot_quota( "bot_quota" );
		bot_quota.SetValue( (difficulty == 0) ? 0 : quota );
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

    const char* pszGameType = NULL;
    const char* pszGameMode = NULL;
    KeyValues* pkvData = NULL;

	while ( pszFilename )
	{
		char mapname[256];
		char *ext, *str;

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

		// strip out maps that shouldn't be displayed
		if ( hiddenMaps )
		{
			if ( hiddenMaps->GetInt( mapname, 0 ) )
			{
				goto nextFile;
			}
		}

        pszGameType = g_pGameTypes->GetGameTypeFromInt( m_nGameTypeID );
        pszGameMode = g_pGameTypes->GetGameModeFromInt( m_nGameTypeID, m_nGameModeID );

		if ( !m_pAllMapsCheck->IsSelected() && !g_pGameTypes->IsValidMapForTypeAndMode(mapname, pszGameType, pszGameMode) )
		{
			goto nextFile;
		}

		// add to the map list
		pkvData = new KeyValues( "data" );
		if ( pkvData )
		{
			pkvData->SetString( "mapname", mapname );
			pkvData->SetString( "uimapname", g_pGameTypes->GetMapNameID( mapname ) );
			m_pMapList->AddItem( pkvData, 0, false, false );
		}

		// get the next file
	nextFile:
		pszFilename = g_pFullFileSystem->FindNext( findHandle );
	}
	g_pFullFileSystem->FindClose( findHandle );

	// set the first item to be selected
	if (m_pMapList->GetItemCount() > 0)
	{
		m_pMapList->SetSingleSelectedItem(m_pMapList->GetItemIDFromRow(0));
	}
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
	KeyValues* pkvData = new KeyValues( "data" );
	if ( pkvData )
	{
		pkvData->SetString( "mapname", RANDOM_MAP );
		pkvData->SetString( "uimapname", RANDOM_MAP );
		m_pMapList->AddItem( pkvData, 0, false, false );
	}

	// Load the GameDir maps
	LoadMaps( "GAME" ); 

	if ( bOldMap )
		SetMap( szOldMapName );
	else
		m_pMapList->SetSingleSelectedItem( 0 );
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

	UpdateGameplayPage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CCreateMultiplayerGameServerPage::IsRandomMapSelected()
{
	const char *mapname = m_pMapList->GetItem(m_pMapList->GetSelectedItem(0))->GetString("mapname");
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

	const char *mapname = m_pMapList->GetItem(m_pMapList->GetSelectedItem(0))->GetString("mapname");
	if (!strcmp( mapname, RANDOM_MAP ))
	{
		if ( !bAllowRandom )
			return NULL;

		int which = RandomInt( 1, count - 1 );
		mapname = m_pMapList->GetItem( which )->GetString("mapname");
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
		if (!m_pMapList->IsValidItemID(i))
			continue;

		if (!stricmp(m_pMapList->GetItem(i)->GetString("mapname"), mapName))
		{
			m_pMapList->SetSingleSelectedItem(i);
			return;
		}
	}

	// just select the first one if mapName isn't in the list
	m_pMapList->SetSingleSelectedItem(m_pMapList->GetItemIDFromRow(0));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CCreateMultiplayerGameServerPage::GetGameTypeID()
{
	return m_nGameTypeID;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetGameTypeID( int nGameTypeID, bool bActivate, bool bForce )
{
	if ( m_nGameTypeID != nGameTypeID || bForce )
	{
		m_nGameTypeID = nGameTypeID;
		if ( bActivate )
			m_pGameTypeList->ActivateItem( nGameTypeID );
		LoadGameModesList();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CCreateMultiplayerGameServerPage::GetGameModeID()
{
	return m_nGameModeID;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetGameModeID( int nGameModeID, bool bActivate, bool bForce )
{
	if ( m_nGameModeID != nGameModeID || bForce )
	{
		m_nGameModeID = nGameModeID;
		if ( bActivate )
			m_pGameModeList->ActivateItem( nGameModeID );
		LoadMapList();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CCreateMultiplayerGameServerPage::IsAllMaps()
{
	return m_bAllMaps;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::SetAllMaps( bool bState, bool bActivate, bool bForce )
{
	if ( m_bAllMaps != bState || bForce )
	{
		m_bAllMaps = bState;
		if ( bActivate )
			m_pAllMapsCheck->SetSelected( bState );
		LoadMapList();
	}
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

	if ( panel == m_pGameTypeList )
	{
		SetGameTypeID( m_pGameTypeList->GetActiveItem(), false );
	}

	if ( panel == m_pGameModeList )
	{
		SetGameModeID( m_pGameModeList->GetActiveItem(), false, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameServerPage::OnCheckButtonChecked( Panel *panel )
{
	if ( panel == m_pAllMapsCheck )
	{
		SetAllMaps( m_pAllMapsCheck->IsSelected(), false );
	}
}
