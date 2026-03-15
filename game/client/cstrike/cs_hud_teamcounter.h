//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: a small piece of HUD that shows alive counter and win counter for each team
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef CS_HUD_TEAMCOUNTER_H
#define CS_HUD_TEAMCOUNTER_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "utlvector.h"
#include "vgui_avatarimage.h"

enum VIEW_MODE
{
    VIEW_MODE_NORMAL = 0,
    VIEW_MODE_GUN_GAME_PROGRESSIVE,
    VIEW_MODE_GUN_GAME_BOMB,
    VIEW_MODE_NUM
};

struct MiniStatus
{
    int nPlayerIdx;
    int nEntIdx;
    int nHealth;
    int nArmor;
    int nPoints;
    int nGunGameLevel;
    int nGGProgressiveRank;
    int nTeammateColor;
    int nTeam;
    bool bIsCT;
    bool bIsLocalPlayer;
    bool bDead;
    bool bTeamLeader;
    bool bDominated;
    bool bDominating;
    bool bSpeaking;
    bool bPlayerBot;
    bool bSpectated;
    float flLastRefresh;

    MiniStatus() { Reset(); }

    void Reset()
    {
        nPlayerIdx = -1;
        nEntIdx = -1;
        nHealth = 0;
        nArmor = 0;
        nPoints = 0;
        nGunGameLevel = -1;
        nGGProgressiveRank = -1;
        nTeammateColor = -1;
        nTeam = 0;
        bIsCT = false;
        bIsLocalPlayer = false;
        bDead = true;
        bTeamLeader = false;
        bDominated = false;
        bDominating = false;
        bSpeaking = false;
        bPlayerBot = false;
        bSpectated = false;
        flLastRefresh = -1;
    }

    bool Update(int idx, int entIdx, int health, int armor, bool isCT, bool isLocal, bool dead, 
                bool leader, int ggLevel, int team, bool dominated, bool dominating,
                bool speaking, bool playerBot, bool spectated, int teammateColor, float curtime)
    {
        bool bChanged = (nPlayerIdx != idx || nEntIdx != entIdx || nHealth != health || nArmor != armor ||
                         bIsCT != isCT || bIsLocalPlayer != isLocal || bDead != dead || bTeamLeader != leader || nGunGameLevel != ggLevel || nTeam != team ||
                         bDominated != dominated || bDominating != dominating || bSpeaking != speaking ||
                         bPlayerBot != playerBot || bSpectated != spectated || nTeammateColor != teammateColor ||
                         (curtime > flLastRefresh + 3.0f)); // Force refresh every 3 seconds
        
        nPlayerIdx = idx;
        nEntIdx = entIdx;
        nHealth = health;
        nArmor = armor;
        bIsCT = isCT;
        bIsLocalPlayer = isLocal;
        bDead = dead;
        bTeamLeader = leader;
        nGunGameLevel = ggLevel;
        nTeam = team;
        bDominated = dominated;
        bDominating = dominating;
        bSpeaking = speaking;
        bPlayerBot = playerBot;
        bSpectated = spectated;
        nTeammateColor = teammateColor;
        
        if (bChanged)
            flLastRefresh = curtime;
        
        return bChanged;
    }
};

class CHudTeamCounter : public CHudElement, public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudTeamCounter, vgui::EditablePanel);

public:
    CHudTeamCounter(const char *pElementName);
    virtual ~CHudTeamCounter();

    virtual void Init();
    virtual void Shutdown();
    virtual void OnScreenSizeChanged(int iOldWide, int iOldTall);
    virtual void ApplySettings(KeyValues *inResourceData);
    virtual void Reset();
    virtual bool ShouldDraw();
    virtual void OnThink();
    virtual void PerformLayout();
    virtual void FireGameEvent(IGameEvent *event);

    // Spectator functions
    int FindNextObserverTargetIndex(bool reverse);
    int GetSpectatorTargetFromSlot(int idx);
    int GetPlayerEntIndexInSlot(int nIndex);
    int GetPlayerSlotIndex(int playerEntIndex);
    const MiniStatus* GetPlayerStatus(int index);
    
    static int PlayerSortFunc(const MiniStatus *a, const MiniStatus *b);

protected:
    void UpdateTimer();
    void UpdateScore();
    void UpdateMiniScoreboard();
    void SetViewMode(VIEW_MODE mode);
    void ResetLeader();
    void UpdatePlayerSlot(int slotIdx, const MiniStatus* ms, bool bIsCT);
    void SortPlayers();
    void LayoutPlayerAvatars();
    void CalculateAvatarPosition(int slotIdx, int totalPlayers, bool bIsCT, int &x, int &y, int &row);
    
    static int GGProgSortFunction(MiniStatus* const* entry1, MiniStatus* const* entry2);
    static int DMSortFunction(MiniStatus* const* entry1, MiniStatus* const* entry2);
    
    static const int MAX_TEAM_SIZE = 16;
    static const int MAX_GGPROG_PLAYERS = 24;
    static const int kTimeRemainingToDisplayRed = 11;

private:
    vgui::Label *m_pCTWinCounterLabel;
    vgui::Label *m_pCTAliveCounterLabel;
    vgui::Label *m_pCTAliveTextLabel;
    vgui::Label *m_pTWinCounterLabel;
    vgui::Label *m_pTAliveCounterLabel;
    vgui::Label *m_pTAliveTextLabel;
    vgui::Label *m_pRoundTimerLabel;
    vgui::VectorImagePanel *m_pBombIcon;
    vgui::VectorImagePanel *m_pCTSkullImage;
    vgui::VectorImagePanel *m_pTSkullImage;
    vgui::Label *m_pProgressiveLeaderLabel;
    
    C_CSPlayer *pPlayer;

    CAvatarImagePanel *m_CTPlayerIcons[MAX_TEAM_SIZE];
    CAvatarImagePanel *m_TPlayerIcons[MAX_TEAM_SIZE];
    vgui::Panel *m_CTPlayerIconFrames[MAX_TEAM_SIZE];
    vgui::Panel *m_TPlayerIconFrames[MAX_TEAM_SIZE];
    vgui::VectorImagePanel *m_CTSkulls[MAX_TEAM_SIZE];
    vgui::ImagePanel *m_CTMicIcons[MAX_TEAM_SIZE];
    vgui::ImagePanel *m_TMicIcons[MAX_TEAM_SIZE];
    vgui::Label *m_CTPlayerNames[MAX_TEAM_SIZE];
    vgui::Label *m_CTPlayerStatus[MAX_TEAM_SIZE];
    vgui::VectorImagePanel *m_TSkulls[MAX_TEAM_SIZE];
    vgui::Label *m_TPlayerNames[MAX_TEAM_SIZE];
    vgui::Label *m_TPlayerStatus[MAX_TEAM_SIZE];

    MiniStatus m_CTTeam[MAX_TEAM_SIZE];
    MiniStatus m_TTeam[MAX_TEAM_SIZE];
    MiniStatus m_GGProgressivePlayers[MAX_GGPROG_PLAYERS];
    CUtlVector<MiniStatus *> m_ggSortedList;

    VIEW_MODE m_Mode;
    bool m_bTimerAlertTriggered;
    bool m_bRoundStarted;
    bool m_bIsBombDefused;
    bool m_bTimerHidden;
    bool m_bIsAtTheBottom;
    bool m_bForceRefresh;
    int m_nTScoreLastUpdate;
    int m_nCTScoreLastUpdate;
    int m_nTerroristTeamCount;
    int m_nCTTeamCount;
    int m_nPreviousGGProgressiveTotalPlayers;
    int m_iOriginalXPos;
    int m_iOriginalYPos;
    int m_iOriginalWide;
    bool m_bLayoutExpanded;
    int m_iLayoutShiftLeft;
    int m_iLayoutShiftRight;
    int m_iTimerXPos;
    int m_iTimerYPos;
    int m_iTimerWide;
    int m_iTimerTall;
    float m_flPlayingTeamFadeoutTime;
    float m_flLastSpecListUpdate;
    bool m_bActive;
    int m_iRoundTime;
    float m_flLastMiniScoreboardUpdate;
    int m_nLastObserverMode;
    int m_nLastObserverTarget;
    
    // Avatar layout parameters from .res
    int m_iAvatarXMargin;
    int m_iAvatarYMargin;
    int m_iAvatarWide;
    int m_iAvatarTall;
    int m_iAvatarXMax;
    int m_iAvatarYMax;
    int m_iAvatarBorderSize;

private:
    int m_nLastAvatarPlayerIdx_CT[MAX_TEAM_SIZE];
    int m_nLastAvatarPlayerIdx_T[MAX_TEAM_SIZE];
    
    CPanelAnimationVar( Color, m_clrRoundTimer, "RoundTimerColor", "White" );
	CPanelAnimationVar( Color, m_clrRoundTimerLow, "RoundTimerLowColor", "White" );
	CPanelAnimationVar( Color, m_clrC4Planted, "C4PlantedColor", "White" );
	CPanelAnimationVar( Color, m_clrC4Defused, "C4DefusedColor", "White" );
    
    
};

#endif // CS_HUD_TEAMCOUNTER_H
