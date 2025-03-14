//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CREATEMULTIPLAYERGAMESERVERPAGE_H
#define CREATEMULTIPLAYERGAMESERVERPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include "CvarToggleCheckButton.h"

//-----------------------------------------------------------------------------
// Purpose: server options page of the create game server dialog
//-----------------------------------------------------------------------------
class CCreateMultiplayerGameServerPage : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CCreateMultiplayerGameServerPage, vgui::PropertyPage );

public:
	CCreateMultiplayerGameServerPage(vgui::Panel *parent, const char *name, int nGameType, int nGameMode, bool bAllMaps);
	~CCreateMultiplayerGameServerPage();

	virtual void OnKeyCodePressed( vgui::KeyCode code );

	// returns currently entered information about the server
	void SetMap(const char *name);
	bool IsRandomMapSelected();
	const char *GetMapName(bool bAllowRandom = true);

	int GetGameTypeID();
	void SetGameTypeID( int nGameTypeID, bool bActivate = true, bool bForce = false );
	int GetGameModeID();
	void SetGameModeID( int nGameModeID, bool bActivate = true, bool bForce = false );

	bool IsAllMaps();
	void SetAllMaps( bool bState, bool bActivate = true, bool bForce = false );

	vgui::ListPanel *GetMapList( void ) { return m_pMapList; }

	// CS Bots
	void EnableBots( KeyValues *data );

	// returns currently entered information about the server
	const char *GetHostName();
	void SetHostName( const char* pszValue );
	int GetMaxPlayers();
	void SetMaxPlayers( const char* pszValue );
	const char* GetPassword();
	void SetPassword( const char* pszValue );

	void UpdateGameplayPage();

protected:
	virtual void OnApplyChanges();
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );
	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel );

private:
	void LoadMapList();
	void LoadMaps( const char *pszPathID );
	void LoadGameModesList();

	vgui::ListPanel *m_pMapList;
	vgui::ComboBox *m_pGameTypeList;
	vgui::ComboBox *m_pGameModeList;
	vgui::ComboBox *m_pBotSkillList;
	vgui::CheckButton *m_pAllMapsCheck;
	vgui::TextEntry *m_pHostNameEdit;
	vgui::TextEntry *m_pMaxPlayersEdit;
	vgui::TextEntry *m_pServerPasswordEdit;
	KeyValues *m_pSavedData;

	enum { DATA_STR_LENGTH = 64 };
	char m_szMapName[DATA_STR_LENGTH];
	int m_nGameModeID;
	int m_nGameTypeID;
	bool m_bAllMaps;
};


#endif // CREATEMULTIPLAYERGAMESERVERPAGE_H
