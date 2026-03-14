//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: a small piece of HUD that shows alive counter and win counter for each team
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "hudelement.h"
#include "c_cs_player.h"
#include "c_cs_team.h"
#include "c_cs_playerresource.h"
#include "cs_gamerules.h"
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include "c_plantedc4.h"
#include "cs_hud_teamcounter.h"
#include "voice_status.h"
#include "engine/IEngineSound.h"

using namespace vgui;

ConVar hud_playercount_pos("hud_playercount_pos", "0", FCVAR_ARCHIVE, "0 = default (top), 1 = bottom");
ConVar hud_playercount_showhealth("hud_playercount_showhealth", "1", FCVAR_ARCHIVE, "Show health/armor in player counter");
ConVar hud_playercount_large_avatars("hud_playercount_large_avatars", "0", FCVAR_ARCHIVE, "Force 64x64 avatars in all modes (for testing)");
ConVar hud_playercount_gungame_layout("hud_playercount_gungame_layout", "0", FCVAR_ARCHIVE, "Force Gun Game layout (single row under timer) for testing");
extern ConVar cl_draw_only_deathnotices;
extern CUtlVector<C_PlantedC4*> g_PlantedC4s;

DECLARE_HUDELEMENT(CHudTeamCounter);

// Helper function to get default avatar based on player's team
static vgui::IImage* GetDefaultAvatarImage(C_CSPlayer *pPlayer)
{
    if (!pPlayer)
        return vgui::scheme()->GetImage("vgui/avatar_default", true);
    
    int team = pPlayer->GetTeamNumber();
    if (team == TEAM_CT)
        return vgui::scheme()->GetImage( CSTRIKE_DEFAULT_CT_AVATAR, true );
    if (team == TEAM_TERRORIST)
        return vgui::scheme()->GetImage( CSTRIKE_DEFAULT_T_AVATAR, true );
        else
        return vgui::scheme()->GetImage("vgui/avatar_default", true);
}

// Static variable for Gun Game Progressive leader tracking
static int g_GGProgLeaderPlayerIdx = -1;

CHudTeamCounter::CHudTeamCounter(const char *pElementName) : CHudElement(pElementName), EditablePanel(NULL, "HudTeamCounter")
{
    vgui::Panel *pParent = g_pClientMode->GetViewport();
    SetParent(pParent);

    SetHiddenBits(HIDEHUD_PLAYERDEAD);

    m_pCTWinCounterLabel = new Label(this, "CTWinCounterLabel", "0");
    m_pCTAliveCounterLabel = new Label(this, "CTAliveCounterLabel", "0");
    m_pCTAliveTextLabel = new Label(this, "CTAliveTextLabel", "#Cstrike_PlayerCount_Alive");
    m_pTWinCounterLabel = new Label(this, "TWinCounterLabel", "0");
    m_pTAliveCounterLabel = new Label(this, "TAliveCounterLabel", "0");
    m_pTAliveTextLabel = new Label(this, "TAliveTextLabel", "#Cstrike_PlayerCount_Alive");
    m_pRoundTimerLabel = new Label(this, "RoundTimerLabel", "0:00");
    m_pBombIcon = new VectorImagePanel(this, "BombIcon");
    m_pCTSkullImage = new VectorImagePanel(this, "CTSkullImage");
    m_pTSkullImage = new VectorImagePanel(this, "TSkullImage");
    m_pProgressiveLeaderLabel = new Label(this, "ProgressiveLeaderLabel", "");

    for (int i = 0; i < MAX_TEAM_SIZE; ++i)
    {
        char panelName[32];
        
        Q_snprintf(panelName, sizeof(panelName), "CTPlayerIconFrame%d", i);
        m_CTPlayerIconFrames[i] = new vgui::Panel(this, panelName);
        m_CTPlayerIconFrames[i]->SetVisible(false);
        
        Q_snprintf(panelName, sizeof(panelName), "CTPlayerIcon%d", i);
        m_CTPlayerIcons[i] = new CAvatarImagePanel(this, panelName);
        m_CTPlayerIcons[i]->SetShouldScaleImage(true);
        m_CTPlayerIcons[i]->SetShouldDrawFriendIcon(false);

        Q_snprintf(panelName, sizeof(panelName), "CTSkull%d", i);
        m_CTSkulls[i] = new VectorImagePanel(this, panelName);
        m_CTSkulls[i]->SetTexture("materials/vgui/hud/svg/teamcounter_aliveskull.svg");

        Q_snprintf(panelName, sizeof(panelName), "CTMicIcon%d", i);
        m_CTMicIcons[i] = new ImagePanel(this, panelName);
        m_CTMicIcons[i]->SetImage("vgui/hud/voice_icon");
        m_CTMicIcons[i]->SetVisible(false);

        Q_snprintf(panelName, sizeof(panelName), "CTPlayerName%d", i);
        m_CTPlayerNames[i] = new Label(this, panelName, "");

        Q_snprintf(panelName, sizeof(panelName), "CTPlayerStatus%d", i);
        m_CTPlayerStatus[i] = new Label(this, panelName, "");

        Q_snprintf(panelName, sizeof(panelName), "TPlayerIconFrame%d", i);
        m_TPlayerIconFrames[i] = new vgui::Panel(this, panelName);
        m_TPlayerIconFrames[i]->SetVisible(false);
        
        Q_snprintf(panelName, sizeof(panelName), "TPlayerIcon%d", i);
        m_TPlayerIcons[i] = new CAvatarImagePanel(this, panelName);
        m_TPlayerIcons[i]->SetShouldScaleImage(true);
        m_TPlayerIcons[i]->SetShouldDrawFriendIcon(false);

        Q_snprintf(panelName, sizeof(panelName), "TSkull%d", i);
        m_TSkulls[i] = new VectorImagePanel(this, panelName);
        m_TSkulls[i]->SetTexture("materials/vgui/hud/svg/teamcounter_aliveskull.svg");

        Q_snprintf(panelName, sizeof(panelName), "TMicIcon%d", i);
        m_TMicIcons[i] = new ImagePanel(this, panelName);
        m_TMicIcons[i]->SetImage("vgui/hud/voice_icon");
        m_TMicIcons[i]->SetVisible(false);

        Q_snprintf(panelName, sizeof(panelName), "TPlayerName%d", i);
        m_TPlayerNames[i] = new Label(this, panelName, "");

        Q_snprintf(panelName, sizeof(panelName), "TPlayerStatus%d", i);
        m_TPlayerStatus[i] = new Label(this, panelName, "");
        
        m_nLastAvatarPlayerIdx_CT[i] = -1;
        m_nLastAvatarPlayerIdx_T[i] = -1;
    }

    m_bTimerAlertTriggered = false;
    m_bRoundStarted = true;
    m_bIsBombDefused = false;
    m_bTimerHidden = false;
    m_nTScoreLastUpdate = -1;
    m_nCTScoreLastUpdate = -1;
    m_nTerroristTeamCount = 0;
    m_nCTTeamCount = 0;
    m_nPreviousGGProgressiveTotalPlayers = -1;
    m_flPlayingTeamFadeoutTime = -1;
    m_flLastSpecListUpdate = -1;
    m_bActive = true;
    m_iRoundTime = 0;
    m_nLastObserverMode = OBS_MODE_NONE;
    m_nLastObserverTarget = 0;
    m_flLastMiniScoreboardUpdate = 0.0f;
    m_Mode = VIEW_MODE_NORMAL;
    m_iOriginalXPos     = 0;
    m_iOriginalYPos     = 0;
    m_iTimerXPos = 0;
    m_iTimerYPos = 0;
    m_iTimerWide = 0;
    m_iTimerTall = 0;
    m_iAvatarXMargin = 2;
    m_iAvatarYMargin = 1;
    m_iAvatarWide = 14;
    m_iAvatarTall = 14;
    m_iAvatarXMax = 6;
    m_iAvatarYMax = 2;
    m_iAvatarBorderSize = 2;
}

CHudTeamCounter::~CHudTeamCounter()
{
}

void CHudTeamCounter::Init()
{
    m_bTimerAlertTriggered = false;
    m_bRoundStarted = true;
    m_iRoundTime = 0;
    m_bIsBombDefused = false;
    m_bTimerHidden = false;
    m_nTScoreLastUpdate = -1;
    m_nCTScoreLastUpdate = -1;
    m_flLastSpecListUpdate = -1;

    // Restore panel to its un-expanded state before LoadControlSettings so
    // ApplySettings records the correct un-shifted position as "original".
    if (m_bLayoutExpanded && m_iOriginalWide > 0)
    {
        SetPos(m_iOriginalXPos, m_iOriginalYPos);
        SetWide(m_iOriginalWide);
    }
    m_bLayoutExpanded   = false;
    m_iLayoutShiftLeft  = 0;
    m_iLayoutShiftRight = 0;

    ListenForGameEvent("round_start");
    ListenForGameEvent("round_announce_warmup");
    ListenForGameEvent("round_end");
    ListenForGameEvent("cs_match_end_restart");
    ListenForGameEvent("bomb_planted");
    ListenForGameEvent("bomb_defused");
    ListenForGameEvent("player_spawn");
    ListenForGameEvent("player_death");
    ListenForGameEvent("player_team");
    ListenForGameEvent("bot_takeover");

    LoadControlSettings("resource/hud/teamcounter.res");

    for (int i = 0; i < MAX_TEAM_SIZE; ++i)
    {
        m_CTTeam[i].Reset();
        m_TTeam[i].Reset();
    }
    
    for (int i = 0; i < MAX_GGPROG_PLAYERS; ++i)
    {
        m_GGProgressivePlayers[i].Reset();
    }

    // Detect game mode
    if (CSGameRules())
    {
        if (CSGameRules()->IsPlayingGunGameProgressive() || CSGameRules()->IsPlayingGunGameDeathmatch())
            SetViewMode(VIEW_MODE_GUN_GAME_PROGRESSIVE);
        else if (CSGameRules()->IsPlayingGunGameTRBomb())
            SetViewMode(VIEW_MODE_GUN_GAME_BOMB);
        else
            SetViewMode(VIEW_MODE_NORMAL);
    }
}

void CHudTeamCounter::Shutdown()
{
    StopListeningForAllEvents();
}

void CHudTeamCounter::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
    // Restore the panel to its un-expanded state so ApplySettings can
    // re-record the correct original position for the new screen size.
    if (m_bLayoutExpanded && m_iOriginalWide > 0)
    {
        SetPos(m_iOriginalXPos, m_iOriginalYPos);
        SetWide(m_iOriginalWide);
    }
    m_bLayoutExpanded   = false;
    m_iLayoutShiftLeft  = 0;
    m_iLayoutShiftRight = 0;

    LoadControlSettings("resource/hud/teamcounter.res");
}

void CHudTeamCounter::ApplySettings(KeyValues *inResourceData)
{
    BaseClass::ApplySettings(inResourceData);
    // Only record the "true" original position while the panel is at its base
    // state. If ApplySettings is triggered while we are already expanded (e.g.
    // a LoadControlSettings call from inside PerformLayout), we skip it so the
    // saved original never gets corrupted by the shifted position.
    if (!m_bLayoutExpanded)
    {
        GetPos(m_iOriginalXPos, m_iOriginalYPos);
        m_iOriginalWide = GetWide();
    }
    
    m_iAvatarXMargin = inResourceData->GetInt("avatar_xmargin", 2);
    m_iAvatarYMargin = inResourceData->GetInt("avatar_ymargin", 1);
    m_iAvatarWide = inResourceData->GetInt("avatar_wide", 14);
    m_iAvatarTall = inResourceData->GetInt("avatar_tall", 14);
    m_iAvatarXMax = inResourceData->GetInt("avatar_xmax", 6);
    m_iAvatarYMax = inResourceData->GetInt("avatar_ymax", 2);
    m_iAvatarBorderSize = inResourceData->GetInt("avatar_border_size", 2);
    
    // Load colors for bomb icon (without default value)
    Color clrPlanted = inResourceData->GetColor("BombPlantedColor");
    Color clrDefused = inResourceData->GetColor("BombDefusedColor");
    
    // Set defaults if not found (check for all-black)
    m_clrC4Planted = (clrPlanted.r() == 0 && clrPlanted.g() == 0 && clrPlanted.b() == 0 && clrPlanted.a() == 0) 
                      ? Color(255, 0, 0, 255) : clrPlanted;
    m_clrC4Defused = (clrDefused.r() == 0 && clrDefused.g() == 0 && clrDefused.b() == 0 && clrDefused.a() == 0) 
                      ? Color(0, 255, 0, 255) : clrDefused;
    
}

void CHudTeamCounter::Reset()
{
    g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("RoundTimerReset");
    m_bTimerAlertTriggered = false;
    m_bRoundStarted = false;
    m_bIsBombDefused = false;
    m_bTimerHidden = false;
    m_nTScoreLastUpdate = -1;
    m_nCTScoreLastUpdate = -1;

    if (CSGameRules())
        m_bRoundStarted = (CSGameRules()->GetRoundStartTime() < gpGlobals->curtime);
}

bool CHudTeamCounter::ShouldDraw()
{
    if (cl_draw_only_deathnotices.GetBool())
        return false;

    C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
    if (!pPlayer)
        return false;

    if (!pPlayer->IsAlive())
        return false;

    return CHudElement::ShouldDraw();
}

void CHudTeamCounter::OnThink()
{
    UpdateTimer();
    UpdateScore();

    if (m_bIsAtTheBottom != hud_playercount_pos.GetBool())
    {
        m_bIsAtTheBottom = hud_playercount_pos.GetBool();
        int ypos = m_bIsAtTheBottom ? (ScreenHeight() - m_iOriginalYPos - GetTall()) : m_iOriginalYPos;
        // Preserve any horizontal expansion that PerformLayout applied.
        SetPos(m_iOriginalXPos - m_iLayoutShiftLeft, ypos);
    }

    if (m_flPlayingTeamFadeoutTime > -1 && m_flPlayingTeamFadeoutTime <= gpGlobals->curtime)
    {
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutSelectedTeam");
        m_flPlayingTeamFadeoutTime = -1;
    }
    
    if (gpGlobals->curtime - m_flLastMiniScoreboardUpdate >= 0.5f)
    {
        m_bForceRefresh = true;
        m_flLastMiniScoreboardUpdate = gpGlobals->curtime;
        UpdateScore();
        UpdateMiniScoreboard();
    }
}

void CHudTeamCounter::PerformLayout()
{
    BaseClass::PerformLayout();

    if (!m_pRoundTimerLabel)
        return;

    // -----------------------------------------------------------------------
    // Step 1: Undo any previous expansion so every run starts from the
    //         clean baseline recorded by ApplySettings.
    //
    // This prevents cumulative drift when Init() / OnScreenSizeChanged() is
    // called after the panel has already been expanded (e.g. on server change).
    // -----------------------------------------------------------------------
    if (m_bLayoutExpanded)
    {
        // Shift all children back to their original (un-expanded) positions.
        if (m_iLayoutShiftLeft > 0)
        {
            for (int ci = 0; ci < GetChildCount(); ++ci)
            {
                vgui::Panel *pChild = GetChild(ci);
                if (!pChild) continue;
                int cx, cy;
                pChild->GetPos(cx, cy);
                pChild->SetPos(cx - m_iLayoutShiftLeft, cy);
            }
        }
        // Restore panel position and width to original.
        SetPos(m_iOriginalXPos, GetYPos());
        SetWide(m_iOriginalWide);

        m_bLayoutExpanded   = false;
        m_iLayoutShiftLeft  = 0;
        m_iLayoutShiftRight = 0;
    }

    // -----------------------------------------------------------------------
    // Step 2: Read baseline timer position (children are now at .res values).
    // -----------------------------------------------------------------------
    m_pRoundTimerLabel->GetPos(m_iTimerXPos, m_iTimerYPos);
    m_pRoundTimerLabel->GetSize(m_iTimerWide, m_iTimerTall);

    // -----------------------------------------------------------------------
    // Step 3: Compute and apply fresh expansion for the current team counts.
    // -----------------------------------------------------------------------
    {
        // One avatar "slot" = avatar pixel width + border on both sides + margin
        int avatarSlot = m_iAvatarWide + m_iAvatarBorderSize * 2 + m_iAvatarXMargin;

        // Widest row on each team (first row always has the most avatars)
        int ctCols = (m_nCTTeamCount > 0) ? (m_nCTTeamCount + 1) / 2 : 0;
        int tCols  = (m_nTerroristTeamCount > 0) ? (m_nTerroristTeamCount + 1) / 2 : 0;

        // Total space needed on each side of the timer
        int ctRequired = ctCols * avatarSlot + m_iAvatarXMargin;
        int tRequired  = tCols  * avatarSlot + m_iAvatarXMargin;

        // Space currently available on each side
        int ctAvail = m_iTimerXPos;
        int tAvail  = GetWide() - m_iTimerXPos - m_iTimerWide;

        int extraLeft  = MAX(0, ctRequired - ctAvail);
        int extraRight = MAX(0, tRequired  - tAvail);

        if (extraLeft > 0 || extraRight > 0)
        {
            // Clamp so the panel never leaves the screen edges
            int panelScreenX = GetXPos();
            extraLeft  = MIN(extraLeft,  panelScreenX);
            extraRight = MIN(extraRight, ScreenWidth() - (panelScreenX + GetWide()));

            if (extraLeft > 0)
            {
                // Shift ALL children rightward to maintain their screen positions
                // after the panel moves left.
                for (int ci = 0; ci < GetChildCount(); ++ci)
                {
                    vgui::Panel *pChild = GetChild(ci);
                    if (!pChild) continue;
                    int cx, cy;
                    pChild->GetPos(cx, cy);
                    pChild->SetPos(cx + extraLeft, cy);
                }
                m_iTimerXPos += extraLeft;
            }

            // Grow the panel to fit
            SetPos(panelScreenX - extraLeft, GetYPos());
            SetWide(GetWide() + extraLeft + extraRight);

            m_bLayoutExpanded   = true;
            m_iLayoutShiftLeft  = extraLeft;
            m_iLayoutShiftRight = extraRight;
        }
    }

    LayoutPlayerAvatars();
}

void CHudTeamCounter::UpdateTimer()
{
    C_CSGameRules *pRules = CSGameRules();
    if (!pRules)
        return;

    bool bBombPlanted = (g_PlantedC4s.Count() > 0);
    if (bBombPlanted)
    {
        C_PlantedC4 *pC4 = g_PlantedC4s[0];

        if (pC4->m_bBombDefused)
        {
            m_pBombIcon->SetAlpha(255);
            m_pBombIcon->SetFgColor(m_clrC4Defused);
            m_pBombIcon->SetVisible(true);
        }
        else
        {
            int alpha = 255;
            if (gpGlobals->curtime + 0.1f >= pC4->m_flNextGlow)
                alpha = 128;

            m_pBombIcon->SetAlpha(alpha);
            m_pBombIcon->SetFgColor(m_clrC4Planted);
            m_pBombIcon->SetVisible(!pC4->m_bExplodeWarning);
        }
    }
    else
        m_pBombIcon->SetVisible(false);

    if (bBombPlanted || pRules->IsTimeOutActive() || pRules->IsWarmupPeriod())
    {
        m_pRoundTimerLabel->SetText(L" ");
    }
    else
    {
        if (m_iRoundTime < (int)ceil(pRules->GetRoundRemainingTime()))
            m_pRoundTimerLabel->SetFgColor(m_clrRoundTimer);

        m_iRoundTime = (int)ceil(pRules->GetRoundRemainingTime());

        if (pRules->IsFreezePeriod())
        {
            // in freeze period countdown to round start time
            m_iRoundTime = (int)ceil(pRules->GetRoundStartTime() - gpGlobals->curtime);
        }

        if (m_iRoundTime < 0)
            m_iRoundTime = 0;

        if (m_iRoundTime <= 10)
            m_pRoundTimerLabel->SetFgColor(m_clrRoundTimerLow);

        int iMinutes = m_iRoundTime / 60;
        int iSeconds = m_iRoundTime % 60;

        wchar_t unicode[32];
        V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d : %.2d", iMinutes, iSeconds);
        m_pRoundTimerLabel->SetText(unicode);
    }
}

void CHudTeamCounter::UpdateScore()
{
    C_CSTeam *teamCT = GetGlobalCSTeam(TEAM_CT);
    C_CSTeam *teamT = GetGlobalCSTeam(TEAM_TERRORIST);

    wchar_t unicode[16];
    
    if (teamCT)
    {
        V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", teamCT->Get_Score());
        m_pCTWinCounterLabel->SetText(unicode);
    }
    
    if (teamT)
    {
        V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", teamT->Get_Score());
        m_pTWinCounterLabel->SetText(unicode);
    }

    if (g_PR)
    {
        int iCTCounter = 0;
        int iTCounter = 0;
        
        for (int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++)
        {
            if (g_PR->IsConnected(playerIndex) && g_PR->IsAlive(playerIndex))
            {
                if (g_PR->GetTeam(playerIndex) == TEAM_CT)
                    iCTCounter++;

                if (g_PR->GetTeam(playerIndex) == TEAM_TERRORIST)
                    iTCounter++;
            }
        }

        V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", iCTCounter);
        m_pCTAliveCounterLabel->SetText(unicode);

        V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", iTCounter);
        m_pTAliveCounterLabel->SetText(unicode);

        m_pCTAliveCounterLabel->SetVisible(iCTCounter > 0);
        m_pCTAliveTextLabel->SetVisible(iCTCounter > 0);
        m_pTAliveCounterLabel->SetVisible(iTCounter > 0);
        m_pTAliveTextLabel->SetVisible(iTCounter > 0);
        m_pCTSkullImage->SetVisible(iCTCounter < 1);
        m_pTSkullImage->SetVisible(iTCounter < 1);
    }
}

static C_CSPlayer* GetPlayerByIndex(int iIndex)
{
    if (iIndex < 1 || iIndex > gpGlobals->maxClients)
        return nullptr;
    return static_cast<C_CSPlayer*>(UTIL_PlayerByIndex(iIndex));
}

// Sort function for normal mode (pointer-to-pointer version)
int CHudTeamCounter::PlayerSortFunc(const MiniStatus *a, const MiniStatus *b)
{
    // Keep server join order - no sorting in normal modes
    return 0;
}

// Wrapper for CUtlVector::Sort
static int PlayerSortFuncWrapper(MiniStatus* const *a, MiniStatus* const *b)
{
    return CHudTeamCounter::PlayerSortFunc(*a, *b);
}

// Gun Game Progressive sort function
int CHudTeamCounter::GGProgSortFunction(MiniStatus* const* entry1, MiniStatus* const* entry2)
{
    if (entry1 == NULL || (*entry1) == NULL)
        return 1;

    if (entry2 == NULL || (*entry2) == NULL)
        return -1;

    // Higher Gun Game level = better
    if ((*entry1)->nGunGameLevel > (*entry2)->nGunGameLevel)
        return -1;
    else if ((*entry1)->nGunGameLevel < (*entry2)->nGunGameLevel)
        return 1;
    else
    {
        if ((*entry1)->bTeamLeader && (*entry2)->bTeamLeader == false)
            return -1;
        else if ((*entry2)->bTeamLeader && (*entry1)->bTeamLeader == false)
            return 1;

        if ((*entry1)->nPlayerIdx == g_GGProgLeaderPlayerIdx)
            return -1;
        else if ((*entry2)->nPlayerIdx == g_GGProgLeaderPlayerIdx)
            return 1;
        else
            return 0;
    }
}

// Deathmatch sort function
int CHudTeamCounter::DMSortFunction(MiniStatus* const* entry1, MiniStatus* const* entry2)
{
    if (entry1 == NULL || (*entry1) == NULL)
        return 1;

    if (entry2 == NULL || (*entry2) == NULL)
        return -1;

    if ((*entry1)->nPoints > (*entry2)->nPoints)
        return -1;
    else if ((*entry1)->nPoints < (*entry2)->nPoints)
        return 1;
    else
        return 0;
}

void CHudTeamCounter::UpdateMiniScoreboard()
{
    if (!CSGameRules() || !g_PR)
        return;

    C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
    if (!pLocalPlayer)
        return;

    // Update last spectator list update time
    m_flLastSpecListUpdate = gpGlobals->curtime;

    C_CS_PlayerResource *pCSPR = (C_CS_PlayerResource *)g_PR;
    int localPlayerIndex = pLocalPlayer->entindex();

    // Get spectator target
    int spectatedTargetIndex = -1;
    if (pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE || pLocalPlayer->GetObserverMode() == OBS_MODE_CHASE)
        spectatedTargetIndex = pLocalPlayer->GetObserverTarget() ? pLocalPlayer->GetObserverTarget()->entindex() : -1;

    // Bot takeover detection
    int LocalBotControlledIdx = -1;
    if (pLocalPlayer->IsControllingBot())
        LocalBotControlledIdx = pLocalPlayer->GetControlledBotIndex();

    bool bGunGameProgressive = CSGameRules()->IsPlayingGunGameProgressive();
    bool bDeathmatch = CSGameRules()->IsPlayingGunGameDeathmatch();

    int nCTTeamCount = 0, nTerroristTeamCount = 0;

    // Reset all slots before filling
    for (int i = 0; i < MAX_TEAM_SIZE; ++i)
    {
        m_CTTeam[i].Reset();
        m_TTeam[i].Reset();
    }

    // Fill slots with players
    for (int playerIndex = 1; playerIndex <= MAX_PLAYERS; ++playerIndex)
    {
        if (!g_PR->IsConnected(playerIndex))
            continue;

        int teamId = g_PR->GetTeam(playerIndex);
        if (teamId != TEAM_CT && teamId != TEAM_TERRORIST)
            continue;

        C_CSPlayer *pPlayer = GetPlayerByIndex(playerIndex);
        if (!pPlayer)
            continue;

        bool bIsCT = (teamId == TEAM_CT);
        bool bIsLocal = (playerIndex == localPlayerIndex);

        int entIdx = pPlayer->entindex();
        int health = 0;
        int armor = 0;
        int ggLevel = -1;

        // Gun Game level
        if (bGunGameProgressive && pPlayer)
        {
            ggLevel = pPlayer->GetPlayerGunGameWeaponIndex();
            if (pPlayer->MadeFinalGunGameProgressiveKill())
                ggLevel++;
        }

        // Voice chat detection
        bool bSpeaking = GetClientVoiceMgr()->IsPlayerSpeaking(playerIndex);
        if (bSpeaking)
        {
            if (!g_PR->IsFakePlayer(playerIndex) && !GetClientVoiceMgr()->IsPlayerAudible(playerIndex))
                bSpeaking = false;
        }

        // Complex dead/alive logic with bot handling
        bool bDead = false;
        int ControlledByIdx = pCSPR->GetControlledByPlayer(playerIndex);

        if (ControlledByIdx != 0)
        {
            // This is a BOT controlled by a player
            // Show player's alive state
            bDead = !g_PR->IsAlive(ControlledByIdx);
        }
        else
        {
            // Normal player or free bot
            bDead = !g_PR->IsAlive(playerIndex) || pCSPR->IsControllingBot(playerIndex);

            // Edge case: bot kicked while we control it
            int controlledBot = pCSPR->GetControlledPlayer(playerIndex);
            if (pCSPR->IsControllingBot(playerIndex))
            {
                if (!pCSPR->IsConnected(controlledBot) || !pCSPR->IsFakePlayer(controlledBot))
                    bDead = false;
            }
        }

        // Health and armor (only if alive and should show)
        // Spectators/GOTV/HLTV can always see health
        bool bShowHealth = (pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR || 
                           pLocalPlayer->IsObserver() || 
                           engine->IsHLTV());
        if (!bShowHealth && pLocalPlayer)
        {
            ConVarRef mp_forcecamera("mp_forcecamera");
            if (!pLocalPlayer->IsOtherEnemy(playerIndex) || (!pLocalPlayer->IsAlive() && mp_forcecamera.GetInt() == OBS_ALLOW_ALL))
                bShowHealth = true;
        }

        int playerHealthIndex = playerIndex;
        int controlledByIndex = pCSPR->GetControlledByPlayer(playerIndex);
        if (controlledByIndex != 0)
            playerHealthIndex = controlledByIndex;

        if (g_PR->IsAlive(playerHealthIndex) && bShowHealth)
        {
            health = pCSPR->GetHealth(playerHealthIndex);
            C_CSPlayer *pHealthPlayer = GetPlayerByIndex(playerHealthIndex);
            if (pHealthPlayer)
                armor = pHealthPlayer->ArmorValue();
        }

        // Spectator detection
        bool bIsSpectating = (playerHealthIndex == spectatedTargetIndex);

        // Teammate color (competitive mode)
        int nPlayerIdxForColor = -1;
        #if 0
        if (pLocalPlayer->ShouldShowTeamPlayerColors(teamId) && teamId == pLocalPlayer->GetTeamNumber())
            nPlayerIdxForColor = pCSPR->GetCompTeammateColor(playerIndex);
        #endif

        // Team leader (Gun Game)
        int nTeam = bIsCT ? TEAM_CT : TEAM_TERRORIST;
        bool bTeamLeader = (entIdx == GetGlobalTeam(nTeam)->GetGGLeader(nTeam));

        MiniStatus *ms = nullptr;
        int slotIdx = -1;

        if (!bGunGameProgressive && !bDeathmatch)
        {
            // Normal mode - separate teams
            if (bIsCT && nCTTeamCount < MAX_TEAM_SIZE)
            {
                slotIdx = nCTTeamCount++;
                ms = &m_CTTeam[slotIdx];
            }
            else if (!bIsCT && nTerroristTeamCount < MAX_TEAM_SIZE)
            {
                slotIdx = nTerroristTeamCount++;
                ms = &m_TTeam[slotIdx];
            }
        }
        else
        {
            // Gun Game mode - combined list
            slotIdx = nCTTeamCount + nTerroristTeamCount;
            
            if (bIsCT)
                nCTTeamCount++;
            else
                nTerroristTeamCount++;

            if (slotIdx >= MAX_GGPROG_PLAYERS)
                continue;

            ms = &m_GGProgressivePlayers[slotIdx];
        }

        if (!ms)
            continue;

        // Update status
        bool bChanged = ms->Update(
            playerIndex, entIdx, health, armor, bIsCT, bIsLocal, bDead,
            bTeamLeader, ggLevel, teamId,
            pLocalPlayer->IsPlayerDominated(playerIndex),
            pLocalPlayer->IsPlayerDominatingMe(playerIndex),
            bSpeaking,
            (LocalBotControlledIdx == playerIndex),
            bIsSpectating,
            nPlayerIdxForColor,
            gpGlobals->curtime
        );

    }

    // Gun Game sorting and ranking
    if (bGunGameProgressive || bDeathmatch)
    {
        int totalPlayers = nCTTeamCount + nTerroristTeamCount;
        
        if (totalPlayers != m_nPreviousGGProgressiveTotalPlayers)
        {
            m_nPreviousGGProgressiveTotalPlayers = totalPlayers;
        }

        // Sort the list
        m_ggSortedList.RemoveAll();
        for (int i = 0; i < MIN(MAX_GGPROG_PLAYERS, totalPlayers); i++)
            m_ggSortedList.AddToTail(&m_GGProgressivePlayers[i]);

        if (bDeathmatch)
            m_ggSortedList.Sort(DMSortFunction);
        else
            m_ggSortedList.Sort(GGProgSortFunction);

        // Update ranks and slots
        int ctIdx = 0, tIdx = 0;
        
        for (int i = 0; i < MIN(MAX_GGPROG_PLAYERS, totalPlayers); i++)
        {
            MiniStatus *ms = m_ggSortedList[i];
            if (ms && ms->nPlayerIdx != -1)
            {
                ms->nGGProgressiveRank = i;
                
                if (ms->bIsCT && ctIdx < MAX_TEAM_SIZE)
                {
                    m_CTTeam[ctIdx] = *ms;
                    UpdatePlayerSlot(ctIdx, &m_CTTeam[ctIdx], true);
                    ctIdx++;
                }
                else if (!ms->bIsCT && tIdx < MAX_TEAM_SIZE)
                {
                    m_TTeam[tIdx] = *ms;
                    UpdatePlayerSlot(tIdx, &m_TTeam[tIdx], false);
                    tIdx++;
                }
            }
        }
    }
    else
    {
        // Normal mode - sort each team separately
        CUtlVector<MiniStatus*> ctList, tList;
        
        for (int i = 0; i < nCTTeamCount; i++)
            ctList.AddToTail(&m_CTTeam[i]);
        for (int i = 0; i < nTerroristTeamCount; i++)
            tList.AddToTail(&m_TTeam[i]);

        ctList.Sort(PlayerSortFuncWrapper);
        tList.Sort(PlayerSortFuncWrapper);

        // Reassign sorted players and update UI immediately
        for (int i = 0; i < ctList.Count(); i++)
        {
            m_CTTeam[i] = *ctList[i];
            UpdatePlayerSlot(i, &m_CTTeam[i], true);
        }
        for (int i = 0; i < tList.Count(); i++)
        {
            m_TTeam[i] = *tList[i];
            UpdatePlayerSlot(i, &m_TTeam[i], false);
        }
    }

    // Hide unused slots
    for (int i = nCTTeamCount; i < MAX_TEAM_SIZE; ++i)
    {
        m_CTPlayerIcons[i]->SetVisible(false);
        m_CTPlayerIconFrames[i]->SetVisible(false);
        m_CTSkulls[i]->SetVisible(false);
        m_CTMicIcons[i]->SetVisible(false);
        m_CTPlayerNames[i]->SetText(L"");
        m_CTPlayerStatus[i]->SetText(L"");
    }
    for (int i = nTerroristTeamCount; i < MAX_TEAM_SIZE; ++i)
    {
        m_TPlayerIcons[i]->SetVisible(false);
        m_TPlayerIconFrames[i]->SetVisible(false);
        m_TSkulls[i]->SetVisible(false);
        m_TMicIcons[i]->SetVisible(false);
        m_TPlayerNames[i]->SetText(L"");
        m_TPlayerStatus[i]->SetText(L"");
    }
    
    
    if (bGunGameProgressive || bDeathmatch)
    {
        for (int i = 0; i < nCTTeamCount; i++)
        {
            if (i < MAX_TEAM_SIZE && m_CTTeam[i].nPlayerIdx >= 0)
            {
                m_CTPlayerIcons[i]->SetVisible(!m_CTTeam[i].bDead);
                m_CTPlayerIconFrames[i]->SetVisible(!m_CTTeam[i].bDead);
                m_CTSkulls[i]->SetVisible(m_CTTeam[i].bDead);
                m_CTMicIcons[i]->SetVisible(m_CTTeam[i].bSpeaking && !m_CTTeam[i].bDead);
            }
        }
        for (int i = 0; i < nTerroristTeamCount; i++)
        {
            if (i < MAX_TEAM_SIZE && m_TTeam[i].nPlayerIdx >= 0)
            {
                m_TPlayerIcons[i]->SetVisible(!m_TTeam[i].bDead);
                m_TPlayerIconFrames[i]->SetVisible(!m_TTeam[i].bDead);
                m_TSkulls[i]->SetVisible(m_TTeam[i].bDead);
                m_TMicIcons[i]->SetVisible(m_TTeam[i].bSpeaking && !m_TTeam[i].bDead);
            }
        }
    }
    else
    {
        for (int i = 0; i < nCTTeamCount; i++)
        {
            m_CTPlayerIcons[i]->SetVisible(!m_CTTeam[i].bDead);
            m_CTPlayerIconFrames[i]->SetVisible(!m_CTTeam[i].bDead);
            m_CTSkulls[i]->SetVisible(m_CTTeam[i].bDead);
            m_CTMicIcons[i]->SetVisible(m_CTTeam[i].bSpeaking && !m_CTTeam[i].bDead);
        }
        for (int i = 0; i < nTerroristTeamCount; i++)
        {
            m_TPlayerIcons[i]->SetVisible(!m_TTeam[i].bDead);
            m_TPlayerIconFrames[i]->SetVisible(!m_TTeam[i].bDead);
            m_TSkulls[i]->SetVisible(m_TTeam[i].bDead);
            m_TMicIcons[i]->SetVisible(m_TTeam[i].bSpeaking && !m_TTeam[i].bDead);
        }
    }

    m_nCTTeamCount = nCTTeamCount;
    m_nTerroristTeamCount = nTerroristTeamCount;
    m_bForceRefresh = false;
    
    InvalidateLayout();
}

void CHudTeamCounter::CalculateAvatarPosition(int slotIdx, int totalPlayers, bool bIsCT, int &x, int &y, int &row)
{
    int avatarWithBorder = m_iAvatarWide + (m_iAvatarBorderSize * 2);
    
    bool bGunGameProgressive = CSGameRules() && CSGameRules()->IsPlayingGunGameProgressive();
    bool bDeathmatch = CSGameRules() && CSGameRules()->IsPlayingGunGameDeathmatch();
    bool bCompetitive = CSGameRules() && CSGameRules()->IsPlayingAnyCompetitiveStrictRuleset();
    
    if (bGunGameProgressive || bDeathmatch)
    {
        row = 0;
        
        int totalGGPlayers = totalPlayers;
        
        int totalWidth = totalGGPlayers * (avatarWithBorder + m_iAvatarXMargin) - m_iAvatarXMargin;
        int startX = m_iTimerXPos + (m_iTimerWide - totalWidth) / 2;
        
        x = startX + slotIdx * (avatarWithBorder + m_iAvatarXMargin);
        y = m_iTimerYPos + m_iTimerTall + m_iAvatarYMargin;
        
        return;
    }
    
    int firstRowCount, secondRowCount;
    
    if (bCompetitive)
    {
        // Competitive: single row if possible, two rows if too many
        if (totalPlayers <= m_iAvatarXMax)
        {
            firstRowCount = totalPlayers;
            secondRowCount = 0;
        }
        else
        {
            firstRowCount = (totalPlayers + 1) / 2;
            secondRowCount = totalPlayers - firstRowCount;
        }
    }
    else
    {
        // Casual and custom modes: always two rows
        firstRowCount = (totalPlayers + 1) / 2;
        secondRowCount = totalPlayers - firstRowCount;
    }
    
    int currentRow = 0;
    int posInRow = slotIdx;
    
    if (slotIdx >= firstRowCount && secondRowCount > 0)
    {
        currentRow = 1;
        posInRow = slotIdx - firstRowCount;
    }
    
    row = currentRow;
    
    int rowCount = (currentRow == 0) ? firstRowCount : secondRowCount;
    
    if (bIsCT)
    {
        int rowWidth = rowCount * (avatarWithBorder + m_iAvatarXMargin) - m_iAvatarXMargin;
        x = m_iTimerXPos - m_iAvatarXMargin - rowWidth + (posInRow * (avatarWithBorder + m_iAvatarXMargin));
    }
    else
    {
        x = m_iTimerXPos + m_iTimerWide + m_iAvatarXMargin + (posInRow * (avatarWithBorder + m_iAvatarXMargin));
    }
    
    int numRows = (secondRowCount > 0) ? 2 : 1;
    int totalHeight = numRows * (m_iAvatarTall + m_iAvatarBorderSize * 2) + (numRows - 1) * m_iAvatarYMargin;
    int startY = m_iTimerYPos;
    
    y = startY + currentRow * ((m_iAvatarTall + m_iAvatarBorderSize * 2) + m_iAvatarYMargin);
}

void CHudTeamCounter::LayoutPlayerAvatars()
{
    bool bGunGameProgressive = (CSGameRules() && CSGameRules()->IsPlayingGunGameProgressive()) || hud_playercount_gungame_layout.GetBool();
    bool bDeathmatch = CSGameRules() && CSGameRules()->IsPlayingGunGameDeathmatch();
    bool bCompetitive = CSGameRules() && CSGameRules()->IsPlayingAnyCompetitiveStrictRuleset();
    
    bool bForceLargeAvatars = hud_playercount_large_avatars.GetBool();
    
    if (bCompetitive || bForceLargeAvatars)
    {
        m_iAvatarWide = 64;
        m_iAvatarTall = 64;
    }
    else if (bGunGameProgressive || bDeathmatch)
    {
        m_iAvatarWide = 64;
        m_iAvatarTall = 64;
        
        int totalGGPlayers = m_nCTTeamCount + m_nTerroristTeamCount;
        int avatarWithBorder = m_iAvatarWide + (m_iAvatarBorderSize * 2);
        
        int totalWidth = totalGGPlayers * (avatarWithBorder + m_iAvatarXMargin) - m_iAvatarXMargin;
        int startX = m_iTimerXPos + (m_iTimerWide - totalWidth) / 2;
        int yPos = m_iTimerYPos + m_iTimerTall + m_iAvatarYMargin;
        
        for (int i = 0; i < MAX_GGPROG_PLAYERS; i++)
        {
            if (i < totalGGPlayers)
            {
                int xPos = startX + i * (avatarWithBorder + m_iAvatarXMargin);
                
                bool bIsCT = (i < m_nCTTeamCount);
                int slotIdx = bIsCT ? i : (i - m_nCTTeamCount);
                
                if (bIsCT && slotIdx < MAX_TEAM_SIZE)
                {
                    m_CTPlayerIconFrames[slotIdx]->SetPos(xPos, yPos);
                    m_CTPlayerIconFrames[slotIdx]->SetSize(avatarWithBorder, avatarWithBorder);
                    
                    m_CTPlayerIcons[slotIdx]->SetPos(xPos + m_iAvatarBorderSize, yPos + m_iAvatarBorderSize);
                    m_CTPlayerIcons[slotIdx]->SetSize(m_iAvatarWide, m_iAvatarTall);
                    
                    m_CTSkulls[slotIdx]->SetPos(xPos + m_iAvatarBorderSize, yPos + m_iAvatarBorderSize);
                    m_CTSkulls[slotIdx]->SetRenderSize(m_iAvatarWide, m_iAvatarTall);
                    m_CTSkulls[slotIdx]->SetSize(m_iAvatarWide, m_iAvatarTall);
                    
                    int micSize = m_iAvatarWide / 3;
                    m_CTMicIcons[slotIdx]->SetPos(xPos + m_iAvatarBorderSize + m_iAvatarWide - micSize, yPos + m_iAvatarBorderSize - 2);
                    m_CTMicIcons[slotIdx]->SetSize(micSize, micSize);
                }
                else if (!bIsCT && slotIdx < MAX_TEAM_SIZE)
                {
                    m_TPlayerIconFrames[slotIdx]->SetPos(xPos, yPos);
                    m_TPlayerIconFrames[slotIdx]->SetSize(avatarWithBorder, avatarWithBorder);
                    
                    m_TPlayerIcons[slotIdx]->SetPos(xPos + m_iAvatarBorderSize, yPos + m_iAvatarBorderSize);
                    m_TPlayerIcons[slotIdx]->SetSize(m_iAvatarWide, m_iAvatarTall);
                    
                    m_TSkulls[slotIdx]->SetPos(xPos + m_iAvatarBorderSize, yPos + m_iAvatarBorderSize);
                    m_TSkulls[slotIdx]->SetRenderSize(m_iAvatarWide, m_iAvatarTall);
                    m_TSkulls[slotIdx]->SetSize(m_iAvatarWide, m_iAvatarTall);
                    
                    int micSize = m_iAvatarWide / 3;
                    m_TMicIcons[slotIdx]->SetPos(xPos + m_iAvatarBorderSize + m_iAvatarWide - micSize, yPos + m_iAvatarBorderSize - 2);
                    m_TMicIcons[slotIdx]->SetSize(micSize, micSize);
                }
            }
        }
        
        return;
    }
    else
    {
        // Start from the default half-size.
        // Then shrink further if needed so avatars never overflow their side of the panel.
        //
        // CT avatars grow LEFT from the timer:
        //   each row must fit in  m_iTimerXPos  pixels wide.
        //   max_size = timerXPos / cols - border*2 - xmargin
        //
        // T avatars grow RIGHT from the timer:
        //   each row must fit in  GetWide() - timerXPos - timerWide  pixels wide.
        //   max_size = tSpace / cols - border*2 - xmargin
        //
        // Use the smallest of the two to keep both sides consistent.
        int baseSize = 14;

        if (m_nCTTeamCount > 0)
        {
            int ctCols = (m_nCTTeamCount + 1) / 2; // first-row count = widest row
            int maxCT  = m_iTimerXPos / ctCols - m_iAvatarBorderSize * 2 - m_iAvatarXMargin;
            baseSize   = MIN(baseSize, MAX(6, maxCT));
        }

        if (m_nTerroristTeamCount > 0)
        {
            int tCols  = (m_nTerroristTeamCount + 1) / 2;
            int tSpace = GetWide() - m_iTimerXPos - m_iTimerWide;
            int maxT   = tSpace / tCols - m_iAvatarBorderSize * 2 - m_iAvatarXMargin;
            baseSize   = MIN(baseSize, MAX(6, maxT));
        }

        m_iAvatarWide = baseSize;
        m_iAvatarTall = baseSize;
    }
    
    for (int i = 0; i < MAX_TEAM_SIZE; i++)
    {
        int x, y, row;
        CalculateAvatarPosition(i, m_nCTTeamCount, true, x, y, row);
        
        m_CTPlayerIconFrames[i]->SetPos(x, y);
        m_CTPlayerIconFrames[i]->SetSize(m_iAvatarWide + m_iAvatarBorderSize * 2, m_iAvatarTall + m_iAvatarBorderSize * 2);
        
        m_CTPlayerIcons[i]->SetPos(x + m_iAvatarBorderSize, y + m_iAvatarBorderSize);
        m_CTPlayerIcons[i]->SetSize(m_iAvatarWide, m_iAvatarTall);
        
        m_CTSkulls[i]->SetPos(x + m_iAvatarBorderSize, y + m_iAvatarBorderSize);
        m_CTSkulls[i]->SetSize(m_iAvatarWide, m_iAvatarTall);
        
        int micSize = m_iAvatarWide / 3;
        m_CTMicIcons[i]->SetPos(x + m_iAvatarBorderSize + m_iAvatarWide - micSize, y + m_iAvatarBorderSize - 2);
        m_CTMicIcons[i]->SetSize(micSize, micSize);
        
        bool bVisible = (i < m_nCTTeamCount);
        if (!bVisible)
        {
            m_CTPlayerIconFrames[i]->SetVisible(false);
            m_CTPlayerIcons[i]->SetVisible(false);
            m_CTSkulls[i]->SetVisible(false);
            m_CTMicIcons[i]->SetVisible(false);
        }
    }
    
    for (int i = 0; i < MAX_TEAM_SIZE; i++)
    {
        int x, y, row;
        CalculateAvatarPosition(i, m_nTerroristTeamCount, false, x, y, row);
        
        m_TPlayerIconFrames[i]->SetPos(x, y);
        m_TPlayerIconFrames[i]->SetSize(m_iAvatarWide + m_iAvatarBorderSize * 2, m_iAvatarTall + m_iAvatarBorderSize * 2);
        
        m_TPlayerIcons[i]->SetPos(x + m_iAvatarBorderSize, y + m_iAvatarBorderSize);
        m_TPlayerIcons[i]->SetSize(m_iAvatarWide, m_iAvatarTall);
        
        m_TSkulls[i]->SetPos(x + m_iAvatarBorderSize, y + m_iAvatarBorderSize);
        m_TSkulls[i]->SetSize(m_iAvatarWide, m_iAvatarTall);
        
        int micSize = m_iAvatarWide / 3;
        m_TMicIcons[i]->SetPos(x + m_iAvatarBorderSize + m_iAvatarWide - micSize, y + m_iAvatarBorderSize - 2);
        m_TMicIcons[i]->SetSize(micSize, micSize);
        
        bool bVisible = (i < m_nTerroristTeamCount);
        if (!bVisible)
        {
            m_TPlayerIconFrames[i]->SetVisible(false);
            m_TPlayerIcons[i]->SetVisible(false);
            m_TSkulls[i]->SetVisible(false);
            m_TMicIcons[i]->SetVisible(false);
        }
    }
}

void CHudTeamCounter::UpdatePlayerSlot(int slotIdx, const MiniStatus* ms, bool bIsCT)
{
    if (!ms || slotIdx < 0 || slotIdx >= MAX_TEAM_SIZE)
        return;

    CAvatarImagePanel **pIcons = bIsCT ? m_CTPlayerIcons : m_TPlayerIcons;
    vgui::Panel **pFrames = bIsCT ? m_CTPlayerIconFrames : m_TPlayerIconFrames;
    VectorImagePanel **pSkulls = bIsCT ? m_CTSkulls : m_TSkulls;
    ImagePanel **pMicIcons = bIsCT ? m_CTMicIcons : m_TMicIcons;
    int *pLastAvatarIdx = bIsCT ? m_nLastAvatarPlayerIdx_CT : m_nLastAvatarPlayerIdx_T;

    // Only update avatar if player changed - reduces Steam API calls
    if (pLastAvatarIdx[slotIdx] != ms->nPlayerIdx)
    {
        pIcons[slotIdx]->SetPlayer(ms->nPlayerIdx, k_EAvatarSize32x32);
        
        C_CSPlayer *pSlotPlayer = GetPlayerByIndex(ms->nPlayerIdx);
        pIcons[slotIdx]->SetDefaultAvatar(GetDefaultAvatarImage(pSlotPlayer));
        
        pLastAvatarIdx[slotIdx] = ms->nPlayerIdx;
    }

    C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
    bool bIsTeammate = false;
    
    C_CSPlayer *pSlotPlayer = GetPlayerByIndex(ms->nPlayerIdx);
    if (pLocalPlayer && pSlotPlayer)
    {
        bIsTeammate = (pLocalPlayer->GetTeamNumber() == pSlotPlayer->GetTeamNumber());
    }
    
    Color skullColor;
    if (bIsTeammate)
    {
        skullColor = Color(255, 50, 50, 220);
    }
    else
    {
        if (bIsCT)
            skullColor = Color(150, 200, 255, 128);
        else
            skullColor = Color(255, 180, 100, 128);
    }
    
    pSkulls[slotIdx]->SetFgColor(skullColor);

    // Frame color priority: 1) Local player (white), 2) Teammate color, 3) Team color
    Color frameColor;
    
    if (ms->bIsLocalPlayer)
    {
        frameColor = Color(255, 255, 255, 255);
    }
    else if (ms->nTeammateColor >= 0)
    {
        switch (ms->nTeammateColor)
        {
            case 0: frameColor = Color(242, 242, 0, 255); break;
            case 1: frameColor = Color(244, 67, 54, 255); break;
            case 2: frameColor = Color(76, 175, 80, 255); break;
            case 3: frameColor = Color(33, 150, 243, 255); break;
            case 4: frameColor = Color(255, 152, 0, 255); break;
            default: frameColor = Color(200, 200, 200, 255); break;
        }
    }
    else
    {
        frameColor = bIsCT ? Color(150, 200, 255, 255) : Color(255, 180, 100, 255);
    }
    
    pFrames[slotIdx]->SetBgColor(frameColor);
    pFrames[slotIdx]->SetPaintBackgroundEnabled(true);
    pFrames[slotIdx]->SetPaintBackgroundType(0); // Flat fill

    pIcons[slotIdx]->SetVisible(!ms->bDead);
    pFrames[slotIdx]->SetVisible(!ms->bDead);
    pSkulls[slotIdx]->SetVisible(ms->bDead);
    pMicIcons[slotIdx]->SetVisible(ms->bSpeaking && !ms->bDead);
}

// Spectator functions
const MiniStatus* CHudTeamCounter::GetPlayerStatus(int index)
{
    // Check if we need to update the list
    if (!m_bActive && (m_flLastSpecListUpdate + 1.0f < gpGlobals->curtime))
        UpdateMiniScoreboard();

    if (!CSGameRules())
        return NULL;

    bool bGunGame = CSGameRules()->IsPlayingGunGameProgressive() || CSGameRules()->IsPlayingGunGameDeathmatch();

    if (bGunGame)
    {
        if (index < 0 || index >= m_ggSortedList.Count())
            return NULL;
        return m_ggSortedList[index];
    }
    else
    {
        // Normal mode - CT team then T team
        int i = m_nCTTeamCount - index - 1;
        if (i >= 0)
            return &m_CTTeam[i];

        i = -i - 1;
        if (i < m_nTerroristTeamCount)
            return &m_TTeam[i];

        return NULL;
    }
}

int CHudTeamCounter::GetPlayerSlotIndex(int playerEntIndex)
{
    // Check if we need to update
    if (!m_bActive && (m_flLastSpecListUpdate + 1.0f < gpGlobals->curtime))
        UpdateMiniScoreboard();

    for (int i = 0; i < MAX_TEAM_SIZE * 2; i++)
    {
        const MiniStatus* pMS = GetPlayerStatus(i);
        if (pMS && (pMS->nPlayerIdx == playerEntIndex))
            return i;
    }

    return -1;
}

int CHudTeamCounter::GetPlayerEntIndexInSlot(int nIndex)
{
    const MiniStatus* pMS = GetPlayerStatus(nIndex);
    if (pMS)
        return pMS->nPlayerIdx;

    return -1;
}

int CHudTeamCounter::GetSpectatorTargetFromSlot(int idx)
{
    C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
    if (!pLocalPlayer)
        return -1;

    // Update if needed
    if (!m_bActive && m_flLastSpecListUpdate + 1.0f < gpGlobals->curtime)
        UpdateMiniScoreboard();

    C_CS_PlayerResource* pCSPR = (C_CS_PlayerResource*)g_PR;
    
    int spectatedTargetIndex = -1;
    if (pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE || pLocalPlayer->GetObserverMode() == OBS_MODE_CHASE)
    {
        if (pLocalPlayer->GetObserverTarget())
            spectatedTargetIndex = pLocalPlayer->GetObserverTarget()->entindex();
    }

    ConVarRef mp_forcecamera("mp_forcecamera");
    bool showEnemy = mp_forcecamera.GetInt() == OBS_ALLOW_ALL;

    if (pLocalPlayer->GetTeamNumber() < TEAM_TERRORIST)
        showEnemy = true;

    bool bWantCT = spectatedTargetIndex > 0 ? (pCSPR->GetTeam(spectatedTargetIndex) == TEAM_CT) : false;

    const MiniStatus* pStatus = GetPlayerStatus(idx);

    if (pStatus && !pStatus->bDead && (showEnemy || (pStatus->bIsCT == bWantCT)))
    {
        int result = pCSPR->GetControlledByPlayer(pStatus->nPlayerIdx);
        if (!result)
            result = pStatus->nPlayerIdx;

        return result;
    }

    return -1;
}

int CHudTeamCounter::FindNextObserverTargetIndex(bool reverse)
{
    C_CSPlayer *pLocalPlayer = C_CSPlayer::GetLocalCSPlayer();
    if (!pLocalPlayer)
        return -1;

    // Update if needed
    if (m_flLastSpecListUpdate + 1.0f < gpGlobals->curtime)
        UpdateMiniScoreboard();

    int localPlayerIndex = pLocalPlayer->entindex();
    C_CS_PlayerResource* pCSPR = (C_CS_PlayerResource*)g_PR;

    int spectatedTargetIndex = -1;
    if (pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE || pLocalPlayer->GetObserverMode() == OBS_MODE_CHASE)
    {
        if (pLocalPlayer->GetObserverTarget())
            spectatedTargetIndex = pLocalPlayer->GetObserverTarget()->entindex();

        if (!spectatedTargetIndex)
            spectatedTargetIndex = localPlayerIndex;
    }

    if (!spectatedTargetIndex)
        return -1;

    int controlledPlayer = pCSPR->GetControlledPlayer(spectatedTargetIndex);
    if (controlledPlayer != 0)
        spectatedTargetIndex = controlledPlayer;

    int originalSlotIndex = GetPlayerSlotIndex(spectatedTargetIndex);
    if (originalSlotIndex == -1)
        return -1;

    ConVarRef mp_forcecamera("mp_forcecamera");
    bool showEnemy = mp_forcecamera.GetInt() == OBS_ALLOW_ALL;

    if (pLocalPlayer->GetTeamNumber() < TEAM_TERRORIST)
        showEnemy = true;

    bool bWantCT = (pCSPR->GetTeam(spectatedTargetIndex) == TEAM_CT);

    int result = -1;
    int currentSlotIndex = originalSlotIndex;
    int nMaxPlayers = MAX_TEAM_SIZE * 2;

    while (result == -1)
    {
        if (reverse)
            currentSlotIndex--;
        else
            currentSlotIndex++;

        if (currentSlotIndex >= nMaxPlayers)
            currentSlotIndex -= nMaxPlayers;
        else if (currentSlotIndex < 0)
            currentSlotIndex += nMaxPlayers;

        if (currentSlotIndex == originalSlotIndex)
            break;

        const MiniStatus *pStatus = GetPlayerStatus(currentSlotIndex);

        if (pStatus && !pStatus->bDead && (showEnemy || (pStatus->bIsCT == bWantCT)))
        {
            result = pCSPR->GetControlledByPlayer(pStatus->nPlayerIdx);
            if (!result)
                result = pStatus->nPlayerIdx;
        }
    }

    return result;
}

void CHudTeamCounter::SetViewMode(VIEW_MODE mode)
{
    m_Mode = mode;
    
    if (m_Mode == VIEW_MODE_GUN_GAME_PROGRESSIVE)
    {
        ResetLeader();
    }
    else if (m_Mode == VIEW_MODE_GUN_GAME_BOMB)
    {
        ResetLeader();
    }
}

void CHudTeamCounter::ResetLeader()
{
    g_GGProgLeaderPlayerIdx = -1;
    if (m_pProgressiveLeaderLabel)
        m_pProgressiveLeaderLabel->SetText(L"");
}

void CHudTeamCounter::FireGameEvent(IGameEvent *event)
{
    const char *type = event->GetName();
    CBasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
    int EventUserID = event->GetInt("userid", -1);
    int LocalPlayerID = pLocalPlayer ? pLocalPlayer->GetUserID() : -2;

    if (!V_strcmp(type, "round_start"))
    {
        m_bRoundStarted = true;
        m_bIsBombDefused = false;
        m_bForceRefresh = true;  
        UpdateMiniScoreboard();   
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("RoundTimerNormal");

        if (m_Mode == VIEW_MODE_GUN_GAME_PROGRESSIVE)
            ResetLeader();
    }
    else if (!V_strcmp(type, "round_announce_warmup"))
    {
        m_pRoundTimerLabel->SetText(L"");
        UpdateMiniScoreboard();
    }
    else if (!V_strcmp(type, "round_end"))
    {
        C_CSGameRules *pRules = CSGameRules();
        if (pRules)
        {
            int nTimer = static_cast<int>(floor(pRules->GetRoundRemainingTime()));
            if (nTimer < 0)
                nTimer = 0;
            wchar_t szTime[32];
            V_snwprintf(szTime, ARRAYSIZE(szTime), L"%d:%.2d", nTimer / 60, nTimer % 60);
            m_pRoundTimerLabel->SetText(szTime);

            int iReason = event->GetInt("reason", -1);
            if (iReason == Bomb_Defused)
                m_bIsBombDefused = true;
        }
        m_bTimerAlertTriggered = false;
        m_bRoundStarted = false;
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HideTeamPanels");
        UpdateMiniScoreboard();
    }
    else if (!V_strcmp(type, "cs_match_end_restart"))
    {
        if (m_Mode == VIEW_MODE_GUN_GAME_PROGRESSIVE || m_Mode == VIEW_MODE_GUN_GAME_BOMB)
            ResetLeader();
            UpdateMiniScoreboard();
    }
    else if (!V_strcmp(type, "bomb_planted"))
    {
        m_pRoundTimerLabel->SetText(L"");
        m_pBombIcon->SetVisible(true);
        m_pBombIcon->SetAlpha(100);
        m_pBombIcon->SetFgColor(m_clrC4Planted);
        UpdateMiniScoreboard();
    }
    else if (!V_strcmp(type, "bomb_defused"))
    {
        m_bIsBombDefused = true;
        UpdateMiniScoreboard();
    }
    else if (!V_strcmp(type, "player_spawn"))
    {
        UpdateMiniScoreboard();
        if (EventUserID == LocalPlayerID)
        {
            g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HideTeamPanels");
            int nTeam = event->GetInt("teamnum", -1);
            if (nTeam > 0)
            {
                g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowSelectedTeam");
                m_flPlayingTeamFadeoutTime = gpGlobals->curtime + 10.0f;
            }
            m_bRoundStarted = true;
        }
    }
    else if (!V_strcmp(type, "player_death"))
    {
        UpdateMiniScoreboard();
        
        if (EventUserID == LocalPlayerID)
            g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HideTeamPanels");
    }
    else if (!V_strcmp(type, "player_team") && EventUserID == LocalPlayerID)
    {
        m_bForceRefresh = true;
        UpdateMiniScoreboard();
    }
    else if (!V_strcmp(type, "bot_takeover") && EventUserID == LocalPlayerID)
    {
        C_BasePlayer *pBot = UTIL_PlayerByUserId(event->GetInt("botid"));
        if (pBot)
        {
            wchar_t wszLocalized[100];
            wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
            g_pVGuiLocalize->ConvertANSIToUnicode(pBot->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName));
            g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#SFUI_Notice_Hint_Bot_Takeover"), 1, wszPlayerName);
            g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowBotTakeover");
            m_flPlayingTeamFadeoutTime = -1;
        }
        UpdateMiniScoreboard();
    }
}
