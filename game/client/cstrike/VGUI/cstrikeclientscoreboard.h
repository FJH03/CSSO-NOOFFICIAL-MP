//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSTRIKECLIENTSCOREBOARDDIALOG_H
#define CSTRIKECLIENTSCOREBOARDDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <clientscoreboarddialog.h>
#include <vgui_controls/ImagePanel.h>
#include "cs_shareddefs.h"
#include <vgui_controls/Frame.h>
#include "vgui_avatarimage.h"
#include "c_cs_playerresource.h"

#define SCOREBOARD_MOUSE_INPUT 0

enum PlayerListIcons_t
{
    // NOTE: these go in order with UpdateImageList, starting from index 1!
    DEFUSER_ICON = 1,
    BOMB_ICON,
    DEAD_ICON,
    CT_AVATAR,
    T_AVATAR
};

class CCSClientScoreBoardLossBonusPanel: public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( CCSClientScoreBoardLossBonusPanel, vgui::Panel );

public:
    CCSClientScoreBoardLossBonusPanel( vgui::Panel* pParent, const char* pszPanelName );

    virtual void Paint();

    void SetFilledSegments( int iCount );
    
private:
    int m_iSegmentsFilled;
    CPanelAnimationVar( int, segment_count, "segment_count", "0" );
    CPanelAnimationVarAliasType( int, segment_gap, "segment_gap", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, segment_wide, "segment_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, segment_tall, "segment_tall", "0", "proportional_height" );
};

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CCSClientScoreBoardDialog : public CClientScoreBoardDialog
{
private:
    DECLARE_CLASS_SIMPLE( CCSClientScoreBoardDialog, CClientScoreBoardDialog );

public:
    CCSClientScoreBoardDialog( IViewPort *pViewPort );

    virtual void Update();
    virtual void Reset();

    // vgui overrides for rounded corner background
    virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
    virtual void PostApplySchemeSettings( vgui::IScheme* pScheme );

    // [tj] Hook in here to hide other UI
    virtual void ShowPanel( bool state ); 

    // [tj] So we can do processing every frame
    virtual void OnThink();

	virtual void PaintBackground();
#if SCOREBOARD_MOUSE_INPUT
    virtual void SetMouseInputEnabled( bool state );
#endif

protected:
    virtual void FireGameEvent( IGameEvent* event );
    virtual bool GetPlayerScoreInfo( int playerIndex, KeyValues* outPlayerInfo );
    virtual void InitScoreboardSections();
    virtual int FindItemIDForPlayerIndex( int playerIndex );
    virtual void UpdateTeamInfo();
    virtual void UpdatePlayerInfo();
    virtual void UpdateHLTVList();
    virtual void UpdatePlayerAvatar( int playerIndex, KeyValues* kv );
    virtual void UpdateImageList();

    // sorts players within a section
    static bool CSStaticPlayerSortFunc( vgui::SectionedListPanel* list, int itemID1, int itemID2 );

private:
	vgui::Label* m_pServerLabel;
	vgui::Label* m_pRoundTimeLabel;
	vgui::VectorImagePanel* m_pGameModeIcon;
    vgui::SectionedListPanel* m_pCTPlayerList;
    vgui::SectionedListPanel* m_pTPlayerList;
    vgui::Label* m_pTeamCTScoreFirstHalf;
    vgui::Label* m_pTeamCTScoreSecondHalf;
    vgui::Label* m_pTeamCTScoreOvertime;
    vgui::Label* m_pFirstHalfLabel;
    vgui::Label* m_pSecondHalfLabel;
    vgui::Label* m_pOvertimeLabel;
    vgui::Label* m_pTeamTScoreFirstHalf;
    vgui::Label* m_pTeamTScoreSecondHalf;
    vgui::Label* m_pTeamTScoreOvertime;
    vgui::Label* m_pLossBonusLabel;
    vgui::Label* m_pSpectatorsLabel;
    CCSClientScoreBoardLossBonusPanel* m_pLossBonusCT;
    CCSClientScoreBoardLossBonusPanel* m_pLossBonusT;

	int     m_iRoundTime;
	int     m_nGameType;
	int     m_nGameMode;
    wchar_t m_pMapName[256];
    wchar_t m_pServerName[256];
	bool    m_bForceShow;
    int     m_iOriginalTall;
    int     m_iOriginalCTPlayerListTall;
    int     m_iOriginalTPlayerListTall;
    int     m_iOriginalPlayerListTall;
    bool    m_bHasHalfTime;
    bool    m_bHasOvertime;
    bool    m_bHasLossBonus;
    bool    m_bSimple;

    CPanelAnimationVarAliasType( int, ping_column_wide, "ping_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, avatar_column_wide, "avatar_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, avatar_name_gap_wide, "avatar_name_gap_wide", "0", "proportional_width" );
    //CPanelAnimationVarAliasType( int, name_column_wide, "name_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, money_column_wide, "money_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, kills_column_wide, "kills_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, assists_column_wide, "assists_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, deaths_column_wide, "deaths_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, kd_column_wide, "kd_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, mvps_column_wide, "mvps_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, score_column_wide, "score_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, gglevel_column_wide, "gglevel_column_wide", "0", "proportional_width" );
    CPanelAnimationVarAliasType( int, player_status_icon_tall, "player_status_icon_tall", "0", "proportional_height" );
    CPanelAnimationVar( Color, player_bgcolor, "player_bgcolor", "White" );
    CPanelAnimationVar( Color, dead_player_bgcolor, "dead_player_bgcolor", "White" );
    CPanelAnimationVar( Color, localplayer_ct_bgcolor, "localplayer_ct_bgcolor", "White" );
    CPanelAnimationVar( Color, localplayer_t_bgcolor, "localplayer_t_bgcolor", "White" );
    CPanelAnimationVar( Color, player_column_bgcolor1, "player_column_bgcolor1", "White" );
    CPanelAnimationVar( Color, player_column_bgcolor2, "player_column_bgcolor2", "White" );
    CPanelAnimationVar( Color, player_header_fgcolor, "player_header_fgcolor", "White" );
};


#endif // CSTRIKECLIENTSCOREBOARDDIALOG_H
