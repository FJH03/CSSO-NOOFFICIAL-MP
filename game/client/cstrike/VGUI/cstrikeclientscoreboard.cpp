//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "cstrikeclientscoreboard.h"
#include "c_cs_team.h"
#include "c_cs_playerresource.h"
#include "c_cs_player.h"
#include "cs_gamerules.h"
#include "backgroundpanel.h"
#include "clientmode.h"
#include "viewpostprocess.h"
#include "c_plantedc4.h"
#include "gametypes.h"
#include "filesystem.h"
#include "bitmap/bitmap.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/SectionedListPanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui_controls/ImageList.h>
#include "vgui_controls/SVGImage.h"
#include "vgui_controls/Tooltip.h"
#include "VGuiMatSurface/IMatSystemSurface.h"

#include "voice_status.h"
#include "vgui_avatarimage.h"

using namespace vgui;

const float kUpdateInterval = 0.5f;                             // how often the scoreboard refreshes

extern ConVar cash_team_loser_bonus;
extern ConVar cash_team_loser_bonus_consecutive_rounds;

CCSClientScoreBoardLossBonusPanel::CCSClientScoreBoardLossBonusPanel( vgui::Panel* pParent, const char* pszPanelName ): BaseClass( pParent, pszPanelName )
{
        m_iSegmentsFilled = 0;
        SetPaintBackgroundEnabled( false );
        SetMouseInputEnabled( true ); // allow the tooltip to show up
}

void CCSClientScoreBoardLossBonusPanel::SetFilledSegments( int iCount )
{
        m_iSegmentsFilled = iCount;

        wchar_t wszCount[16];
        V_snwprintf( wszCount, sizeof( wszCount ), L"%d", cash_team_loser_bonus.GetInt() + (cash_team_loser_bonus_consecutive_rounds.GetInt() * MIN( iCount, segment_count )) );

        wchar_t wszHint[64];
        g_pVGuiLocalize->ConstructString( wszHint, sizeof( wszHint ), g_pVGuiLocalize->Find( "#CStrike_Scoreboard_LossBonus_Hint" ), 1, wszCount );

        char szHint[64];
        g_pVGuiLocalize->ConvertUnicodeToANSI( wszHint, szHint, sizeof( szHint ) );
        GetTooltip()->SetText( szHint );
}

void CCSClientScoreBoardLossBonusPanel::Paint()
{
        int iSegmentsWide = (segment_wide * segment_count) + (segment_gap * (segment_count - 1));
        int iSegmentsTall = segment_tall;
        int iPanelWide, iPanelTall;
        GetSize( iPanelWide, iPanelTall );
        int iXPos = (iPanelWide / 2) - (iSegmentsWide / 2);
        int iYPos = (iPanelTall / 2) - (iSegmentsTall / 2);

        for ( int i = 0; i < segment_count; i++ )
        {
                if ( i < m_iSegmentsFilled )
                        surface()->DrawSetColor( GetFgColor() );
                else
                        surface()->DrawSetColor( GetBgColor() );
                surface()->DrawFilledRect( iXPos, iYPos, iXPos + segment_wide, iYPos + iSegmentsTall );

                iXPos += segment_wide + segment_gap;
        }
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCSClientScoreBoardDialog::CCSClientScoreBoardDialog( IViewPort *pViewPort ) : CClientScoreBoardDialog( pViewPort )
{
        m_pServerLabel = new Label( this, "ServerNameLabel", L"" );
        m_pRoundTimeLabel = new Label( this, "RoundTimeLabel", L"" );
        m_pGameModeIcon = new VectorImagePanel( this, "GameModeIcon" );
        m_pTeamCTScoreFirstHalf = new Label( this, "TeamCTScoreFirstHalf", L"" );
        m_pTeamCTScoreSecondHalf = new Label( this, "TeamCTScoreSecondHalf", L"" );
        m_pTeamCTScoreOvertime = new Label( this, "TeamCTScoreOvertime", L"" );
        m_pFirstHalfLabel = new Label( this, "FirstHalfLabel", L"" );
        m_pSecondHalfLabel = new Label( this, "SecondHalfLabel", L"" );
        m_pOvertimeLabel = new Label( this, "OvertimeLabel", L"" );
        m_pTeamTScoreFirstHalf = new Label( this, "TeamTScoreFirstHalf", L"" );
        m_pTeamTScoreSecondHalf = new Label( this, "TeamTScoreSecondHalf", L"" );
        m_pTeamTScoreOvertime = new Label( this, "TeamTScoreOvertime", L"" );
        m_pLossBonusLabel = new Label( this, "LossBonusLabel", L"" );
        m_pSpectatorsLabel = new Label( this, "SpectatorsLabel", L"" );
        m_pLossBonusCT = new CCSClientScoreBoardLossBonusPanel( this, "LossBonusCT" );
        m_pLossBonusT = new CCSClientScoreBoardLossBonusPanel( this, "LossBonusT" );

        m_pCTPlayerList = new SectionedListPanel( this, "TeamCTPlayerList" );
        m_pCTPlayerList->SetVerticalScrollbar( false );
        m_pCTPlayerList->SetPaintBackgroundEnabled( false );
        m_pCTPlayerList->SetPaintBorderEnabled( false );
        m_pCTPlayerList->SetSectionInset( 0, 0 );

        m_pTPlayerList = new SectionedListPanel( this, "TeamTPlayerList" );
        m_pTPlayerList->SetVerticalScrollbar( false );
        m_pTPlayerList->SetPaintBackgroundEnabled( false );
        m_pTPlayerList->SetPaintBorderEnabled( false );
        m_pTPlayerList->SetSectionInset( 0, 0 );

        // gets initialized in base class
        m_pPlayerList->SetPaintBackgroundEnabled( false );
        m_pPlayerList->SetPaintBorderEnabled( false );
        m_pPlayerList->SetSectionInset( 0, 0 );

        ListenForGameEvent( "server_spawn" );
        ListenForGameEvent( "game_newmap" );
        ListenForGameEvent( "announce_phase_end" );
        ListenForGameEvent( "round_start" );

        SetVisible( false );
        SetProportional( true );
        SetPaintBorderEnabled( false );
        SetScheme( "ClientScheme" );

        // [pfreese] Make the scoreboard a popup so it renders over the chat interface (which is also a popup). Hacky.
        MakePopup();
        SetMouseInputEnabled( false ); // PiMoN: MakePopup() makes both mouse and keyboard inputs "true", we don't need that
        SetKeyBoardInputEnabled( false );

        m_iRoundTime = 0;
        m_nGameType = -1;
        m_nGameMode = -1;

        if ( g_pClientMode &&
                 g_pClientMode->GetMapName() )
        {
                V_wcsncpy( m_pMapName, g_pClientMode->GetMapName(), sizeof( m_pMapName ) );

                const char* pszLocalizedGameModeName = g_pVGuiLocalize->FindAsUTF8( g_pGameTypes->GetCurrentGameModeNameID() );
                if ( pszLocalizedGameModeName )
                {
                        char szLocalizedGameModeName[128];
                        Q_strcpy( szLocalizedGameModeName, pszLocalizedGameModeName );
                        char szMapName[256];
                        g_pVGuiLocalize->ConvertUnicodeToANSI( m_pMapName, szMapName, sizeof( szMapName ) );

                        const char* pszLocalizedMapName = g_pVGuiLocalize->FindAsUTF8( g_pGameTypes->GetMapNameID( szMapName ) );
                        char szGameModeMap[256];

                        V_snprintf( szGameModeMap, sizeof( szGameModeMap ), "%s | %s", szLocalizedGameModeName, pszLocalizedMapName ? pszLocalizedMapName : szMapName );
                        SetDialogVariable( "mapname_gamemode", szGameModeMap );
                }
        }

        m_pServerName[0] = L'\0';

        m_bForceShow = false;
        m_iOriginalTall = 0;
        m_iOriginalCTPlayerListTall = 0;
        m_iOriginalTPlayerListTall = 0;
        m_iOriginalPlayerListTall = 0;
        m_bHasHalfTime = false;
        m_bHasOvertime = false;
        m_bHasLossBonus = false;
        m_bSimple = false;
}

const wchar_t *LocalizeFindSafe( const char *pTokenName )
{
        const wchar_t *pStr = g_pVGuiLocalize->Find( pTokenName );
        return pStr ? pStr : L"\0";
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CCSClientScoreBoardDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
        BaseClass::ApplySchemeSettings( pScheme );

        //
        // [smessick] Note: ApplySchemeSettings is called multiple times for the scoreboard.
        // Therefore, we must make sure to delete previously allocated items.
        //

        LoadControlSettings( "Resource/UI/scoreboard.res" );
        m_iOriginalTall = GetTall();
        m_iOriginalCTPlayerListTall = m_pCTPlayerList->GetTall();
        m_iOriginalTPlayerListTall = m_pTPlayerList->GetTall();
        m_iOriginalPlayerListTall = m_pPlayerList->GetTall();

        SetPaintBorderEnabled( false );

        // Set the server name (in the case of a resolution change).
        if ( m_pServerName[0] == L'\0' &&
                 g_pClientMode->GetServerName() != NULL )
        {
                V_wcsncpy( m_pServerName, g_pClientMode->GetServerName(), sizeof( m_pServerName ) );
        }

        SetVisible( false );
}

//-----------------------------------------------------------------------------
// Purpose: Does dialog-specific customization after applying scheme settings.
//-----------------------------------------------------------------------------
void CCSClientScoreBoardDialog::PostApplySchemeSettings( vgui::IScheme *pScheme )
{
        m_pCTPlayerList->SetImageList( m_pImageList, false );
        m_pTPlayerList->SetImageList( m_pImageList, false );
        m_pPlayerList->SetImageList( m_pImageList, false );
}

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CCSClientScoreBoardDialog::Reset()
{
        // clear
        m_pCTPlayerList->DeleteAllItems();
        m_pCTPlayerList->RemoveAllSections();
        m_pTPlayerList->DeleteAllItems();
        m_pTPlayerList->RemoveAllSections();

        BaseClass::Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the dialog
//-----------------------------------------------------------------------------
void CCSClientScoreBoardDialog::Update()
{
    if ( m_pServerLabel )
    {
        m_pServerLabel->SetText( m_pServerName );
    }

        UpdateImageList();
    UpdateTeamInfo();
        UpdatePlayerInfo();
        UpdateHLTVList();

    // update every second
    m_fNextUpdateTime = gpGlobals->curtime + kUpdateInterval;

        // grow the scoreboard to fit all the players
        int iWide, iTall;

        if ( m_bSimple )
        {
                m_pPlayerList->GetContentSize( iWide, iTall );
                int iAdditionalPlayerListTall = MAX( 0, iTall - m_iOriginalPlayerListTall );
                m_pPlayerList->SetTall( m_iOriginalPlayerListTall + iAdditionalPlayerListTall );

                SetTall( m_iOriginalTall + iAdditionalPlayerListTall );
        }
        else
        {
                m_pCTPlayerList->GetContentSize( iWide, iTall );
                int iAdditionalCTPlayerListTall = MAX( 0, iTall - m_iOriginalCTPlayerListTall );
                m_pCTPlayerList->SetTall( m_iOriginalCTPlayerListTall + iAdditionalCTPlayerListTall );

                m_pTPlayerList->GetContentSize( iWide, iTall );
                int iAdditionalTPlayerListTall = MAX( 0, iTall - m_iOriginalTPlayerListTall );
                m_pTPlayerList->SetTall( m_iOriginalTPlayerListTall + iAdditionalTPlayerListTall );

                int iAdditionalTall = iAdditionalCTPlayerListTall + iAdditionalTPlayerListTall;
                SetTall( m_iOriginalTall + iAdditionalTall );
        }

        MoveToCenterOfScreen();

    // Catch the case where we call ShowPanel before ApplySchemeSettings, eg when going from windowed <-> fullscreen
    if ( m_pImageList == NULL )
    {
        InvalidateLayout( true, true );
    }
}

void CCSClientScoreBoardDialog::UpdateImageList()
{
        if ( m_pImageList->GetImageCount() < 2 ) // < 2 because it always has a blank image by default
        {
                // fill the image list with default images
                // NOTE: these go in order of PlayerListIcons_t!

                // DEFUSER_ICON
                SVGImage* pImage = new SVGImage();
                pImage->SetSize( 0, player_status_icon_tall );
                pImage->SetTexture( "materials/vgui/hud/svg/defuser.svg" ); // TODO: softcode
                m_pImageList->AddImage( pImage );

                // BOMB_ICON
                pImage = new SVGImage();
                pImage->SetSize( 0, player_status_icon_tall );
                pImage->SetTexture( "materials/vgui/hud/svg/bomb_c4.svg" ); // TODO: softcode
                m_pImageList->AddImage( pImage );

                // DEAD_ICON
                pImage = new SVGImage();
                pImage->SetSize( 0, player_status_icon_tall );
                pImage->SetTexture( "materials/vgui/hud/svg/elimination.svg" );  // TODO: softcode
                m_pImageList->AddImage( pImage );

                // CT_AVATAR
                CAvatarImage* pAvatarImage = new CAvatarImage();
                pAvatarImage->SetDrawFriend( false );
                pAvatarImage->SetAvatarSize( avatar_column_wide, avatar_column_wide );  // Deliberately non scaling
                pAvatarImage->SetDefaultImage( scheme()->GetImage( CSTRIKE_DEFAULT_CT_AVATAR, true ) );
                pAvatarImage->SetSize( avatar_column_wide, avatar_column_wide );
                m_pImageList->AddImage( pAvatarImage );

                // T_AVATAR
                pAvatarImage = new CAvatarImage();
                pAvatarImage->SetDrawFriend( false );
                pAvatarImage->SetAvatarSize( avatar_column_wide, avatar_column_wide );  // Deliberately non scaling
                pAvatarImage->SetDefaultImage( scheme()->GetImage( CSTRIKE_DEFAULT_T_AVATAR, true ) );
                pAvatarImage->SetSize( avatar_column_wide, avatar_column_wide );
                m_pImageList->AddImage( pAvatarImage );
        }
}

extern ConVar mat_blur_strength;
extern ConVar mat_blur_desaturate;
void CCSClientScoreBoardDialog::PaintBackground()
{
        if ( engine->GetDXSupportLevel() < 90 )
                BaseClass::PaintBackground();
        else
        {
                // do the blur here instead of clientmode because it needs to render over VGUI elements
                int x, y, w, h;
                GetBounds( x, y, w, h );
                DoBlurFade( mat_blur_strength.GetFloat(), mat_blur_desaturate.GetFloat(), x, y, w, h );
        }
}

#if SCOREBOARD_MOUSE_INPUT
void CCSClientScoreBoardDialog::SetMouseInputEnabled( bool state )
{
        BaseClass::SetMouseInputEnabled( state );

        // hide server label when we enable mouse input
        // so the buttons can go over
        m_pServerLabel->SetVisible( !state );
}
#endif

bool CCSClientScoreBoardDialog::CSStaticPlayerSortFunc( vgui::SectionedListPanel* list, int itemID1, int itemID2 )
{
        // TODO: support for different sorting!

        KeyValues* it1 = list->GetItemData( itemID1 );
        KeyValues* it2 = list->GetItemData( itemID2 );
        Assert( it1 && it2 );

        // first compare gg progression level
        int v1 = it1->GetInt( "gglevel" );
        int v2 = it2->GetInt( "gglevel" );
        if ( v1 > v2 )
                return true;
        else if ( v1 < v2 )
                return false;

        // next compare score
        v1 = it1->GetInt( "score" );
        v2 = it2->GetInt( "score" );
        if ( v1 > v2 )
                return true;
        else if ( v1 < v2 )
                return false;

        // next compare kills
        v1 = it1->GetInt( "kills" );
        v2 = it2->GetInt( "kills" );
        if ( v1 > v2 )
                return true;
        else if ( v1 < v2 )
                return false;

        // next compare assists
        v1 = it1->GetInt( "assists" );
        v2 = it2->GetInt( "assists" );
        if ( v1 > v2 )
                return true;
        else if ( v1 < v2 )
                return false;

        // next compare mvps
        // cant use GetInt because its not a number (eg. x2)
        v1 = _wtoi( it1->GetWString( "mvps" ) + 1 );
        v2 = _wtoi( it2->GetWString( "mvps" ) + 1 );
        if ( v1 > v2 )
                return true;
        else if ( v1 < v2 )
                return false;

        // next compare deaths
        v1 = it1->GetInt( "deaths" );
        v2 = it2->GetInt( "deaths" );
        if ( v1 > v2 )
                return false;
        else if ( v1 < v2 )
                return true;

        // the same, so compare itemID's (as a sentinel value to get deterministic sorts)
        return itemID1 < itemID2;
}

void CCSClientScoreBoardDialog::InitScoreboardSections()
{
        if ( m_bSimple )
        {
                bool bGunGameProgressive = (m_nGameType == CS_GameType_GunGame && m_nGameMode == CS_GameMode::GunGame_Progressive);
                int iColumnsWide = ping_column_wide + avatar_column_wide + avatar_name_gap_wide + kills_column_wide + deaths_column_wide + score_column_wide;
                if ( bGunGameProgressive )
                        iColumnsWide += gglevel_column_wide;
                else
                        iColumnsWide += assists_column_wide;

                // setup the columns
                m_pPlayerList->AddSection( 0, "", CSStaticPlayerSortFunc );
                m_pPlayerList->SetSectionDrawDividerBar( 0, false );
                m_pPlayerList->SetSectionFgColor( 0, player_header_fgcolor );
                m_pPlayerList->AddColumnToSection( 0, "ping", "", SectionedListPanel::COLUMN_CENTER, ping_column_wide );
                m_pPlayerList->AddColumnToSection( 0, "avatar", "", SectionedListPanel::COLUMN_IMAGE, avatar_column_wide );
                m_pPlayerList->AddColumnToSection( 0, "nothing", "", 0, avatar_name_gap_wide );
                //m_pPlayerList->AddColumnToSection( 0, "name", "", 0, name_column_wide );
                m_pPlayerList->AddColumnToSection( 0, "name", "", 0, m_pPlayerList->GetWide() - iColumnsWide );
                m_pPlayerList->AddColumnToSection( 0, "kills", "#CStrike_SB_Kills", SectionedListPanel::COLUMN_CENTER, kills_column_wide );
                if ( !bGunGameProgressive )
                        m_pPlayerList->AddColumnToSection( 0, "assists", "#CStrike_SB_Assists", SectionedListPanel::COLUMN_CENTER, assists_column_wide );
                m_pPlayerList->AddColumnToSection( 0, "deaths", "#CStrike_SB_Deaths", SectionedListPanel::COLUMN_CENTER, deaths_column_wide );
                m_pPlayerList->AddColumnToSection( 0, "score", "#CStrike_SB_Score", SectionedListPanel::COLUMN_CENTER, score_column_wide );
                if ( bGunGameProgressive )
                        m_pPlayerList->AddColumnToSection( 0, "gglevel", "#CStrike_SB_GGLevel", SectionedListPanel::COLUMN_CENTER, gglevel_column_wide );

                // setup the column bg color
                bool bSkip = false;
                int iColumnCount = m_pPlayerList->GetColumnCountBySection( 0 );
                for ( int i = 4; i < iColumnCount; i++ ) // TODO: change 4 to m_pPlayerList->GetColumnIndexByName( 0, "name" ) + 1 if it ever changes!
                {
                        if ( i == iColumnCount - 1 )
                        {
                                m_pPlayerList->SetColumnBgColor( 0, i, player_column_bgcolor2 );
                        }
                        else
                        {
                                if ( bSkip )
                                {
                                        bSkip = false;
                                        continue;
                                }

                                m_pPlayerList->SetColumnBgColor( 0, i, player_column_bgcolor1 );

                                bSkip = true;
                        }
                }
        }
        else
        {
                bool bGunGameTR = (m_nGameType == CS_GameType_GunGame && m_nGameMode == CS_GameMode::GunGame_Bomb);
                int iColumnsWide = ping_column_wide + avatar_column_wide + avatar_name_gap_wide + kills_column_wide + assists_column_wide + deaths_column_wide + mvps_column_wide + score_column_wide;
                if ( bGunGameTR )
                        iColumnsWide += kd_column_wide;
                else
                        iColumnsWide += money_column_wide;

                // setup the columns
                m_pCTPlayerList->AddSection( 0, "", CSStaticPlayerSortFunc );
                m_pCTPlayerList->SetSectionDrawDividerBar( 0, false );
                m_pCTPlayerList->SetSectionFgColor( 0, player_header_fgcolor );
                m_pCTPlayerList->AddColumnToSection( 0, "ping", "", SectionedListPanel::COLUMN_CENTER, ping_column_wide );
                m_pCTPlayerList->AddColumnToSection( 0, "avatar", "", SectionedListPanel::COLUMN_IMAGE, avatar_column_wide );
                m_pCTPlayerList->AddColumnToSection( 0, "nothing", "", 0, avatar_name_gap_wide );
                //m_pCTPlayerList->AddColumnToSection( 0, "name", "", 0, name_column_wide );
                m_pCTPlayerList->AddColumnToSection( 0, "name", "", 0, m_pCTPlayerList->GetWide() - iColumnsWide );
                if ( !bGunGameTR )
                        m_pCTPlayerList->AddColumnToSection( 0, "money", "#CStrike_SB_Money", SectionedListPanel::COLUMN_CENTER, money_column_wide );
                m_pCTPlayerList->AddColumnToSection( 0, "kills", "#CStrike_SB_Kills", SectionedListPanel::COLUMN_CENTER, kills_column_wide );
                m_pCTPlayerList->AddColumnToSection( 0, "assists", "#CStrike_SB_Assists", SectionedListPanel::COLUMN_CENTER, assists_column_wide );
                m_pCTPlayerList->AddColumnToSection( 0, "deaths", "#CStrike_SB_Deaths", SectionedListPanel::COLUMN_CENTER, deaths_column_wide );
                if ( bGunGameTR )
                        m_pCTPlayerList->AddColumnToSection( 0, "kd", "#CStrike_SB_KD", SectionedListPanel::COLUMN_CENTER, kd_column_wide );
                m_pCTPlayerList->AddColumnToSection( 0, "mvps", "#CStrike_SB_MVP", SectionedListPanel::COLUMN_CENTER, mvps_column_wide );
                m_pCTPlayerList->AddColumnToSection( 0, "score", "#CStrike_SB_Score", SectionedListPanel::COLUMN_CENTER, score_column_wide );

                // setup the column bg color
                bool bSkip = false;
                int iColumnCount = m_pCTPlayerList->GetColumnCountBySection( 0 );
                for ( int i = 4; i < iColumnCount; i++ ) // TODO: change 4 to m_pCTPlayerList->GetColumnIndexByName( 0, "name" ) + 1 if it ever changes!
                {
                        if ( i == iColumnCount - 1 )
                        {
                                m_pCTPlayerList->SetColumnBgColor( 0, i, player_column_bgcolor2 );
                        }
                        else
                        {
                                if ( bSkip )
                                {
                                        bSkip = false;
                                        continue;
                                }

                                m_pCTPlayerList->SetColumnBgColor( 0, i, player_column_bgcolor1 );

                                bSkip = true;
                        }
                }

                // setup the columns
                m_pTPlayerList->AddSection( 0, "", CSStaticPlayerSortFunc );
                m_pTPlayerList->SetSectionDrawDividerBar( 0, false );
                m_pTPlayerList->SetSectionFgColor( 0, player_header_fgcolor );
                m_pTPlayerList->AddColumnToSection( 0, "ping", "", SectionedListPanel::COLUMN_CENTER, ping_column_wide );
                m_pTPlayerList->AddColumnToSection( 0, "avatar", "", SectionedListPanel::COLUMN_IMAGE, avatar_column_wide );
                m_pTPlayerList->AddColumnToSection( 0, "nothing", "", 0, avatar_name_gap_wide );
                //m_pTPlayerList->AddColumnToSection( 0, "name", "", 0, name_column_wide );
                m_pTPlayerList->AddColumnToSection( 0, "name", "", 0, m_pTPlayerList->GetWide() - iColumnsWide );
                if ( !bGunGameTR )
                        m_pTPlayerList->AddColumnToSection( 0, "money", "#CStrike_SB_Money", SectionedListPanel::COLUMN_CENTER, money_column_wide );
                m_pTPlayerList->AddColumnToSection( 0, "kills", "#CStrike_SB_Kills", SectionedListPanel::COLUMN_CENTER, kills_column_wide );
                m_pTPlayerList->AddColumnToSection( 0, "assists", "#CStrike_SB_Assists", SectionedListPanel::COLUMN_CENTER, assists_column_wide );
                m_pTPlayerList->AddColumnToSection( 0, "deaths", "#CStrike_SB_Deaths", SectionedListPanel::COLUMN_CENTER, deaths_column_wide );
                if ( bGunGameTR )
                        m_pTPlayerList->AddColumnToSection( 0, "kd", "#CStrike_SB_KD", SectionedListPanel::COLUMN_CENTER, kd_column_wide );
                m_pTPlayerList->AddColumnToSection( 0, "mvps", "#CStrike_SB_MVP", SectionedListPanel::COLUMN_CENTER, mvps_column_wide );
                m_pTPlayerList->AddColumnToSection( 0, "score", "#CStrike_SB_Score", SectionedListPanel::COLUMN_CENTER, score_column_wide );

                // setup the column bg color
                bSkip = false;
                iColumnCount = m_pTPlayerList->GetColumnCountBySection( 0 );
                for ( int i = 4; i < iColumnCount; i++ ) // TODO: change 4 to m_pTPlayerList->GetColumnIndexByName( 0, "name" ) + 1 if it ever changes!
                {
                        if ( i == iColumnCount - 1 )
                        {
                                m_pTPlayerList->SetColumnBgColor( 0, i, player_column_bgcolor2 );
                        }
                        else
                        {
                                if ( bSkip )
                                {
                                        bSkip = false;
                                        continue;
                                }

                                m_pTPlayerList->SetColumnBgColor( 0, i, player_column_bgcolor1 );

                                bSkip = true;
                        }
                }
        }
}

//-----------------------------------------------------------------------------
// Purpose: searches for the player in the scoreboard
//-----------------------------------------------------------------------------
int CCSClientScoreBoardDialog::FindItemIDForPlayerIndex( int playerIndex )
{
        if ( !g_PR )
                return -1;

        if ( m_bSimple )
        {
                for ( int i = 0; i <= m_pPlayerList->GetHighestItemID(); i++ )
                {
                        if ( m_pPlayerList->IsItemIDValid( i ) )
                        {
                                KeyValues* kv = m_pPlayerList->GetItemData( i );
                                kv = kv->FindKey( m_iPlayerIndexSymbol );
                                if ( kv && kv->GetInt() == playerIndex )
                                        return i;
                        }
                }
        }
        else
        {
                int iTeamNumber = g_PR->GetTeam( playerIndex );

                if ( iTeamNumber == TEAM_CT )
                {
                        for ( int i = 0; i <= m_pCTPlayerList->GetHighestItemID(); i++ )
                        {
                                if ( m_pCTPlayerList->IsItemIDValid( i ) )
                                {
                                        KeyValues* kv = m_pCTPlayerList->GetItemData( i );
                                        kv = kv->FindKey( m_iPlayerIndexSymbol );
                                        if ( kv && kv->GetInt() == playerIndex )
                                                return i;
                                }
                        }
                }
                else if ( iTeamNumber == TEAM_TERRORIST )
                {
                        for ( int i = 0; i <= m_pTPlayerList->GetHighestItemID(); i++ )
                        {
                                if ( m_pTPlayerList->IsItemIDValid( i ) )
                                {
                                        KeyValues* kv = m_pTPlayerList->GetItemData( i );
                                        kv = kv->FindKey( m_iPlayerIndexSymbol );
                                        if ( kv && kv->GetInt() == playerIndex )
                                                return i;
                                }
                        }
                }
        }

        return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Updates information about teams
//-----------------------------------------------------------------------------
void CCSClientScoreBoardDialog::UpdateTeamInfo()
{
        C_CS_PlayerResource* cs_PR = (C_CS_PlayerResource*) g_PR;
        if ( !cs_PR )
                return;

        // update the team sections in the scoreboard
        for ( int teamIndex = TEAM_TERRORIST; teamIndex <= TEAM_CT; teamIndex++ )
        {
                wchar_t teamName[512];
                C_CSTeam* team = GetGlobalCSTeam( teamIndex );
                if ( team )
                {
                        // choose dialog variables to set depending on team
                        const char* pDialogVarTeamName = NULL;
                        const char* pDialogVarAliveCount = NULL;
                        const char* pDialogVarTeamScore = NULL;
                        const char* pDialogVarTeamScoreFirstHalf = NULL;
                        const char* pDialogVarTeamScoreSecondHalf = NULL;
                        const char* pDialogVarTeamScoreOvertime = NULL;
                        switch ( teamIndex )
                        {
                                case TEAM_TERRORIST:
                                        g_pVGuiLocalize->ConstructString( teamName, sizeof( teamName ), g_pVGuiLocalize->Find( "#Cstrike_Team_T_Upper" ), nullptr );
                                        pDialogVarTeamName = "t_teamname";
                                        pDialogVarAliveCount = "t_alivecount";
                                        pDialogVarTeamScore = "t_totalteamscore";
                                        pDialogVarTeamScoreFirstHalf = "t_firsthalfteamscore";
                                        pDialogVarTeamScoreSecondHalf = "t_secondhalfteamscore";
                                        pDialogVarTeamScoreOvertime = "t_overtimeteamscore";
                                        break;
                                case TEAM_CT:
                                        g_pVGuiLocalize->ConstructString( teamName, sizeof( teamName ), g_pVGuiLocalize->Find( "#CStrike_Team_CT_Upper" ), nullptr );
                                        pDialogVarTeamName = "ct_teamname";
                                        pDialogVarAliveCount = "ct_alivecount";
                                        pDialogVarTeamScore = "ct_totalteamscore";
                                        pDialogVarTeamScoreFirstHalf = "ct_firsthalfteamscore";
                                        pDialogVarTeamScoreSecondHalf = "ct_secondhalfteamscore";
                                        pDialogVarTeamScoreOvertime = "ct_overtimeteamscore";
                                        break;
                                default:
                                        Assert( false );
                                        break;
                        }

                        if ( !StringIsEmpty( team->Get_ClanName() ) )
                        {
                                g_pVGuiLocalize->ConstructString( teamName, sizeof( teamName ), team->Get_ClanName(), nullptr );
                        }

                        // Count the players on the team.
                        int numPlayers = 0;
                        int numAlive = 0;
                        for ( int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++ )
                        {
                                if ( g_PR->IsConnected( playerIndex ) && g_PR->GetTeam( playerIndex ) == teamIndex )
                                {
                                        int nControlledByPlayerIndex = cs_PR->GetControlledByPlayer( playerIndex );

                                        bool bIsAlive = false;
                                        if ( nControlledByPlayerIndex > 0 )
                                                bIsAlive = cs_PR->IsAlive( nControlledByPlayerIndex );
                                        else
                                                bIsAlive = !cs_PR->IsControllingBot( playerIndex ) && cs_PR->IsAlive( playerIndex );

                                        numPlayers++;
                                        if ( bIsAlive )
                                        {
                                                ++numAlive;
                                        }
                                }
                        }

                        SetDialogVariable( pDialogVarTeamName, teamName );

                        // Team score
                        wchar_t wNumScore[16];
                        V_snwprintf( wNumScore, ARRAYSIZE( wNumScore ), L"%i", team->Get_Score() );
                        SetDialogVariable( pDialogVarTeamScore, wNumScore );
                        if ( m_bHasHalfTime )
                        {
                                V_snwprintf( wNumScore, ARRAYSIZE( wNumScore ), L"%i", team->Get_Score_First_Half() );
                                SetDialogVariable( pDialogVarTeamScoreFirstHalf, wNumScore );
                                V_snwprintf( wNumScore, ARRAYSIZE( wNumScore ), L"%i", team->Get_Score_Second_Half() );
                                SetDialogVariable( pDialogVarTeamScoreSecondHalf, wNumScore );
                        }
                        if ( m_bHasOvertime )
                        {
                                V_snwprintf( wNumScore, ARRAYSIZE( wNumScore ), L"%i", team->Get_Score_Overtime() );
                                SetDialogVariable( pDialogVarTeamScoreOvertime, wNumScore );
                        }

                        // Number of alive players
                        wchar_t wszNumAlive[8];
                        wchar_t wszNumPlayers[8];
                        wchar_t wszAlivePlayers[64];
                        V_snwprintf( wszNumAlive, ARRAYSIZE( wszNumAlive ), L"%d", numAlive );
                        V_snwprintf( wszNumPlayers, ARRAYSIZE( wszNumPlayers ), L"%d", numPlayers );
                        g_pVGuiLocalize->ConstructString( wszAlivePlayers, sizeof( wszAlivePlayers ), g_pVGuiLocalize->Find( "#CStrike_SB_Alive" ), 2, wszNumAlive, wszNumPlayers );
                        SetDialogVariable( pDialogVarAliveCount, wszAlivePlayers );
                }
        }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSClientScoreBoardDialog::UpdatePlayerInfo()
{
        C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer();
        C_CS_PlayerResource* cs_PR = (C_CS_PlayerResource*) g_PR;
        if ( !pPlayer || !cs_PR )
                return;

        int iLocalPlayerIndex = pPlayer->entindex();
        if ( pPlayer->IsControllingBot() )
                iLocalPlayerIndex = pPlayer->GetControlledBotIndex();

        // walk all the players and make sure they're in the scoreboard
        int nSpectators = 0;
        wchar_t wszSpectatorsList[256];
        wszSpectatorsList[0] = '\0';
        for ( int i = 1; i <= gpGlobals->maxClients; i++ )
        {
                int nTeamNumber = cs_PR->GetTeam( i );
                bool bShouldShow = cs_PR->IsConnected( i );
                int nControlledByPlayerIndex = cs_PR->GetControlledByPlayer( i );

                bool bIsAlive = false;
                if ( nControlledByPlayerIndex > 0 )
                        bIsAlive = cs_PR->IsAlive( nControlledByPlayerIndex );
                else
                        bIsAlive = !cs_PR->IsControllingBot( i ) && cs_PR->IsAlive( i );

                SectionedListPanel* pPlayerList;
                if ( m_bSimple )
                        pPlayerList = m_pPlayerList;
                else
                        pPlayerList = (nTeamNumber == TEAM_CT) ? m_pCTPlayerList : m_pTPlayerList;

                if ( bShouldShow )
                {
                        if ( nTeamNumber == TEAM_CT || nTeamNumber == TEAM_TERRORIST )
                        {
                                KeyValues* playerData = new KeyValues( "data" );
                                GetPlayerScoreInfo( i, playerData );
                                UpdatePlayerAvatar( i, playerData );
                                int nItemID = FindItemIDForPlayerIndex( i );

                                if ( nItemID == -1 )
                                {
                                        // add a new row
                                        nItemID = pPlayerList->AddItem( 0, playerData );
                                }
                                else
                                {
                                        // modify the current row
                                        pPlayerList->ModifyItem( nItemID, 0, playerData );
                                }

                                // set the row color based on the players team
                                Color fgColor = cs_PR->GetTeamColor( nTeamNumber );
                                if ( !bIsAlive )
                                        fgColor[3] *= 0.5f; // half transparent

                                if ( i == iLocalPlayerIndex && bIsAlive )
                                {
                                        pPlayerList->SetItemFgColor( nItemID, COLOR_WHITE );
                                        pPlayerList->SetItemBgColor( nItemID, (nTeamNumber == TEAM_CT) ? localplayer_ct_bgcolor : localplayer_t_bgcolor );
                                }
                                else
                                {
                                        pPlayerList->SetItemFgColor( nItemID, fgColor );
                                        pPlayerList->SetItemBgColor( nItemID, bIsAlive ? player_bgcolor : dead_player_bgcolor );
                                }

                                playerData->deleteThis();
                        }
                        else
                        {
                                // remove the player
                                int nItemID = FindItemIDForPlayerIndex( i );
                                if ( nItemID != -1 )
                                {
                                        pPlayerList->RemoveItem( nItemID );
                                }

                                if ( nTeamNumber == TEAM_UNASSIGNED || nTeamNumber == TEAM_SPECTATOR )
                                {
                                        nSpectators++;
                                        wchar_t wszPlayerName[MAX_DECORATED_PLAYER_NAME_LENGTH];
                                        cs_PR->GetDecoratedPlayerName( i, wszPlayerName, sizeof( wszPlayerName ), k_EDecoratedPlayerNameFlag_Simple );

                                        if ( nSpectators > 1 )
                                                V_wcscat_safe( wszSpectatorsList, L", " );
                                        V_wcscat_safe( wszSpectatorsList, wszPlayerName );
                                }
                        }
                }
                else
                {
                        // remove the player
                        int nItemID = FindItemIDForPlayerIndex( i );
                        if ( nItemID != -1 )
                        {
                                pPlayerList->RemoveItem( nItemID );
                        }
                }
        }

        if ( nSpectators > 0 )
        {
                wchar_t wszSpectatorsLabel[512];
                g_pVGuiLocalize->ConstructString( wszSpectatorsLabel, sizeof( wszSpectatorsLabel ), g_pVGuiLocalize->Find( "#CStrike_Scoreboard_Spectators" ), 1, wszSpectatorsList );
                m_pSpectatorsLabel->SetText( wszSpectatorsLabel );
                m_pSpectatorsLabel->SetVisible( true );
        }
        else
        {
                m_pSpectatorsLabel->SetVisible( false );
        }
}

//-----------------------------------------------------------------------------
// Purpose: Display the number of HLTV viewers
//-----------------------------------------------------------------------------
void CCSClientScoreBoardDialog::UpdateHLTVList( void )
{
        if ( m_HLTVSpectators > 0 )
        {
                // Build the text for the number of viewers.
                wchar_t countText[16];
                V_snwprintf( countText, sizeof( countText ), L"%i", m_HLTVSpectators );

                // Build the combined text.
                wchar_t labelText[128];
                g_pVGuiLocalize->ConstructString( labelText, sizeof( labelText ), g_pVGuiLocalize->Find( "#Cstrike_Scoreboard_HLTV" ), 1, countText );

                m_pServerLabel->SetText( labelText );
        }
        else
        {
                m_pServerLabel->SetText( m_pServerName );
        }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSClientScoreBoardDialog::UpdatePlayerAvatar( int playerIndex, KeyValues* kv )
{
        if ( !g_PR )
                return;

        player_info_t pi;
        int iTeamNumber = g_PR->GetTeam( playerIndex );
        int iImageIndex;

        // First, try to load custom VTF avatar from networked CRC (customFiles[2])
        // This works like the spray system - avatars are uploaded via sv_allowupload
        if ( engine->GetPlayerInfo( playerIndex, &pi ) && pi.customFiles[2] != 0 )
        {
                // Check if we already have this CRC cached
                int iMapIndex = m_mapAvatarsToImageList.Find( CSteamID( pi.customFiles[2], 0, k_EUniverseInvalid, k_EAccountTypeInvalid ) );
                if ( iMapIndex == m_mapAvatarsToImageList.InvalidIndex() )
                {
                        CAvatarImage* pImage = new CAvatarImage();
                        pImage->SetDrawFriend( false );
                        pImage->SetAvatarSize( avatar_column_wide, avatar_column_wide );
                        
                        // Try to load the VTF avatar from CRC
                        if ( pImage->SetAvatarFromCRC( pi.customFiles[2] ) )
                        {
                                iImageIndex = m_pImageList->AddImage( pImage );
                                m_mapAvatarsToImageList.Insert( CSteamID( pi.customFiles[2], 0, k_EUniverseInvalid, k_EAccountTypeInvalid ), iImageIndex );
                                kv->SetInt( "avatar", iImageIndex );
                                return;
                        }
                        else
                        {
                                delete pImage;
                        }
                }
                else
                {
                        // Already cached, use it
                        iImageIndex = m_mapAvatarsToImageList[iMapIndex];
                        kv->SetInt( "avatar", iImageIndex );
                        return;
                }
        }

        // Update their avatar - try Steam avatars
        if ( kv && ShowAvatars() && steamapicontext->SteamFriends() && steamapicontext->SteamUtils() )
        {
                if ( engine->GetPlayerInfo( playerIndex, &pi ) )
                {
                        if ( pi.friendsID )
                        {
                                IImage* pDefaultImage;
                                if ( iTeamNumber == TEAM_TERRORIST )
                                {
                                        pDefaultImage = scheme()->GetImage( CSTRIKE_DEFAULT_T_AVATAR, true );
                                }
                                else
                                {
                                        pDefaultImage = scheme()->GetImage( CSTRIKE_DEFAULT_CT_AVATAR, true );
                                }

                                CSteamID steamIDForPlayer( pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );

                                // See if we already have that avatar in our list
                                int iMapIndex = m_mapAvatarsToImageList.Find( steamIDForPlayer );
                                if ( iMapIndex == m_mapAvatarsToImageList.InvalidIndex() )
                                {
                                        CAvatarImage* pImage = new CAvatarImage();
                                        pImage->SetDrawFriend( false );
                                        pImage->SetAvatarSize( avatar_column_wide, avatar_column_wide );        // Deliberately non scaling
                                        pImage->SetDefaultImage( pDefaultImage );
                                        pImage->SetAvatarSteamID( steamIDForPlayer );
                                        iImageIndex = m_pImageList->AddImage( pImage );

                                        m_mapAvatarsToImageList.Insert( steamIDForPlayer, iImageIndex );
                                }
                                else
                                {
                                        iImageIndex = m_mapAvatarsToImageList[iMapIndex];
                                }

                                kv->SetInt( "avatar", iImageIndex );

                                CAvatarImage* pAvIm = (CAvatarImage*) m_pImageList->GetImage( iImageIndex );
                                if ( pAvIm )
                                {
                                        pAvIm->UpdateFriendStatus();
                                }
                                return;
                        }
                }
        }

        // Fallback: use team avatars for bots or when Steam is not available
        if ( iTeamNumber == TEAM_TERRORIST )
                iImageIndex = T_AVATAR;
        else
                iImageIndex = CT_AVATAR;

        kv->SetInt( "avatar", iImageIndex );
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CCSClientScoreBoardDialog::GetPlayerScoreInfo( int playerIndex, KeyValues* kv )
{
        C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
        C_CS_PlayerResource* cs_PR = dynamic_cast<C_CS_PlayerResource*>(g_PR);
        if ( !pPlayer || !cs_PR )
                return false;

        int iPing = cs_PR->GetPing( playerIndex );
        if ( iPing < 1 )
                kv->SetWString( "ping", g_pVGuiLocalize->Find( "CStrike_Scoreboard_Bot" ) );
        else
                kv->SetInt( "ping", cs_PR->GetPing( playerIndex ) );

        wchar_t wszPlayerName[MAX_DECORATED_PLAYER_NAME_LENGTH];
        cs_PR->GetDecoratedPlayerName( playerIndex, wszPlayerName, sizeof( wszPlayerName ), k_EDecoratedPlayerNameFlag_Simple );
        kv->SetWString( "name", wszPlayerName );

        if ( pPlayer->GetTeamNumber() == cs_PR->GetTeam( playerIndex ) ) // PiMoN TODO: is there a convar for this in CSGO?
        {
                wchar_t wszMoney[16];
                V_snwprintf( wszMoney, sizeof( wszMoney ), L"$%d", cs_PR->GetAccount( playerIndex ) );
                kv->SetWString( "money", wszMoney );
        }
        else
        {
                kv->SetWString( "money", L" " );
        }

        int iKills = cs_PR->GetPlayerScore( playerIndex );
        int iDeaths = cs_PR->GetDeaths( playerIndex );
        kv->SetInt( "kills", iKills );
        kv->SetInt( "assists", cs_PR->GetAssists( playerIndex ) );
        kv->SetInt( "deaths", iDeaths );

        wchar_t wszKD[8];
        V_snwprintf( wszKD, sizeof( wszKD ), L"%.2f", MAX( 0.0f, (float) iKills / (float) MAX( 1, iDeaths ) ) );
        kv->SetWString( "kd", wszKD );

        int iMVPs = cs_PR->GetNumMVPs( playerIndex );
        if ( iMVPs < 1 )
        {
                kv->SetWString( "mvps", L" " );
        }
        else
        {
                wchar_t wszMVP[8];
                V_snwprintf( wszMVP, sizeof( wszMVP ), L"x%d", cs_PR->GetNumMVPs( playerIndex ) );
                kv->SetWString( "mvps", wszMVP );
        }
        kv->SetInt( "score", cs_PR->GetContributionScore( playerIndex ) );
        kv->SetInt( "gglevel", cs_PR->GetPlayerGunGameWeaponIndex( playerIndex ) + 1 );

        int nControlledByPlayerIndex = cs_PR->GetControlledByPlayer( playerIndex );

        bool bIsAlive = false;
        if ( nControlledByPlayerIndex > 0 )
                bIsAlive = cs_PR->IsAlive( nControlledByPlayerIndex );
        else
                bIsAlive = !cs_PR->IsControllingBot( playerIndex ) && cs_PR->IsAlive( playerIndex );

        bool bHasC4 = false;
        if ( nControlledByPlayerIndex > 0 )
                bHasC4 = cs_PR->HasC4( nControlledByPlayerIndex );
        else
                bHasC4 = !cs_PR->IsControllingBot( playerIndex ) && cs_PR->HasC4( playerIndex );

        bool bHasDefuser = false;
        if ( nControlledByPlayerIndex > 0 )
                bHasDefuser = cs_PR->HasDefuser( nControlledByPlayerIndex );
        else
                bHasDefuser = !cs_PR->IsControllingBot( playerIndex ) && cs_PR->HasDefuser( playerIndex );

        if ( !bIsAlive )
                kv->SetInt( SECTIONED_LIST_HEADER_IMAGE, DEAD_ICON );
        else if ( pPlayer->GetTeamNumber() == TEAM_TERRORIST && bHasC4 )
                kv->SetInt( SECTIONED_LIST_HEADER_IMAGE, BOMB_ICON );
        else if ( pPlayer->GetTeamNumber() == TEAM_CT && bHasDefuser )
                kv->SetInt( SECTIONED_LIST_HEADER_IMAGE, DEFUSER_ICON );

        kv->SetInt( "playerIndex", playerIndex );

        return true;
}

void CCSClientScoreBoardDialog::FireGameEvent( IGameEvent *event )
{
        if ( event == NULL )
                return;

    const char *pEventName = event->GetName();
        if ( pEventName == NULL )
                return;

    if ( Q_strcmp( pEventName, "server_spawn" ) == 0 )
    {
        // set server name in scoreboard
        const char *hostname = event->GetString( "hostname" );
                if ( hostname != NULL )
                {
                        wchar_t wzHostName[256];
                        g_pVGuiLocalize->ConvertANSIToUnicode( hostname, wzHostName, sizeof( wzHostName ) );
                        g_pVGuiLocalize->ConstructString( m_pServerName, sizeof(m_pServerName), g_pVGuiLocalize->Find( "#Cstrike_SB_Server" ), 1, wzHostName );

                        if ( m_pServerLabel )
                        {
                                m_pServerLabel->SetText( m_pServerName );
                        }

                        // Save the server name for use after this panel is reconstructed
                        if ( g_pClientMode )
                        {
                                g_pClientMode->SetServerName( m_pServerName );
                        }
                }
    }
    else if ( Q_strcmp( pEventName, "game_newmap" ) == 0 )
    {
        const char *mapName = event->GetString( "mapname" );
                if ( mapName != NULL )
                {
                        g_pVGuiLocalize->ConvertANSIToUnicode( mapName, m_pMapName, sizeof( m_pMapName ) );

                        const char* pszLocalizedGameModeName = g_pVGuiLocalize->FindAsUTF8( g_pGameTypes->GetCurrentGameModeNameID() );
                        if ( pszLocalizedGameModeName )
                        {
                                char szLocalizedGameModeName[128];
                                Q_strcpy( szLocalizedGameModeName, pszLocalizedGameModeName );

                                const char* pszLocalizedMapName = g_pVGuiLocalize->FindAsUTF8( g_pGameTypes->GetMapNameID( mapName ) );
                                char szGameModeMap[256];

                                V_snprintf( szGameModeMap, sizeof( szGameModeMap ), "%s | %s", szLocalizedGameModeName, pszLocalizedMapName ? pszLocalizedMapName : mapName );
                                SetDialogVariable( "mapname_gamemode", szGameModeMap );
                        }

                        // Save the map name for use after this panel is reconstructed
                        if ( g_pClientMode )
                        {
                                g_pClientMode->SetMapName( m_pMapName );
                        }
                }

                m_bForceShow = false; // clear it so players wont get stuck on scoreboard after map change
    }
        else if ( Q_strcmp( pEventName, "announce_phase_end" ) == 0 )
        {
                m_bForceShow = true;
                ShowPanel( m_bForceShow );
        }
        else if ( Q_strcmp( pEventName, "round_start" ) == 0 )
        {
                m_bForceShow = false;
                ShowPanel( m_bForceShow );
        }

        BaseClass::FireGameEvent( event );
}

// [tj] We hook into the show command so we can lock or unlock all the elements that need to be hidden
//
// [pfreese] This used to enable/disable keyboard input, but since the scoreboard is now a popup, we have
// to leave the keyboard disabled
void CCSClientScoreBoardDialog::ShowPanel( bool state )
{
        if ( m_bForceShow && !state )
                return;

#if SCOREBOARD_MOUSE_INPUT
        SetMouseInputEnabled( false ); // always disable it before showing the panel since clientmode might have enabled it
#endif

        // set a correct game mode icon
        int iGameType = g_pGameTypes->GetCurrentGameType();
        int iGameMode = g_pGameTypes->GetCurrentGameMode();
        if ( m_nGameType != iGameType || m_nGameMode != iGameMode )
        {
                m_nGameType = iGameType;
                m_nGameMode = iGameMode;

                // simplified scoreboard for AR and DM
                if ( iGameType == CS_GameType_GunGame && (iGameMode == CS_GameMode::GunGame_Progressive || iGameMode == CS_GameMode::GunGame_Deathmatch) )
                {
                        LoadControlSettings( "Resource/UI/scoreboard_simple.res" );

                        m_iOriginalTall = GetTall();
                        m_iOriginalPlayerListTall = m_pPlayerList->GetTall();
                        m_bSimple = true;
                }
                else
                {
                        LoadControlSettings( "Resource/UI/scoreboard.res" );

                        m_iOriginalTall = GetTall();
                        m_iOriginalCTPlayerListTall = m_pCTPlayerList->GetTall();
                        m_iOriginalTPlayerListTall = m_pTPlayerList->GetTall();
                        m_bSimple = false;
                }

                const char* pszCurrentGameMode = g_pGameTypes->GetGameModeFromInt( iGameType, iGameMode );
                if ( pszCurrentGameMode )
                {
                        char szIconPath[64];
                        V_snprintf( szIconPath, sizeof( szIconPath ), "materials/vgui/hud/svg/%s.svg", pszCurrentGameMode );
                        if ( !g_pFullFileSystem->FileExists( szIconPath ) )
                                m_pGameModeIcon->SetTexture( "materials/vgui/hud/svg/casual.svg" );
                        else
                                m_pGameModeIcon->SetTexture( szIconPath );
                }
                else
                {
                        m_pGameModeIcon->SetTexture( "materials/vgui/hud/svg/casual.svg" );
                }
        }

        if ( state )
        {
                m_bHasHalfTime = CSGameRules() ? CSGameRules()->HasHalfTime() : false;
                m_bHasOvertime = CSGameRules() ? (CSGameRules()->GetOvertimePlaying() > 0) : false;
                m_bHasLossBonus = (cash_team_loser_bonus_consecutive_rounds.GetInt() > 0) && (cash_team_loser_bonus.GetInt() > 0);

                m_pTeamCTScoreFirstHalf->SetVisible( m_bHasHalfTime );
                m_pTeamCTScoreSecondHalf->SetVisible( m_bHasHalfTime );
                m_pTeamCTScoreOvertime->SetVisible( m_bHasOvertime );
                m_pFirstHalfLabel->SetVisible( m_bHasHalfTime );
                m_pSecondHalfLabel->SetVisible( m_bHasHalfTime );
                m_pOvertimeLabel->SetVisible( m_bHasOvertime );
                m_pTeamTScoreFirstHalf->SetVisible( m_bHasHalfTime );
                m_pTeamTScoreSecondHalf->SetVisible( m_bHasHalfTime );
                m_pTeamTScoreOvertime->SetVisible( m_bHasOvertime );
                m_pLossBonusLabel->SetVisible( m_bHasLossBonus );
                m_pLossBonusCT->SetVisible( m_bHasLossBonus );
                m_pLossBonusT->SetVisible( m_bHasLossBonus );
                m_pLossBonusCT->SetFilledSegments( CSGameRules() ? CSGameRules()->m_iNumConsecutiveCTLoses : 0 );
                m_pLossBonusT->SetFilledSegments( CSGameRules() ? CSGameRules()->m_iNumConsecutiveTerroristLoses : 0 );
        }

    BaseClass::ShowPanel(state);

    int iRenderGroup = gHUD.LookupRenderGroupIndexByName( "hide_for_scoreboard" );

    if ( state )
    {
        gHUD.LockRenderGroup( iRenderGroup );
    }
    else
    {           
        gHUD.UnlockRenderGroup( iRenderGroup );
    }
}


// [tj] Disabling joystick input if you are dead.
void CCSClientScoreBoardDialog::OnThink()
{
    BaseClass::OnThink();

#ifdef _XBOX
    C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
    if ( pLocalPlayer )
    {
        bool mouseEnabled = IsMouseInputEnabled();
        if (pLocalPlayer->IsAlive() == mouseEnabled)
        {
            SetMouseInputEnabled( !mouseEnabled );
        }
    }
#endif

        if ( !CSGameRules() )
                return;

        bool bBombPlanted = (g_PlantedC4s.Count() > 0);
        if ( bBombPlanted || CSGameRules()->IsTimeOutActive() || CSGameRules()->IsWarmupPeriod() )
                m_pRoundTimeLabel->SetVisible( false );
        else
        {
                int iRoundTime = (int) ceil( CSGameRules()->GetRoundRemainingTime() );
                if ( iRoundTime < 0 )
                        iRoundTime = 0;

                if ( m_iRoundTime != iRoundTime )
                {
                        m_iRoundTime = iRoundTime;

                        int iMinutes = iRoundTime / 60;
                        int iSeconds = iRoundTime % 60;

                        wchar_t unicode[16];
                        V_snwprintf( unicode, ARRAYSIZE( unicode ), L"%d:%.2d", iMinutes, iSeconds );
                        m_pRoundTimeLabel->SetText( unicode );

                        m_pRoundTimeLabel->SetVisible( true );
                }
        }
}
